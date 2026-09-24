// Proves the controlled positive DWG round trip against the locked vendor
// fixture (Execution Packet BIM-AA-P0-T006 v1.0 sections 6, 9, 10; AA
// CP2B-H3 clarification's "integration_dwg_same_version_round_trip"):
// AC1018 -> explicit OdDb::vAC18 -> AC1018, with semantic MText
// verification around "External Reference". This is the test that
// actually exercises the full production read -> write -> reopen sequence
// through real ODA calls end to end - the genuine executable-level proof
// of ODA link closure and runtime/module resolution the static-library
// compile check (CP2B-H3) could not itself provide.
//
// Per Execution Packet BIM-AA-P0-T006 v1.0 and its AA clarification
// response: binary equality, DWG handle equality, and object-count
// equality are deliberately never used as acceptance criteria here - every
// check below is either a first-party Status/bool outcome or a
// property-level semantic comparison already performed inside
// RunControlledRoundTrip() (text/position/normal/direction/height/width/
// attachment/resolved-text-style-name, each compared with the fixture-local
// 1e-9 absolute tolerance for floating-point fields - see
// src/interop/dwg/src/detail/oda_adapter.cpp's own CompareSnapshots()). The
// original locked fixture is never modified in place: `source_path` and
// `export_path` are always distinct, and export_path is always a fresh,
// disposable temp path removed at the end of this test.

#include "bim/dwg/probe.hpp"
#include "dwg_test_support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {
using bim::dwg::DwgRoundTripEvidence;
using bim::dwg::RunDwgRoundTripProbe;
using bim::dwg::test_support::MakeTempDwgPath;
using bim::dwg::test_support::VendorFixturePath;
} // namespace

TEST_CASE("the controlled AC1018 -> AC18 -> AC1018 round trip preserves every required MText invariant",
          "[integration][dwg]") {
    const auto source_path = VendorFixturePath();
    const std::filesystem::path export_path = MakeTempDwgPath("same_version_round_trip");

    // Packet section 7 / section 11: the original fixture is never
    // overwritten - export_path must not already exist before the probe
    // runs, and must be a different path than source_path.
    REQUIRE_FALSE(std::filesystem::exists(export_path));
    REQUIRE(source_path != export_path);

    DwgRoundTripEvidence evidence;
    const bim::foundation::Status status = RunDwgRoundTripProbe(source_path, export_path, evidence);

    INFO("status: " << status.message());
    REQUIRE(status.ok());

    // AC1018 source/reopen evidence (Packet section 9: explicit
    // OdDb::vAC18 target version on write, never "current"; both the
    // source and the reopened export must report AC1018).
    CHECK(evidence.source_version == "AC1018");
    CHECK(evidence.reopened_version == "AC1018");

    // Every mandatory mechanical step succeeded.
    CHECK(evidence.read_success);
    CHECK(evidence.write_success);
    CHECK(evidence.reopen_success);

    // Semantic MText evidence: the known user-visible "External Reference"
    // MTEXT was located, semantically (by content), in both the source and
    // the reopened export - never by a hardcoded DWG handle (Packet section
    // 10).
    CHECK(evidence.source_mtext_found);
    CHECK(evidence.reopened_mtext_found);

    // Every required property-level invariant survived the round trip.
    CHECK(evidence.text_content_preserved);
    CHECK(evidence.position_preserved);
    CHECK(evidence.normal_preserved);
    CHECK(evidence.direction_preserved);
    CHECK(evidence.text_height_preserved);
    CHECK(evidence.width_preserved);
    CHECK(evidence.attachment_preserved);
    CHECK(evidence.text_style_preserved);

    CHECK(evidence.overall_passed);

    // The original locked fixture is untouched by this test: only the
    // disposable export_path was ever written, and it is cleaned up here.
    std::error_code ec;
    std::filesystem::remove(export_path, ec);
}
