#pragma once

// Shared, non-executable test-support header for the P0-T005 IFC integration
// tests (mirrors tests/integration/persistence_journal_test_support.hpp's
// existing TempDatabasePath convention - same deterministic-shape,
// not deterministic-valued, unique temp-path pattern).

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>

namespace bim::ifc::test_support {

// A unique, disposable path under the OS temp directory for this process's
// own exported .ifc file. Never inside the source tree (Brief section 15:
// evidence/tests must not depend on or leave behind unstable in-tree
// state).
[[nodiscard]] inline std::filesystem::path MakeTempIfcPath(const std::string& label) {
    static std::atomic<std::uint64_t> counter{0};
    const auto now_ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::uint64_t unique = counter.fetch_add(1, std::memory_order_relaxed);
    std::ostringstream name;
    name << "bim_p0_t005_ifc_" << label << "_" << now_ticks << "_" << unique << ".ifc";
    return std::filesystem::temp_directory_path() / name.str();
}

// Directory containing the P0-T005 IFC negative-input fixtures
// (malformed_input.ifc, unsupported_schema.ifc), injected at compile time
// via target_compile_definitions(... BIM_P0_T005_IFC_FIXTURES_DIR=...) in
// tests/integration/CMakeLists.txt - never guessed or relative-path-derived
// at runtime.
#ifndef BIM_P0_T005_IFC_FIXTURES_DIR
#error                                                                                             \
    "BIM_P0_T005_IFC_FIXTURES_DIR must be defined by the build (tests/integration/CMakeLists.txt)"
#endif

[[nodiscard]] inline std::filesystem::path FixturesDir() {
    return std::filesystem::path(BIM_P0_T005_IFC_FIXTURES_DIR);
}

} // namespace bim::ifc::test_support
