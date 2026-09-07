#include "bim/geometry_api/geometry.hpp"

#include "solid_impl.hpp"

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <cmath>
#include <exception>
#include <memory>
#include <string>

// P0-T002 OCCT adapter (Implementation Brief BIM-TASK-P0-T002-CLAUDE v1.0
// Phase D/E; Amendment 01 AA-C02/AA-C03/AA-C04). This translation unit is
// the ONLY project-owned code (together with spike_diagnostics.cpp) that
// includes an OCCT header and holds a bim::geometry_api::Solid complete
// definition (solid_impl.hpp).
//
// OCCT API discipline (P0-T002 OCCT 8.0.1 API VERIFICATION GATE, this
// session): every OCCT symbol used below was independently confirmed
// present by reading the actual OCCT-8.0.1-READPACK header text, with one
// documented exception - see the "NOT INDEPENDENTLY VERIFIED" note above
// Cut()/Fuse() for the BRepAlgoAPI Boolean family's IsDone(). Per that
// verification round and this authorization's section 4, HasErrors() (not
// IsDone()) is used as the Boolean-family failure gate throughout this file.

namespace bim::geometry_api {

namespace {

// --- small numeric helpers --------------------------------------------------

[[nodiscard]] bool IsFinite(double v) noexcept {
    return std::isfinite(v);
}

[[nodiscard]] bool IsFinite(const Point3& p) noexcept {
    return IsFinite(p.x) && IsFinite(p.y) && IsFinite(p.z);
}

[[nodiscard]] bool IsFinite(const Vector3& v) noexcept {
    return IsFinite(v.x) && IsFinite(v.y) && IsFinite(v.z);
}

[[nodiscard]] double Magnitude(const Vector3& v) noexcept {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

[[nodiscard]] bool ToleranceIsUsable(const GeometryTolerance& tolerance) noexcept {
    return IsFinite(tolerance.linear) && tolerance.linear > 0.0 &&
           IsFinite(tolerance.angular_radians) && tolerance.angular_radians > 0.0;
}

[[nodiscard]] gp_Pnt ToOcctPoint(const Point3& p) noexcept {
    return gp_Pnt(p.x, p.y, p.z);
}

[[nodiscard]] GeometryError MakeError(GeometryErrorCode code, std::string message) {
    return GeometryError{code, std::move(message)};
}

[[nodiscard]] SolidResult ErrorResult(GeometryErrorCode code, std::string message) {
    return SolidResult{nullptr, MakeError(code, std::move(message))};
}

// Catches Standard_Failure / std::exception / unknown exceptions raised by
// `fn` (which performs OCCT work) and translates them to a
// KernelOperationFailed SolidResult (Brief section 29: never expose an
// OCCT exception to the caller). `fn` returns SolidResult on the success
// path, so the try/catch is centralized here rather than duplicated in
// every operation.
template <class Fn> [[nodiscard]] SolidResult ContainOcctExceptions(const char* op_name, Fn&& fn) {
    try {
        return fn();
    } catch (const Standard_Failure& failure) {
        const char* what = failure.what();
        return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                           std::string(op_name) + ": OCCT Standard_Failure: " +
                               (what != nullptr ? what : "<no message>"));
    } catch (const std::exception& ex) {
        return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                           std::string(op_name) + ": std::exception: " + ex.what());
    } catch (...) {
        return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                           std::string(op_name) + ": unknown exception");
    }
}

// Validates a completed solid-producing OCCT result per Brief section 9:
// non-null, contains 3D solid content, passes BRepCheck_Analyzer validity,
// and has a finite positive volume. Returns an InvalidResult SolidResult on
// any failure, or a successful SolidResult wrapping `shape` otherwise.
// `require_single_solid` additionally enforces "exactly one resulting 3D
// solid" (Fuse's connected-union contract, Brief section 7.3; Amendment 01
// AA-C04 point 4).
[[nodiscard]] SolidResult ValidateAndWrapSolid(TopoDS_Shape shape, const char* op_name,
                                               bool require_single_solid) {
    if (shape.IsNull()) {
        return ErrorResult(GeometryErrorCode::InvalidResult,
                           std::string(op_name) + ": OCCT returned a null shape");
    }

    int solid_count = 0;
    for (TopExp_Explorer explorer(shape, TopAbs_SOLID); explorer.More(); explorer.Next()) {
        ++solid_count;
    }
    if (solid_count < 1) {
        return ErrorResult(GeometryErrorCode::InvalidResult,
                           std::string(op_name) + ": result contains no 3D solid content");
    }
    if (require_single_solid && solid_count != 1) {
        return ErrorResult(GeometryErrorCode::InvalidResult,
                           std::string(op_name) +
                               ": connected-union result must contain exactly one solid, found " +
                               std::to_string(solid_count));
    }

    BRepCheck_Analyzer analyzer(shape);
    if (!analyzer.IsValid()) {
        return ErrorResult(GeometryErrorCode::InvalidResult,
                           std::string(op_name) + ": BRepCheck_Analyzer rejected the result shape");
    }

    GProp_GProps props;
    BRepGProp::VolumeProperties(shape, props);
    const double volume = props.Mass();
    if (!IsFinite(volume) || volume <= 0.0) {
        return ErrorResult(GeometryErrorCode::InvalidResult,
                           std::string(op_name) + ": computed volume is not finite/positive");
    }

    return SolidResult{std::make_shared<const Solid>(Solid(std::move(shape))),
                       MakeError(GeometryErrorCode::None, "")};
}

} // namespace

