// Proves clean first-party rejection of malformed IFC input (Implementation
// Brief BIM-TASK-P0-T005-CLAUDE v1.0 section 11; Execution Packet v1.1
// section 9's "integration_ifc_malformed_input"). Fixture:
// tests/fixtures/p0_t005_ifc/malformed_input.ifc - a deliberately
// syntactically-broken STEP/IFC file (see that file's own header comment).

#include "bim/ifc/probe.hpp"
#include "ifc_test_support.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {
using bim::ifc::OpenAndValidateIfcFile;
using bim::ifc::test_support::FixturesDir;
} // namespace

TEST_CASE("opening malformed IFC input is rejected with a first-party error",
          "[integration][ifc]") {
    const auto fixture_path = FixturesDir() / "malformed_input.ifc";

    const bim::foundation::Status status = OpenAndValidateIfcFile(fixture_path);

    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(status.message().empty());
}
