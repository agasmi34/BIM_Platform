// Proves clean first-party rejection of source_path == export_path
// (Execution Packet BIM-AA-P0-T006 v1.0 section 11: "input path equal to
// output path" - the original fixture must never be written in place,
// section 7; AA CP2B-H3 clarification's "integration_dwg_same_path_rejected"),
// exercised through the full public contract (RunDwgRoundTripProbe) against
// the real locked vendor fixture path, WITHOUT ever performing any write -
// dwg_probe.cpp's RejectUnsafeRoundTripInputs() rejects this case before
// any detail::/ODA call is made.

#include "bim/dwg/probe.hpp"
#include "dwg_test_support.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {
using bim::dwg::DwgRoundTripEvidence;
using bim::dwg::RunDwgRoundTripProbe;
using bim::dwg::test_support::VendorFixturePath;
} // namespace

TEST_CASE("a round trip whose export_path equals its source_path is rejected before any write",
          "[integration][dwg]") {
    const auto fixture_path = VendorFixturePath();

    DwgRoundTripEvidence evidence;
    const bim::foundation::Status status = RunDwgRoundTripProbe(fixture_path, fixture_path, evidence);

    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(status.message().empty());

    // The up-front rejection must happen before any mechanical step of the
    // round trip - every evidence field stays at its default-constructed
    // "no accidental pass" value (Packet section 11: no write of any kind).
    CHECK_FALSE(evidence.read_success);
    CHECK_FALSE(evidence.write_success);
    CHECK_FALSE(evidence.reopen_success);
    CHECK_FALSE(evidence.overall_passed);
}