// ============================================================================
// MakeLinearExtrusion (Brief section 7.1; Amendment 01 AA-C02)
// ============================================================================
SolidResult MakeLinearExtrusion(const LinearExtrusionSpec& spec,
                                const GeometryTolerance& tolerance) {
    const RectangleProfile3& profile = spec.profile;

    if (!ToleranceIsUsable(tolerance)) {
        return ErrorResult(
            GeometryErrorCode::InvalidInput,
            "MakeLinearExtrusion: tolerance.linear/angular_radians must be finite and positive");
    }

    // --- finiteness (Brief section 8 InvalidInput) ---
    if (!IsFinite(profile.origin) || !IsFinite(profile.u_axis) || !IsFinite(profile.v_axis) ||
        !IsFinite(profile.size_u) || !IsFinite(profile.size_v) || !IsFinite(spec.direction) ||
        !IsFinite(spec.distance)) {
        return ErrorResult(GeometryErrorCode::InvalidInput,
                           "MakeLinearExtrusion: non-finite (NaN/infinite) numeric input");
    }

    // --- zero/negative physical dimensions and distance -> InvalidInput
    //     (Brief section 8; corpus F01 zero distance, F03 negative size) ---
    if (profile.size_u <= 0.0 || profile.size_v <= 0.0 || spec.distance <= 0.0) {
        return ErrorResult(GeometryErrorCode::InvalidInput,
                           "MakeLinearExtrusion: size_u/size_v/distance must be strictly positive");
    }

    // --- non-degenerate axes/direction -> DegenerateGeometry
    //     (Brief section 8: "vector magnitude at or below supplied linear
    //     tolerance"; corpus F02 zero direction) ---
    const double u_mag = Magnitude(profile.u_axis);
    const double v_mag = Magnitude(profile.v_axis);
    const double dir_mag = Magnitude(spec.direction);
    if (u_mag <= tolerance.linear || v_mag <= tolerance.linear || dir_mag <= tolerance.linear) {
        return ErrorResult(GeometryErrorCode::DegenerateGeometry,
                           "MakeLinearExtrusion: u_axis/v_axis/direction magnitude at or below "
                           "the supplied linear tolerance");
    }

    return ContainOcctExceptions("MakeLinearExtrusion", [&]() -> SolidResult {
        // Normalize u/v into gp_Dir and test orthogonality with gp_Dir::IsNormal
        // using the CALLER-SUPPLIED angular tolerance (Amendment 01 AA-C02:
        // "The orthogonality comparison must use normalized direction vectors
        // and an angularly meaningful comparison. Raw, unnormalized dot-product
        // magnitude must not be compared directly to an angular tolerance.").
        // gp_Dir's own constructor additionally raises Standard_ConstructionError
        // on a near-zero-magnitude input; that is redundant with the magnitude
        // check above but is left as defense-in-depth and is caught by
        // ContainOcctExceptions.
        const gp_Dir u_dir(profile.u_axis.x, profile.u_axis.y, profile.u_axis.z);
        const gp_Dir v_dir(profile.v_axis.x, profile.v_axis.y, profile.v_axis.z);
        if (!u_dir.IsNormal(v_dir, tolerance.angular_radians)) {
            // Skew axes are NEVER silently rotated/projected/orthogonalized
            // (Amendment 01 AA-C02) - they are reported as DegenerateGeometry.
            return ErrorResult(GeometryErrorCode::DegenerateGeometry,
                               "MakeLinearExtrusion: u_axis/v_axis are not orthogonal within "
                               "the supplied angular tolerance");
        }

        // Build the rectangle's four corners from the ORIGINAL (unnormalized
        // magnitude, normalized direction) axes: corner = origin + dir * size.
        const gp_Pnt origin = ToOcctPoint(profile.origin);
        const gp_Pnt p0 = origin;
        const gp_Pnt p1(origin.X() + u_dir.X() * profile.size_u,
                        origin.Y() + u_dir.Y() * profile.size_u,
                        origin.Z() + u_dir.Z() * profile.size_u);
        const gp_Pnt p2(p1.X() + v_dir.X() * profile.size_v, p1.Y() + v_dir.Y() * profile.size_v,
                        p1.Z() + v_dir.Z() * profile.size_v);
        const gp_Pnt p3(origin.X() + v_dir.X() * profile.size_v,
                        origin.Y() + v_dir.Y() * profile.size_v,
                        origin.Z() + v_dir.Z() * profile.size_v);

        BRepBuilderAPI_MakePolygon polygon_builder;
        polygon_builder.Add(p0);
        polygon_builder.Add(p1);
        polygon_builder.Add(p2);
        polygon_builder.Add(p3);
        polygon_builder.Close();
        if (!polygon_builder.IsDone()) {
            return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                               "MakeLinearExtrusion: BRepBuilderAPI_MakePolygon did not complete");
        }

        BRepBuilderAPI_MakeFace face_builder(polygon_builder.Wire());
        if (!face_builder.IsDone()) {
            return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                               "MakeLinearExtrusion: BRepBuilderAPI_MakeFace did not complete");
        }

        const gp_Dir extrude_dir(spec.direction.x, spec.direction.y, spec.direction.z);
        const gp_Vec extrude_vec(extrude_dir.X() * spec.distance, extrude_dir.Y() * spec.distance,
                                 extrude_dir.Z() * spec.distance);

        BRepPrimAPI_MakePrism prism_builder(face_builder.Face(), extrude_vec);
        if (!prism_builder.IsDone()) {
            return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                               "MakeLinearExtrusion: BRepPrimAPI_MakePrism did not complete");
        }

        return ValidateAndWrapSolid(prism_builder.Shape(), "MakeLinearExtrusion",
                                    /*require_single_solid=*/true);
    });
}

