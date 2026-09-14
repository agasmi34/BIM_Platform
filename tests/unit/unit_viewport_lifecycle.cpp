#include <catch2/catch_test_macros.hpp>

#include "bim/viewport/lifecycle.hpp"

using bim::viewport::ViewportLifecycle;
using bim::viewport::ViewportState;

TEST_CASE("A freshly constructed ViewportLifecycle starts Uninitialized", "[viewport][lifecycle]") {
    ViewportLifecycle lifecycle;
    REQUIRE(lifecycle.State() == ViewportState::Uninitialized);
}

TEST_CASE("The happy-path sequence walks Uninitialized -> SurfaceUnavailable -> Ready",
          "[viewport][lifecycle]") {
    ViewportLifecycle lifecycle;
    REQUIRE(lifecycle.BeginInitialization().IsOk());
    REQUIRE(lifecycle.State() == ViewportState::SurfaceUnavailable);
    REQUIRE(lifecycle.OnSurfaceAvailable().IsOk());
    REQUIRE(lifecycle.State() == ViewportState::Ready);
}

TEST_CASE("Ready and Suspended toggle via OnSuspend/OnResume", "[viewport][lifecycle]") {
    ViewportLifecycle lifecycle;
    REQUIRE(lifecycle.BeginInitialization().IsOk());
    REQUIRE(lifecycle.OnSurfaceAvailable().IsOk());

    REQUIRE(lifecycle.OnSuspend().IsOk());
    REQUIRE(lifecycle.State() == ViewportState::Suspended);

    REQUIRE(lifecycle.OnResume().IsOk());
    REQUIRE(lifecycle.State() == ViewportState::Ready);
}

TEST_CASE("OnSurfaceLost drops both Ready and Suspended back to SurfaceUnavailable",
          "[viewport][lifecycle]") {
    SECTION("from Ready") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginInitialization().IsOk());
        REQUIRE(lifecycle.OnSurfaceAvailable().IsOk());
        REQUIRE(lifecycle.OnSurfaceLost().IsOk());
        REQUIRE(lifecycle.State() == ViewportState::SurfaceUnavailable);
    }
    SECTION("from Suspended") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginInitialization().IsOk());
        REQUIRE(lifecycle.OnSurfaceAvailable().IsOk());
        REQUIRE(lifecycle.OnSuspend().IsOk());
        REQUIRE(lifecycle.OnSurfaceLost().IsOk());
        REQUIRE(lifecycle.State() == ViewportState::SurfaceUnavailable);
    }
}

TEST_CASE("BeginShutdown is reachable from every non-terminal state", "[viewport][lifecycle]") {
    SECTION("from Uninitialized") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginShutdown().IsOk());
        REQUIRE(lifecycle.State() == ViewportState::ShuttingDown);
    }
    SECTION("from SurfaceUnavailable") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginInitialization().IsOk());
        REQUIRE(lifecycle.BeginShutdown().IsOk());
        REQUIRE(lifecycle.State() == ViewportState::ShuttingDown);
    }
    SECTION("from Ready") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginInitialization().IsOk());
        REQUIRE(lifecycle.OnSurfaceAvailable().IsOk());
        REQUIRE(lifecycle.BeginShutdown().IsOk());
        REQUIRE(lifecycle.State() == ViewportState::ShuttingDown);
    }
    SECTION("from Suspended") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginInitialization().IsOk());
        REQUIRE(lifecycle.OnSurfaceAvailable().IsOk());
        REQUIRE(lifecycle.OnSuspend().IsOk());
        REQUIRE(lifecycle.BeginShutdown().IsOk());
        REQUIRE(lifecycle.State() == ViewportState::ShuttingDown);
    }
}

TEST_CASE("CompleteShutdown reaches the terminal Destroyed state", "[viewport][lifecycle]") {
    ViewportLifecycle lifecycle;
    REQUIRE(lifecycle.BeginShutdown().IsOk());
    REQUIRE(lifecycle.CompleteShutdown().IsOk());
    REQUIRE(lifecycle.State() == ViewportState::Destroyed);
}

TEST_CASE("BeginShutdown is refused once already ShuttingDown or Destroyed",
          "[viewport][lifecycle]") {
    ViewportLifecycle lifecycle;
    REQUIRE(lifecycle.BeginShutdown().IsOk());
    REQUIRE_FALSE(lifecycle.BeginShutdown().IsOk());

    REQUIRE(lifecycle.CompleteShutdown().IsOk());
    REQUIRE_FALSE(lifecycle.BeginShutdown().IsOk());
}

TEST_CASE("Invalid transitions are refused with InvalidState and do not change State()",
          "[viewport][lifecycle]") {
    SECTION("OnSurfaceAvailable from Uninitialized") {
        ViewportLifecycle lifecycle;
        const auto status = lifecycle.OnSurfaceAvailable();
        REQUIRE_FALSE(status.IsOk());
        REQUIRE(status.Code() == bim::viewport::ViewportErrorCode::InvalidState);
        REQUIRE(lifecycle.State() == ViewportState::Uninitialized);
    }
    SECTION("OnSuspend from SurfaceUnavailable") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginInitialization().IsOk());
        const auto status = lifecycle.OnSuspend();
        REQUIRE_FALSE(status.IsOk());
        REQUIRE(lifecycle.State() == ViewportState::SurfaceUnavailable);
    }
    SECTION("OnResume from Ready") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginInitialization().IsOk());
        REQUIRE(lifecycle.OnSurfaceAvailable().IsOk());
        const auto status = lifecycle.OnResume();
        REQUIRE_FALSE(status.IsOk());
        REQUIRE(lifecycle.State() == ViewportState::Ready);
    }
    SECTION("CompleteShutdown from a non-ShuttingDown state") {
        ViewportLifecycle lifecycle;
        const auto status = lifecycle.CompleteShutdown();
        REQUIRE_FALSE(status.IsOk());
        REQUIRE(lifecycle.State() == ViewportState::Uninitialized);
    }
    SECTION("BeginInitialization a second time") {
        ViewportLifecycle lifecycle;
        REQUIRE(lifecycle.BeginInitialization().IsOk());
        const auto status = lifecycle.BeginInitialization();
        REQUIRE_FALSE(status.IsOk());
        REQUIRE(lifecycle.State() == ViewportState::SurfaceUnavailable);
    }
}
