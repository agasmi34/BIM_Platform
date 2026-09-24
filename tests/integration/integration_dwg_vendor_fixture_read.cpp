// Proves the locked, vendor-supplied controlled fixture ("OdWriteEx
// XRef.dwg" - Execution Packet BIM-AA-P0-T006 v1.0 section 8) opens and
// validates cleanly through the production public contract
// (OpenAndValidateDwgFile), reporting exactly the expected AC1018 source
// version, WITHOUT performing any write (AA CP2B-H3 clarification's
// "integration_dwg_vendor_fixture_read" - deliberately distinct from
// integration_dwg_same_version_round_trip's full read -> write -> reopen ->
// compare sequence below). This is a genuine read of the real external
// fixture through bim::dwg's own production API, never a duplicated
// re-implementation of the header-parsing logic.

#include "bim/dwg/probe.hpp"
#include "dwg_test_support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
using bim::dwg::OpenAndValidateDwgFile;
using bim::dwg::test_support::VendorFixturePath;
} // namespace

TEST_CASE("the locked vendor fixture opens and validates as AC1018 through the production contract",
          "[integration][dwg]") {
    const auto fixture_path = VendorFixturePath();

    std::string version;
    const bim::foundation::Status status = OpenAndValidateDwgFile(fixture_path, version);

    INFO("status: " << status.message());
    REQUIRE(status.ok());

    // Packet section 9: the controlled accepted source version is exactly
    // "AC1018" - the same identity the Windows Execution Operator's own
    // Checkpoint 1 independently verified for this fixture.
    CHECK(version == "AC1018");
}