// ============================================================================
// Cut (Brief section 7.2; Amendment 01 AA-C03)
// ============================================================================
SolidResult Cut(const SolidHandle& host, const SolidHandle& tool,
                const GeometryTolerance& tolerance) {
    if (host == nullptr || tool == nullptr) {
        return ErrorResult(GeometryErrorCode::InvalidInput, "Cut: null solid handle");
    }
    if (!ToleranceIsUsable(tolerance)) {
        return ErrorResult(GeometryErrorCode::InvalidInput,
                           "Cut: tolerance.linear/angular_radians must be finite and positive");
    }

    return ContainOcctExceptions("Cut", [&]() -> SolidResult {
        // --- AA-C03 qualification: Common(host, tool) -> valid 3D common
        //     content -> common volume > volume_epsilon = linear_tolerance^3.
        //     This is a SEPARATE API surface from AA-C04's Fuse qualification
        //     below (BRepExtrema_DistShapeShape) - the two are never merged,
        //     per the explicit instruction not to conflate their semantics. ---
        BRepAlgoAPI_Common qualifier(host->shape, tool->shape);
        qualifier.SetFuzzyValue(tolerance.linear);
        qualifier.Build();
        if (qualifier.HasErrors()) {
            return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                               "Cut: Common() qualification kernel reported errors");
        }

        const TopoDS_Shape common_shape = qualifier.Shape();
        bool has_solid_content = false;
        if (!common_shape.IsNull()) {
            for (TopExp_Explorer explorer(common_shape, TopAbs_SOLID); explorer.More();
                 explorer.Next()) {
                has_solid_content = true;
                break;
            }
        }

        double common_volume = 0.0;
        if (has_solid_content) {
            GProp_GProps props;
            BRepGProp::VolumeProperties(common_shape, props);
            common_volume = props.Mass();
        }

        const double volume_epsilon = tolerance.linear * tolerance.linear * tolerance.linear;
        if (!has_solid_content || !IsFinite(common_volume) || common_volume <= volume_epsilon) {
            // No 3D common content, or boundary-only zero-volume contact
            // (corpus O02, O04): NoIntersection, never disguised as a
            // kernel failure (Brief section 7.2).
            return ErrorResult(GeometryErrorCode::NoIntersection,
                               "Cut: no qualifying volumetric overlap between host and tool");
        }

        // --- positive qualifying volume: attempt the actual Cut ---
        BRepAlgoAPI_Cut cut_algo(host->shape, tool->shape);
        cut_algo.SetFuzzyValue(tolerance.linear);
        cut_algo.Build();
        if (cut_algo.HasErrors()) {
            return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                               "Cut: Boolean Cut kernel reported errors");
        }

        return ValidateAndWrapSolid(cut_algo.Shape(), "Cut", /*require_single_solid=*/false);
    });
}

