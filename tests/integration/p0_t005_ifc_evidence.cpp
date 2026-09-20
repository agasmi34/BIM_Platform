// P0-T005 standalone IFC evidence executable (Implementation Brief
// BIM-TASK-P0-T005-CLAUDE v1.0 section 15; Execution Packet v1.1 section
// 12). Mirrors tests/integration/p0_t004_persistence_evidence.cpp's own
// convention exactly: NOT a Catch2 target - it has its own main() - links
// the SAME production bim::ifc implementation exercised by the
// integration_ifc_* Catch2 tests (never a separate copy), and every field
// is recorded via hand-rolled JSON serialization so no new JSON dependency
// is introduced anywhere in src/**.
//
// Controlled JSON channel discipline (same as p0_t004_persistence_evidence):
// stdout (or the --json file) carries the JSON document ONLY. All
// diagnostics go to stderr.
//
// Determinism (Brief section 15 / Packet section 12): the emitted JSON
// contains no wall-clock timestamp, random value, machine-specific absolute
// path, or nondeterministically-ordered collection. The temp export path
// used internally is never itself written into the JSON.
//
// Usage: p0_t005_ifc_evidence [--json <output-path>]
// Exit code: 0 only if every required boolean field is true; non-zero
// otherwise, including on any std::exception/unknown exception observed at
// this boundary.

#include "bim/ifc/probe.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

[[nodiscard]] std::string JsonString(const std::string& key, const std::string& value) {
    std::string out;
    out.reserve(value.size() + key.size() + 8);
    out += '"';
    out += key;
    out += "\":\"";
    for (const char c : value) {
        // Every value passed to this in this executable is either a fixed
        // first-party literal ("IFC4", "P0-T005") or a first-party status
        // message built only from fixed literals - mirroring
        // p0_t004_persistence_evidence.cpp's own minimal-escape rationale.
        if (c == '"' || c == '\\') {
            out += '\\';
        }
        out += c;
    }
    out += '"';
    return out;
}

[[nodiscard]] std::string JsonBool(const std::string& key, bool value) {
    return "\"" + key + "\":" + (value ? "true" : "false");
}

[[nodiscard]] std::filesystem::path MakeDefaultExportPath() {
    static std::atomic<std::uint64_t> counter{0};
    const auto now_ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::uint64_t unique = counter.fetch_add(1, std::memory_order_relaxed);
    std::ostringstream name;
    name << "bim_p0_t005_ifc_evidence_" << now_ticks << "_" << unique << ".ifc";
    return std::filesystem::temp_directory_path() / name.str();
}

struct FailureEvidence {
    bool missing_file_rejected = false;
    bool malformed_input_rejected = false;
    bool unsupported_schema_rejected = false;
};

FailureEvidence RunFailureScenarios(const std::filesystem::path& fixtures_dir) {
    FailureEvidence out;

    const auto missing_path = fixtures_dir / "this_file_does_not_exist.ifc";
    out.missing_file_rejected = !bim::ifc::OpenAndValidateIfcFile(missing_path).ok();

    const auto malformed_path = fixtures_dir / "malformed_input.ifc";
    out.malformed_input_rejected = !bim::ifc::OpenAndValidateIfcFile(malformed_path).ok();

    const auto unsupported_schema_path = fixtures_dir / "unsupported_schema.ifc";
    out.unsupported_schema_rejected =
        !bim::ifc::OpenAndValidateIfcFile(unsupported_schema_path).ok();

    return out;
}

int RunEvidenceMain(int argc, char** argv) {
    std::string json_path;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--json" && i + 1 < argc) {
            json_path = argv[++i];
        }
    }

#ifndef BIM_P0_T005_IFC_FIXTURES_DIR
#error                                                                                             \
    "BIM_P0_T005_IFC_FIXTURES_DIR must be defined by the build (tests/integration/CMakeLists.txt)"
#endif
    const std::filesystem::path fixtures_dir(BIM_P0_T005_IFC_FIXTURES_DIR);

    const auto seed = bim::ifc::MakeDefaultSeed();
    const std::filesystem::path export_path = MakeDefaultExportPath();

    bim::ifc::RoundTripEvidence evidence;
    const bim::foundation::Status round_trip_status =
        bim::ifc::RunIfcRoundTripProbe(seed, export_path, evidence);

    std::error_code remove_ec;
    std::filesystem::remove(export_path, remove_ec);

    const FailureEvidence failures = RunFailureScenarios(fixtures_dir);

    const bool overall = round_trip_status.ok() && evidence.overall_passed &&
                         failures.missing_file_rejected && failures.malformed_input_rejected &&
                         failures.unsupported_schema_rejected;

    std::ostringstream json;
    json << "{";
    json << JsonString("task", "P0-T005") << ",";
    json << JsonString("schema_expected", evidence.schema_expected) << ",";
    json << JsonString("schema_reopened", evidence.schema_reopened) << ",";
    json << JsonBool("export_success", evidence.export_success) << ",";
    json << JsonBool("reopen_success", evidence.reopen_success) << ",";
    json << JsonBool("project_found", evidence.project_found) << ",";
    json << JsonBool("project_identity_preserved", evidence.project_identity_preserved) << ",";
    json << JsonBool("project_name_preserved", evidence.project_name_preserved) << ",";
    json << JsonBool("element_found", evidence.element_found) << ",";
    json << JsonBool("element_global_id_preserved", evidence.element_global_id_preserved) << ",";
    json << JsonBool("element_name_preserved", evidence.element_name_preserved) << ",";
    json << JsonBool("element_object_type_preserved", evidence.element_object_type_preserved)
         << ",";
    json << JsonBool("scalar_properties_preserved", evidence.scalar_properties_preserved) << ",";
    json << JsonBool("missing_file_rejected", failures.missing_file_rejected) << ",";
    json << JsonBool("malformed_input_rejected", failures.malformed_input_rejected) << ",";
    json << JsonBool("unsupported_schema_rejected", failures.unsupported_schema_rejected) << ",";
    json << JsonBool("overall", overall);
    json << "}";
    const std::string json_text = json.str();

    if (!json_path.empty()) {
        std::error_code mkdir_ec;
        const std::filesystem::path output_path(json_path);
        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path(), mkdir_ec);
            if (mkdir_ec) {
                std::cerr << "p0_t005_ifc_evidence: failed to create output directory '"
                          << output_path.parent_path().string() << "': " << mkdir_ec.message()
                          << "\n";
                return 1;
            }
        }
        std::ofstream out(json_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::cerr << "p0_t005_ifc_evidence: failed to open --json output path: " << json_path
                      << "\n";
            return 1;
        }
        out << json_text;
        out.close();
        if (!out) {
            std::cerr << "p0_t005_ifc_evidence: failed while writing --json output path: "
                      << json_path << "\n";
            return 1;
        }
        std::cerr << "p0_t005_ifc_evidence: wrote evidence to " << json_path << "\n";
    } else {
        std::cout << json_text;
    }

    std::cerr << "p0_t005_ifc_evidence: overall=" << (overall ? "true" : "false");
    if (!round_trip_status.ok()) {
        std::cerr << " round_trip_status=\"" << round_trip_status.message() << "\"";
    }
    std::cerr << "\n";

    return overall ? 0 : 1;
}

} // namespace

int main(int argc, char** argv) noexcept {
    try {
        return RunEvidenceMain(argc, argv);
    } catch (const std::exception&) {
        return 1;
    } catch (...) {
        return 1;
    }
}
