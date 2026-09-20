// Proves the neutral, first-party shape of bim::ifc's public probe contract
// (Implementation Brief BIM-TASK-P0-T005-CLAUDE v1.0 section 8; Execution
// Packet v1.1 section 9's "unit_ifc_probe_contract"). Links bim::ifc (which
// requires BIM_ENABLE_IFC and a bootstrapped IfcOpenShell to even compile -
// see src/interop/ifc/CMakeLists.txt) but every assertion here concerns only
// the public contract's own values/shape, never an IfcOpenShell type or
// exercising the private adapter's file I/O (that is
// integration_ifc_round_trip's job).

#include "bim/ifc/probe.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
using bim::ifc::MakeDefaultSeed;
using bim::ifc::RoundTripEvidence;
} // namespace

TEST_CASE("MakeDefaultSeed produces a well-formed, non-empty seed", "[unit][ifc]") {
    const auto seed = MakeDefaultSeed();

    REQUIRE_FALSE(seed.project_global_id.empty());
    REQUIRE_FALSE(seed.project_name.empty());
    REQUIRE_FALSE(seed.element_global_id.empty());
    REQUIRE_FALSE(seed.element_name.empty());
    REQUIRE_FALSE(seed.element_object_type.empty());
    REQUIRE_FALSE(seed.scalar_properties.empty());

    // Compressed IFC GlobalId literals are always exactly 22 characters
    // (probe.hpp's IfcProjectSeed doc comment).
    REQUIRE(seed.project_global_id.size() == 22);
    REQUIRE(seed.element_global_id.size() == 22);
    REQUIRE(seed.project_global_id != seed.element_global_id);
}

TEST_CASE("MakeDefaultSeed is deterministic across calls", "[unit][ifc]") {
    const auto first = MakeDefaultSeed();
    const auto second = MakeDefaultSeed();

    REQUIRE(first.project_global_id == second.project_global_id);
    REQUIRE(first.project_name == second.project_name);
    REQUIRE(first.element_global_id == second.element_global_id);
    REQUIRE(first.element_name == second.element_name);
    REQUIRE(first.element_object_type == second.element_object_type);
    REQUIRE(first.scalar_properties == second.scalar_properties);
}

TEST_CASE("a default-constructed RoundTripEvidence carries no accidental pass", "[unit][ifc]") {
    const RoundTripEvidence evidence;

    REQUIRE(evidence.schema_expected.empty());
    REQUIRE(evidence.schema_reopened.empty());
    REQUIRE_FALSE(evidence.export_success);
    REQUIRE_FALSE(evidence.reopen_success);
    REQUIRE_FALSE(evidence.project_found);
    REQUIRE_FALSE(evidence.project_identity_preserved);
    REQUIRE_FALSE(evidence.project_name_preserved);
    REQUIRE_FALSE(evidence.element_found);
    REQUIRE_FALSE(evidence.element_global_id_preserved);
    REQUIRE_FALSE(evidence.element_name_preserved);
    REQUIRE_FALSE(evidence.element_object_type_preserved);
    REQUIRE_FALSE(evidence.scalar_properties_preserved);
    REQUIRE_FALSE(evidence.overall_passed);
}
