// Proves the controlled positive IFC4 round trip (Implementation Brief
// BIM-TASK-P0-T005-CLAUDE v1.0 sections 9-10; Execution Packet v1.1 section
// 9's "integration_ifc_round_trip"): construct seed -> export -> close ->
// reopen -> locate project/element -> compare every required invariant.

#include "bim/ifc/probe.hpp"
#include "ifc_test_support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {
using bim::ifc::MakeDefaultSeed;
using bim::ifc::RoundTripEvidence;
using bim::ifc::RunIfcRoundTripProbe;
using bim::ifc::test_support::MakeTempIfcPath;
} // namespace

TEST_CASE("the controlled IFC4 round trip preserves every required invariant",
          "[integration][ifc]") {
    const auto seed = MakeDefaultSeed();
    const std::filesystem::path export_path = MakeTempIfcPath("round_trip");

    RoundTripEvidence evidence;
    const bim::foundation::Status status = RunIfcRoundTripProbe(seed, export_path, evidence);

    INFO("status: " << status.message());
    REQUIRE(status.ok());

    // Brief section 10's exact required-invariant list.
    CHECK(evidence.schema_expected == "IFC4");
    CHECK(evidence.schema_reopened == "IFC4");
    CHECK(evidence.export_success);
    CHECK(evidence.reopen_success);
    CHECK(evidence.project_found);
    CHECK(evidence.project_identity_preserved);
    CHECK(evidence.project_name_preserved);
    CHECK(evidence.element_found);
    CHECK(evidence.element_global_id_preserved);
    CHECK(evidence.element_name_preserved);
    CHECK(evidence.element_object_type_preserved);
    CHECK(evidence.scalar_properties_preserved);
    CHECK(evidence.overall_passed);

    std::error_code ec;
    std::filesystem::remove(export_path, ec);
}
