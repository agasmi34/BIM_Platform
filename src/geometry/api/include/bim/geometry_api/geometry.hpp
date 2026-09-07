#pragma once

#include <memory>
#include <string>

namespace bim::geometry_api {

// P0-T002 OCCT Geometry Spike - project-owned, OCCT-free public geometry
// contract (Implementation Brief BIM-TASK-P0-T002-CLAUDE v1.0 section 6;
// Amendment 01 BIM-TASK-P0-T002-CLAUDE-A01 AA-C02/AA-C05/AA-C08).
//
// This header must never include, forward-declare, alias or otherwise
// reference an OCCT type (TopoDS_*, gp_*, BRep*, Handle(...), ...). The
// architecture checker's GEOMETRY_API_NO_OCCT_LEAK rule
// (tools/architecture_checker.cmake, Amendment 01 AA-C09) enforces this
// mechanically across the whole src/geometry/api production surface.
//
// This is NOT a general-purpose CAD kernel wrapper and introduces no BIM
// semantics (Implementation Brief section 25): no Wall/Slab/Door/Window/
// Room/HostedElement/Level/Grid type appears anywhere below.

// --- 6.1 GeometryErrorCode --------------------------------------------------
// No OCCT error code may appear here (Brief section 6.1). This is the sole
// machine-readable failure classification for the public geometry contract.
// Keep the default enum representation for this Phase-0 Geometry API contract;
// changing the underlying representation is outside P0-T002.
enum class GeometryErrorCode { // NOLINT(performance-enum-size)
    None,
    InvalidInput,
    DegenerateGeometry,
    NoIntersection,
    KernelOperationFailed,
    InvalidResult,
    UnsupportedOperation
};

// --- 6.2 GeometryError -------------------------------------------------------
// message is diagnostic only (Brief sections 6.2 and 29): callers must
// branch on `code`, never parse or compare `message`.
struct GeometryError {
    GeometryErrorCode code = GeometryErrorCode::None;
    std::string message;

    [[nodiscard]] bool ok() const noexcept { return code == GeometryErrorCode::None; }
};

// --- 6.3 Basic neutral types -------------------------------------------------
struct Point3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct Vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// --- 6.4 GeometryTolerance ----------------------------------------------------
// Deliberately has NO default value (Brief section 6.4): P0-T002 exists to
// collect the evidence Architecture Authority needs to choose a future
// platform-wide tolerance default (final decision D2-B, Brief section 39).
// Hard-coding a default here would presupply that answer. Every call site in
// this candidate (tests and the evidence executable) supplies an explicit
// GeometryTolerance value.
struct GeometryTolerance {
    double linear;
    double angular_radians;
};

// --- 6.5 RectangleProfile3 ----------------------------------------------------
// Rules (Brief section 6.5; Amendment 01 AA-C02): u_axis/v_axis finite and
// non-degenerate, orthogonal within the caller-supplied angular tolerance
// (not merely non-parallel), size_u/size_v finite and strictly positive. The
// adapter (geometry_occt_adapter.cpp) normalizes axes internally for the
// orthogonality test and NEVER silently rotates, projects or orthogonalizes
// a skewed caller-supplied axis pair - a finite-but-skew pair is classified
// DegenerateGeometry, not silently corrected.
struct RectangleProfile3 {
    Point3 origin;
    Vector3 u_axis;
    Vector3 v_axis;
    double size_u = 0.0;
    double size_v = 0.0;
};

// --- 6.6 LinearExtrusionSpec --------------------------------------------------
struct LinearExtrusionSpec {
    RectangleProfile3 profile;
    Vector3 direction;
    double distance = 0.0;
};

// --- 6.7 Opaque solid ownership -----------------------------------------------
// `Solid` is deliberately incomplete in every public geometry_api header.
// Its complete definition exists only in the adapter-private
// src/geometry/occt/src/solid_impl.hpp, which is never included by, nor
// reachable from, any header under src/geometry/api/. No accessor to the
// underlying OCCT TopoDS_Shape is declared anywhere in this file (Brief
// section 6.7; Amendment 01 AA-C05: "No public caller may retrieve or
// mutate a TopoDS_Shape or any other OCCT object.").
struct Solid;
using SolidHandle = std::shared_ptr<const Solid>;

// --- 6.8 SolidMetrics ----------------------------------------------------------
// Diagnostic only (Brief section 6.8). solid_count/face_count/edge_count are
// NOT an identity contract: they must never be treated as persistent
// topology identity (Brief section 19; Amendment 01 AA-C06). Persistent
// topology reference design remains out of scope for P0-T002 (P0-T008).
struct SolidMetrics {
    bool valid = false;
    double volume = 0.0;
    Point3 bounding_box_min;
    Point3 bounding_box_max;
    int solid_count = 0;
    int face_count = 0;
    int edge_count = 0;
};

// --- 6.9 Operation results ------------------------------------------------------
// Accepted result-type names per Amendment 01 AA-C05/AA-C06 disposition.
struct SolidResult {
    SolidHandle solid;
    GeometryError error;

    [[nodiscard]] bool ok() const noexcept { return error.ok() && solid != nullptr; }
};

struct MetricsResult {
    SolidMetrics metrics;
    GeometryError error;

    [[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

// --- 6.10 Required public operations ---------------------------------------------
// Implemented exclusively in bim_geometry_occt
// (src/geometry/occt/src/geometry_occt_adapter.cpp). No third-party
// (Standard_Failure/std::exception/unknown) exception ever crosses this
// boundary (Brief section 29): every OCCT/std exception is caught and
// translated to a GeometryError entirely inside the adapter.
[[nodiscard]] SolidResult MakeLinearExtrusion(const LinearExtrusionSpec& spec,
                                              const GeometryTolerance& tolerance);

// Cut(host, tool, tolerance): volumetric Boolean subtraction (Brief section
// 7.2; Amendment 01 AA-C03). A NoIntersection result must not be disguised
// as a kernel failure, and a kernel failure must not be disguised as
// NoIntersection.
[[nodiscard]] SolidResult Cut(const SolidHandle& host, const SolidHandle& tool,
                              const GeometryTolerance& tolerance);

// Fuse(a, b, tolerance): connected solid union (Brief section 7.3; Amendment
// 01 AA-C04). Qualification uses geometric separation/proximity, never
// common volume (that would incorrectly reject face-contact case J04).
[[nodiscard]] SolidResult Fuse(const SolidHandle& a, const SolidHandle& b,
                               const GeometryTolerance& tolerance);

// Inspect(solid): neutral geometry metrics without exposing any OCCT object
// (Brief section 7.4).
[[nodiscard]] MetricsResult Inspect(const SolidHandle& solid);

} // namespace bim::geometry_api
