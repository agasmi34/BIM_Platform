// Proves clean first-party rejection of a missing DWG file (Execution
// Packet BIM-AA-P0-T006 v1.0 section 11; AA CP2B-H3 clarification's
// "integration_dwg_missing_file"), exercised through the full public
// contract (OpenAndValidateDwgFile) rather than a raw filesystem check.

#include "bim/dwg/probe.hpp"
#include "dwg_test_support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
using bim::dwg::OpenAndValidateDwgFile;
using bim::dwg::test_support::MakeTempDwgPath;
} // namespace

TEST_CASE("opening a missing DWG file is rejected with a first-party error", "[integration][dwg]") {
    // A path that is guaranteed never to have been created: MakeTempDwgPath()
    // only builds a unique path, it does not create the file - mirroring
    // integration_ifc_missing_file's own MakeTempIfcPath() usage exactly.
    const auto missing_path = MakeTempDwgPath("missing_file_never_created");

    std::string version;
    const bim::foundation::Status status = OpenAndValidateDwgFile(missing_path, version);

    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(status.message().empty());
    REQUIRE(version.empty());
}
