#pragma once

// Shared, non-executable test-support header for the P0-T006 DWG
// integration tests (mirrors tests/integration/ifc_test_support.hpp's
// existing MakeTempIfcPath()/FixturesDir() convention exactly: the same
// deterministic-shape-not-deterministic-value unique temp-path pattern,
// and the same compile-time-injected external-resource-location pattern).

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>

namespace bim::dwg::test_support {

// A unique, disposable path under the OS temp directory for this
// process's own exported .dwg file. Never inside the source tree (same
// rationale as MakeTempIfcPath: evidence/tests must not depend on or leave
// behind unstable in-tree state).
[[nodiscard]] inline std::filesystem::path MakeTempDwgPath(const std::string& label) {
    static std::atomic<std::uint64_t> counter{0};
    const auto now_ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::uint64_t unique = counter.fetch_add(1, std::memory_order_relaxed);
    std::ostringstream name;
    name << "bim_p0_t006_dwg_" << label << "_" << now_ticks << "_" << unique << ".dwg";
    return std::filesystem::temp_directory_path() / name.str();
}

// Absolute path to the locked, vendor-supplied AC1018 controlled fixture
// ("OdWriteEx XRef.dwg", shipped inside the ODA Drawings SDK's own
// Drawing/Examples/OdWriteEx/ tree - Execution Packet BIM-AA-P0-T006 v1.0
// section 8's "Controlled fixture"). This fixture is third-party vendor
// example content under the locked trial SDK's own license - it is never
// copied into this repository (unlike P0-T005's first-party-authored
// malformed_input.ifc/unsupported_schema.ifc fixtures) - so its absolute
// path is supplied by the Windows Execution Operator via the
// BIM_P0_T006_DWG_FIXTURE CMake variable and injected here at compile time
// via target_compile_definitions(...) in tests/integration/CMakeLists.txt,
// exactly mirroring BIM_P0_T005_IFC_FIXTURES_DIR's own injection pattern -
// never guessed or relative-path-derived at runtime.
#ifndef BIM_P0_T006_DWG_FIXTURE
#error "BIM_P0_T006_DWG_FIXTURE must be defined by the build (tests/integration/CMakeLists.txt)"
#endif

[[nodiscard]] inline std::filesystem::path VendorFixturePath() {
    return std::filesystem::path(BIM_P0_T006_DWG_FIXTURE);
}

} // namespace bim::dwg::test_support
