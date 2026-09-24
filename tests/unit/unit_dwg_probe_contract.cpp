// Proves the neutral, first-party shape of bim::dwg's public probe contract
// (Execution Packet BIM-AA-P0-T006 v1.0 sections 6, 10; AA CP2B-H3
// clarification's "unit_dwg_probe_contract"). Links bim::dwg (which
// requires BIM_ENABLE_DWG and the locked ODA Drawings SDK to even compile -
// see src/interop/dwg/CMakeLists.txt) but every assertion here concerns
// only the public DwgRoundTripEvidence struct's own default-constructed
// shape and OpenAndValidateDwgFile's up-front path contract, never an ODA
// type or the private adapter's file I/O against the vendor fixture (that
// is integration_dwg_same_version_round_trip's / integration_dwg_missing_file's
// job).

#include "bim/dwg/probe.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
using bim::dwg::DwgRoundTripEvidence;
using bim::dwg::OpenAndValidateDwgFile;
} // namespace

TEST_CASE("a default-constructed DwgRoundTripEvidence carries no accidental pass", "[unit][dwg]") {
    const DwgRoundTripEvidence evidence;

    REQUIRE(evidence.source_version.empty());
    REQUIRE(evidence.reopened_version.empty());
    REQUIRE_FALSE(evidence.read_success);
    REQUIRE_FALSE(evidence.write_success);
    REQUIRE_FALSE(evidence.reopen_success);
    REQUIRE_FALSE(evidence.source_mtext_found);
    REQUIRE_FALSE(evidence.reopened_mtext_found);
    REQUIRE_FALSE(evidence.text_content_preserved);
    REQUIRE_FALSE(evidence.position_preserved);
    REQUIRE_FALSE(evidence.normal_preserved);
    REQUIRE_FALSE(evidence.direction_preserved);
    REQUIRE_FALSE(evidence.text_height_preserved);
    REQUIRE_FALSE(evidence.width_preserved);
    REQUIRE_FALSE(evidence.attachment_preserved);
    REQUIRE_FALSE(evidence.text_style_preserved);
    REQUIRE_FALSE(evidence.overall_passed);
}

TEST_CASE("OpenAndValidateDwgFile rejects a path that does not exist, via the first-party contract alone",
          "[unit][dwg]") {
    // A plain first-party-only contract check (relies only on
    // std::filesystem::exists() failing before any ODA call is made -
    // probe.hpp's own documented up-front rejection), kept narrowly scoped
    // here rather than in the integration suite (which exercises the same
    // missing-input contract through the full vendor-fixture-adjacent path
    // in integration_dwg_missing_file), mirroring how P0-T005 kept a
    // narrowly-scoped positive-shape assertion in its own unit test.
    std::string version;
    const bim::foundation::Status status = OpenAndValidateDwgFile(
        "bim_p0_t006_this_path_is_never_created_by_any_test.dwg", version);

    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(status.message().empty());
    REQUIRE(version.empty());
}