// ============================================================================
// Fuse (Brief section 7.3; Amendment 01 AA-C04)
// ============================================================================
SolidResult Fuse(const SolidHandle& a, const SolidHandle& b, const GeometryTolerance& tolerance) {
    if (a == nullptr || b == nullptr) {
        return ErrorResult(GeometryErrorCode::InvalidInput, "Fuse: null solid handle");
    }
    if (!ToleranceIsUsable(tolerance)) {
        return ErrorResult(GeometryErrorCode::InvalidInput,
                           "Fuse: tolerance.linear/angular_radians must be finite and positive");
    }

    return ContainOcctExceptions("Fuse", [&]() -> SolidResult {
        // --- AA-C04 qualification: verified geometric separation/proximity
        //     via BRepExtrema_DistShapeShape - deliberately NOT common
        //     volume (that would incorrectly reject face-contact case J04).
        //     distance > linear_tolerance -> NoIntersection;
        //     distance <= linear_tolerance -> attempt Fuse. ---
        BRepExtrema_DistShapeShape distance_tool(a->shape, b->shape);
        if (!distance_tool.IsDone()) {
            return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                               "Fuse: BRepExtrema_DistShapeShape qualification did not complete");
        }
        const double separation = distance_tool.Value();
        if (!IsFinite(separation)) {
            return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                               "Fuse: BRepExtrema_DistShapeShape returned a non-finite distance");
        }
        if (separation > tolerance.linear) {
            return ErrorResult(GeometryErrorCode::NoIntersection,
                               "Fuse: minimum separation exceeds the supplied linear tolerance");
        }

        // --- separation <= tolerance: attempt the actual Fuse ---
        BRepAlgoAPI_Fuse fuse_algo(a->shape, b->shape);
        fuse_algo.SetFuzzyValue(tolerance.linear);
        fuse_algo.Build();
        if (fuse_algo.HasErrors()) {
            return ErrorResult(GeometryErrorCode::KernelOperationFailed,
                               "Fuse: Boolean Fuse kernel reported errors");
        }

        // Connected-union success requires exactly one resulting 3D solid
        // (Amendment 01 AA-C04 point 4); more than one -> InvalidResult.
        return ValidateAndWrapSolid(fuse_algo.Shape(), "Fuse", /*require_single_solid=*/true);
    });
}

// ============================================================================
// Inspect (Brief section 7.4)
// ============================================================================
MetricsResult Inspect(const SolidHandle& solid) {
    if (solid == nullptr) {
        return MetricsResult{SolidMetrics{}, MakeError(GeometryErrorCode::InvalidInput,
                                                       "Inspect: null solid handle")};
    }

    try {
        const TopoDS_Shape& shape = solid->shape;

        SolidMetrics metrics;

        BRepCheck_Analyzer analyzer(shape);
        metrics.valid = analyzer.IsValid();

        GProp_GProps props;
        BRepGProp::VolumeProperties(shape, props);
        metrics.volume = props.Mass();

        Bnd_Box bounds;
        BRepBndLib::Add(shape, bounds);
        if (!bounds.IsVoid()) {
            const gp_Pnt min_corner = bounds.CornerMin();
            const gp_Pnt max_corner = bounds.CornerMax();
            metrics.bounding_box_min = Point3{min_corner.X(), min_corner.Y(), min_corner.Z()};
            metrics.bounding_box_max = Point3{max_corner.X(), max_corner.Y(), max_corner.Z()};
        }

        for (TopExp_Explorer explorer(shape, TopAbs_SOLID); explorer.More(); explorer.Next()) {
            ++metrics.solid_count;
        }
        for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
            ++metrics.face_count;
        }
        for (TopExp_Explorer explorer(shape, TopAbs_EDGE); explorer.More(); explorer.Next()) {
            ++metrics.edge_count;
        }

        return MetricsResult{metrics, MakeError(GeometryErrorCode::None, "")};
    } catch (const Standard_Failure& failure) {
        const char* what = failure.what();
        return MetricsResult{SolidMetrics{},
                             MakeError(GeometryErrorCode::KernelOperationFailed,
                                       std::string("Inspect: OCCT Standard_Failure: ") +
                                           (what != nullptr ? what : "<no message>"))};
    } catch (const std::exception& ex) {
        return MetricsResult{SolidMetrics{},
                             MakeError(GeometryErrorCode::KernelOperationFailed,
                                       std::string("Inspect: std::exception: ") + ex.what())};
    } catch (...) {
        return MetricsResult{SolidMetrics{}, MakeError(GeometryErrorCode::KernelOperationFailed,
                                                       "Inspect: unknown exception")};
    }
}

} // namespace bim::geometry_api
