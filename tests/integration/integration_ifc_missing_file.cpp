// Proves clean first-party rejection of a missing IFC file (Implementation
// Brief BIM-TASK-P0-T005-CLAUDE v1.0 section 11; Execution Packet v1.1
// section 9's "integration_ifc_missing_file").

#include "bim/ifc/probe.hpp"
#include "ifc_test_support.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {
using bim::ifc::OpenAndValidateIfcFile;
using bim::ifc::test_support::MakeTempIfcPath;
} // namespace

TEST_CASE("opening a missing IFC file is rejected with a first-party error", "[integration][ifc]") {
    // A path that is guaranteed never to have been created: MakeTempIfcPath()
    // only builds a unique path, it does not create the file.
    const auto missing_path = MakeTempIfcPath("missing_file_never_created");

    const bim::foundation::Status status = OpenAndValidateIfcFile(missing_path);

    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(status.message().empty());
}
