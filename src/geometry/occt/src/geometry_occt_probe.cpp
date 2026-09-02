#include "bim/geometry_api/probe.hpp"

#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

#include <string>

namespace bim::geometry_api {

// Implementation lives ONLY here (bim_geometry_occt). No OCCT type crosses
// this translation unit's boundary: the function signature and return type
// are both defined in bim_geometry_api, which never includes an OCCT header.
ProbeResult RunKernelSmokeProbe() {
    try {
        // Construct one trivial OCCT solid entirely internally, per
        // Implementation Brief Phase I.
        BRepPrimAPI_MakeBox box_builder(2.0, 3.0, 4.0);
        const TopoDS_Shape box = box_builder.Shape();

        GProp_GProps props;
        BRepGProp::VolumeProperties(box, props);
        const double volume = props.Mass();

        return ProbeResult{bim::foundation::Status::Ok(), volume};
    } catch (const Standard_Failure& failure) {
        const char* what = failure.what();
        return ProbeResult{
            bim::foundation::Status::Error(std::string("OCCT Standard_Failure: ") +
                                           (what != nullptr ? what : "<no message>")),
            0.0};
    } catch (const std::exception& ex) {
        return ProbeResult{
            bim::foundation::Status::Error(std::string("std::exception: ") + ex.what()), 0.0};
    } catch (...) {
        return ProbeResult{
            bim::foundation::Status::Error("unknown exception in RunKernelSmokeProbe"), 0.0};
    }
}

} // namespace bim::geometry_api
