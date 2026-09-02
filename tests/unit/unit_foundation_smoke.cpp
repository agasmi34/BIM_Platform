#include "bim/foundation/status.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Status::Ok reports success with an empty message", "[unit][foundation]") {
    const bim::foundation::Status status = bim::foundation::Status::Ok();
    REQUIRE(status.ok());
    REQUIRE(static_cast<bool>(status));
    REQUIRE(status.message().empty());
}

TEST_CASE("Status::Error reports failure and preserves the message", "[unit][foundation]") {
    const bim::foundation::Status status = bim::foundation::Status::Error("boom");
    REQUIRE_FALSE(status.ok());
    REQUIRE_FALSE(static_cast<bool>(status));
    REQUIRE(status.message() == "boom");
}
