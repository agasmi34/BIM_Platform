#include "bim/geometry_api/face_observation.hpp"

#include "solid_impl.hpp"

#include <BRepAdaptor_Surface.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>

#include <exception>
#include <string>

// P0-T008 Topological Reference Spike - kernel-owning implementation of
// bim::geometry_api::ObserveFaces() (Implementation Brief
// BIM-TASK-P0-T008-CLAUDE v1.0; Architecture Gate 01-ARCHITECTURE-GATE.md).
// This translation unit is the only place authorized to translate a real
// kernel face into the neutral FaceObservation contract - solid_impl.hpp's
// own header comment already restricts the complete Solid definition to
// this adapter's translation units, and this file adds no new escape route
// (no raw kernel shape ever crosses into bim::geometry_api's public types
// here).
//
// PROVISIONAL API NOTE (mirrors spike_diagnostics.cpp's own disclosure
// style): BRepAdaptor_Surface::GetType() / ::Plane(), BRep_Tool-adjacent
// GProp_GProps::CentreOfMass()/Mass(), and gp_Pln::Axis() are used below on
// the strength of well-established, long-stable OCCT surface/adaptor API
// shape, but - unlike BRepGProp::SurfaceProperties() and the
// TopExp_Explorer face-walk pattern, both already exercised by
// geometry_occt_adapter.cpp/spike_diagnostics.cpp in this same tree this
// session - BRepAdaptor_Surface.hxx, GeomAbs_SurfaceType.hxx, and
// gp_Pln.hxx/gp_Dir.hxx were NOT independently read from the supplied OCCT
// 8.0.1 header set this session. This is disclosed as UNVERIFIED rather
// than silently assumed; the Windows Execution Operator's real MSVC compile
// against the locked OCCT 8.0.1 install is this file's first actual
// verification. If compilation shows any signature here wrong, the fix is
// localized to this file - no other translation unit depends on these
// specific calls.

namespace bim::geometry_api {

namespace {

[[nodiscard]] FaceObservation ObserveOneFace(const TopoDS_Face& face) {
    FaceObservation observation;

    GProp_GProps props;
    BRepGProp::SurfaceProperties(face, props);
    observation.area = props.Mass();
    const gp_Pnt centroid = props.CentreOfMass();
    observation.point_on_surface = Point3{centroid.X(), centroid.Y(), centroid.Z()};

    const BRepAdaptor_Surface adaptor(face, true);
    if (adaptor.GetType() == GeomAbs_Plane) {
        observation.surface_kind = SurfaceKind::Planar;
        const gp_Pln plane = adaptor.Plane();
        gp_Dir normal_dir = plane.Axis().Direction();
        // A TopoDS_Face carries an orientation independent of the
        // underlying surface's own analytic normal; a Reversed face's
        // outward normal is the geometric surface normal flipped. Ignoring
        // this would silently pair some faces with an inward instead of
        // outward normal - harmless for this spike's sign-agnostic
        // parallel check, but wrong as a general-purpose observation, so it
        // is handled correctly here rather than left as a known gap.
        if (face.Orientation() == TopAbs_REVERSED) {
            normal_dir.Reverse();
        }
        observation.normal = Vector3{normal_dir.X(), normal_dir.Y(), normal_dir.Z()};
    } else {
        observation.surface_kind = SurfaceKind::NonPlanar;
        observation.normal = Vector3{0.0, 0.0, 0.0};
    }

    return observation;
}

} // namespace

FaceObservationResult ObserveFaces(const SolidHandle& solid) {
    FaceObservationResult result;

    if (solid == nullptr) {
        result.error = GeometryError{GeometryErrorCode::InvalidInput, "ObserveFaces: null solid handle"};
        return result;
    }

    try {
        const TopoDS_Shape& shape = solid->shape;
        for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
            const TopoDS_Face face = TopoDS::Face(explorer.Current());
            result.faces.push_back(ObserveOneFace(face));
        }
    } catch (const Standard_Failure& failure) {
        const char* what = failure.what();
        result.faces.clear();
        result.error = GeometryError{GeometryErrorCode::KernelOperationFailed,
                                     std::string("ObserveFaces: OCCT Standard_Failure: ") +
                                         (what != nullptr ? what : "<no message>")};
    } catch (const std::exception& ex) {
        result.faces.clear();
        result.error = GeometryError{GeometryErrorCode::KernelOperationFailed,
                                     std::string("ObserveFaces: std::exception: ") + ex.what()};
    } catch (...) {
        result.faces.clear();
        result.error =
            GeometryError{GeometryErrorCode::KernelOperationFailed, "ObserveFaces: unknown exception"};
    }

    return result;
}

} // namespace bim::geometry_api
