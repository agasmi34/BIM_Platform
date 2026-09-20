#include "bim/ifc/probe.hpp"

#include "detail/openshell_adapter.hpp"

namespace bim::ifc {

IfcProjectSeed MakeDefaultSeed() {
    IfcProjectSeed seed;

    // Fixed, first-party-chosen 22-character compressed IFC GlobalId
    // literals (see probe.hpp's IfcProjectSeed doc comment). Both are
    // well-formed compressed-GUID strings (every character is drawn from
    // the IFC GUID compression alphabet 0-9/A-Z/a-z/_/$) and are distinct
    // from each other.
    seed.project_global_id = "1sD4wA9r5PzR8fN$Q2Kx7Y";
    seed.project_name = "P0-T005 IFC Spike Project";

    seed.element_global_id = "2xT9mK3vQ8hL$B6Zw4Yc1J";
    seed.element_name = "P0-T005 Spike Element";
    seed.element_object_type = "P0T005_SPIKE_PROXY";

    seed.scalar_properties = {
        {"spike_id", "P0-T005"},
        {"deterministic", "true"},
        {"schema", "IFC4"},
    };

    return seed;
}

bim::foundation::Status RunIfcRoundTripProbe(const IfcProjectSeed& seed,
                                             const std::filesystem::path& export_path,
                                             RoundTripEvidence& out_evidence) {
    out_evidence = RoundTripEvidence{};
    out_evidence.schema_expected = "IFC4";

    bim::foundation::Status export_status = detail::WriteSeedToIfc4File(seed, export_path);
    out_evidence.export_success = export_status.ok();
    if (!export_status.ok()) {
        return export_status;
    }

    return detail::ReadIfc4RoundTrip(export_path, seed, out_evidence);
}

bim::foundation::Status OpenAndValidateIfcFile(const std::filesystem::path& path) {
    std::string schema;
    return detail::ParseAndCheckIfc4Schema(path, schema);
}

} // namespace bim::ifc
