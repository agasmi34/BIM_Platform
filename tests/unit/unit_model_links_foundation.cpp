// This test proves the CMake target graph configures and links correctly
// for bim_model -> bim_foundation, per Architecture Gate section 12.1 item 2
// ("Model can link against foundation without third-party leakage") and
// Implementation Brief IC-004: bim_model has no public API yet in P0-T001,
// so this test cannot exercise model behavior directly. Its value is
// proving the target graph resolves and links cleanly, not runtime
// behavior of a nonexistent model API.

#include "bim/foundation/status.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("bim_model links against bim_foundation and foundation is usable in the same binary",
          "[unit][model][foundation]") {
    // This executable links bim::model (which privately depends on
    // bim::foundation per src/model/CMakeLists.txt) alongside bim::foundation
    // directly. A successful configure/build already proves the target
    // graph resolves without conflict; this assertion exercises the
    // foundation type that model's only dependency provides.
    const bim::foundation::Status status = bim::foundation::Status::Ok();
    REQUIRE(status.ok());
}
