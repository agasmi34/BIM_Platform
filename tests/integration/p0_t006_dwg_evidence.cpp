// P0-T006 standalone DWG evidence executable (Execution Packet
// BIM-AA-P0-T006 v1.0; AA CP2B-H3 clarification's "integration_dwg_evidence").
// Mirrors tests/integration/p0_t005_ifc_evidence.cpp's own convention
// exactly: NOT a Catch2 target - it has its own main() - links the SAME
// production bim::dwg implementation exercised by the integration_dwg_*
// Catch2 tests (never a separate copy), and every field is recorded via
// hand-rolled JSON serialization so no new JSON dependency is introduced
// anywhere in src/**.
//
// Controlled JSON channel discipline (same as p0_t005_ifc_evidence): stdout
// (or the --json file) carries the JSON document ONLY. All diagnostics go
// to stderr.
//
// Determinism (mirrors p0_t005_ifc_evidence exactly): the emitted JSON
// contains no wall-clock timestamp, random value, machine-specific absolute
// path, or nondeterministically-ordered collection. The locked vendor
// fixture's own absolute path (BIM_P0_T006_DWG_FIXTURE, itself a
// machine-specific external path - see dwg_test_support.hpp) and the
// disposable temp export path used internally are never themselves written
// into the JSON.
//
// Usage: p0_t006_dwg_evidence [--json <output-path>]
// Exit code: 0 only if every required boolean field is true; non-zero
// otherwise, including on any std::exception/unknown exception observed at
// this boundary.

#include "bim/dwg/probe.hpp"
#include "dwg_test_support.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>

namespace {

[[nodiscard]] std::string JsonString(const std::string& key, const std::string& value) {
    std::string out;
    out.reserve(value.size() + key.size() + 8);
    out += '"';
    out += key;
    out += "\":\"";
    for (const char c : value) {
        // Every value passed to this in this executable is either a fixed
        // first-party literal ("AC1018", "P0-T006") or a first-party status
        // message built only from fixed literals plus this executable's own
        // known-safe strings - mirroring p0_t005_ifc_evidence.cpp's own
        // minimal-escape rationale.
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

struct FailureEvidence {
    bool missing_file_rejected = false;
    bool same_path_rejected = false;
};

FailureEvidence RunFailureScenarios(const std::filesystem::path& vendor_fixture_path) {
    FailureEvidence out;

    // integration_dwg_missing_file's own scenario: a path guaranteed never
    // to have been created.
    const auto missing_path =
        bim::dwg::test_support::MakeTempDwgPath("evidence_missing_file_never_created");
    std::string missing_version;
    out.missing_file_rejected =
        !bim::dwg::OpenAndValidateDwgFile(missing_path, missing_version).ok();

    // integration_dwg_same_path_rejected's own scenario: source_path ==
    // export_path, against the real locked vendor fixture. Never writes
    // anything - dwg_probe.cpp's own up-front rejection fires before any
    // ODA call.
    bim::dwg::DwgRoundTripEvidence same_path_evidence;
    out.same_path_rejected =
        !bim::dwg::RunDwgRoundTripProbe(vendor_fixture_path, vendor_fixture_path, same_path_evidence)
             .ok();

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

    const std::filesystem::path vendor_fixture_path = bim::dwg::test_support::VendorFixturePath();
    const std::filesystem::path export_path =
        bim::dwg::test_support::MakeTempDwgPath("evidence_round_trip");

    bim::dwg::DwgRoundTripEvidence evidence;
    const bim::foundation::Status round_trip_status =
        bim::dwg::RunDwgRoundTripProbe(vendor_fixture_path, export_path, evidence);

    std::error_code remove_ec;
    std::filesystem::remove(export_path, remove_ec);

    const FailureEvidence failures = RunFailureScenarios(vendor_fixture_path);

    const bool overall = round_trip_status.ok() && evidence.overall_passed &&
                         evidence.source_version == "AC1018" && evidence.reopened_version == "AC1018" &&
                         failures.missing_file_rejected && failures.same_path_rejected;

    std::ostringstream json;
    json << "{";
    json << JsonString("task", "P0-T006") << ",";
    json << JsonString("source_version", evidence.source_version) << ",";
    json << JsonString("reopened_version", evidence.reopened_version) << ",";
    json << JsonBool("read_success", evidence.read_success) << ",";
    json << JsonBool("write_success", evidence.write_success) << ",";
    json << JsonBool("reopen_success", evidence.reopen_success) << ",";
    json << JsonBool("source_mtext_found", evidence.source_mtext_found) << ",";
    json << JsonBool("reopened_mtext_found", evidence.reopened_mtext_found) << ",";
    json << JsonBool("text_content_preserved", evidence.text_content_preserved) << ",";
    json << JsonBool("position_preserved", evidence.position_preserved) << ",";
    json << JsonBool("normal_preserved", evidence.normal_preserved) << ",";
    json << JsonBool("direction_preserved", evidence.direction_preserved) << ",";
    json << JsonBool("text_height_preserved", evidence.text_height_preserved) << ",";
    json << JsonBool("width_preserved", evidence.width_preserved) << ",";
    json << JsonBool("attachment_preserved", evidence.attachment_preserved) << ",";
    json << JsonBool("text_style_preserved", evidence.text_style_preserved) << ",";
    json << JsonBool("round_trip_overall_passed", evidence.overall_passed) << ",";
    json << JsonBool("missing_file_rejected", failures.missing_file_rejected) << ",";
    json << JsonBool("same_path_rejected", failures.same_path_rejected) << ",";
    json << JsonBool("overall", overall);
    json << "}";
    const std::string json_text = json.str();

    if (!json_path.empty()) {
        std::error_code mkdir_ec;
        const std::filesystem::path output_path(json_path);
        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path(), mkdir_ec);
            if (mkdir_ec) {
                std::cerr << "p0_t006_dwg_evidence: failed to create output directory '"
                          << output_path.parent_path().string() << "': " << mkdir_ec.message()
                          << "\n";
                return 1;
            }
        }
        std::ofstream out(json_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::cerr << "p0_t006_dwg_evidence: failed to open --json output path: " << json_path
                      << "\n";
            return 1;
        }
        out << json_text;
        out.close();
        if (!out) {
            std::cerr << "p0_t006_dwg_evidence: failed while writing --json output path: " << json_path
                      << "\n";
            return 1;
        }
        std::cerr << "p0_t006_dwg_evidence: wrote evidence to " << json_path << "\n";
    } else {
        std::cout << json_text;
    }

    std::cerr << "p0_t006_dwg_evidence: overall=" << (overall ? "true" : "false");
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
