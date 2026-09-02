#include "bim/foundation/status.hpp"
#include "bim/persistence/probe.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("bim_persistence in-memory SQLite probe opens, executes, and closes cleanly",
          "[integration][persistence][sqlite]") {
    const bim::foundation::Status status = bim::persistence::RunSqliteMemoryProbe();
    REQUIRE(status.ok());
}
