// P0-T004 Phase G standalone persistence evidence executable (Implementation
// Brief BIM-TASK-P0-T004-CLAUDE v1.0 section 22; Architecture Gate section
// 14; Phase G Execution Handoff v1.0). Mirrors the governance intent and
// structural convention of the P0-T002 standalone evidence executable
// (tests/integration/p0_t002_geometry_evidence.cpp): NOT a Catch2 target -
// it has its own main() - links the SAME production implementation
// exercised by the Phase D/E integration tests (bim::persistence, compiled
// once, never a separate copy), and every case is recorded via hand-rolled
// JSON serialization so no new JSON dependency is introduced.
//
// Deviation from the P0-T002 executable's own convention, disclosed here:
// P0-T002 prints its JSON followed by an unconditional human-readable
// summary line, both to stdout, when no --json path is given. Phase G
// Execution Handoff v1.0 section 9 is stricter ("emit only the controlled
// deterministic JSON on the intended output channel"; "Do not add human
// prose around JSON on the controlled JSON channel"; "Diagnostics may go to
// stderr only"), so this executable never writes anything but the JSON
// document itself to stdout (or to the --json file) - the human-readable
// summary always goes to stderr, in every mode.
//
// Usage: p0_t004_persistence_evidence [--db <path>] [--json <output-path>]
// --db supplies the base path this run's own private, disposable SQLite
// database file(s) are created under (see bim::persistence::RunPersistence
// Spike); if omitted, a unique path under the OS temp directory is used and
// removed again before exit. --json writes the JSON evidence document to
// <output-path> (its parent directory is created if needed); without it,
// the JSON document is printed to stdout. Neither path, nor any other
// environment-dependent value, ever appears inside the emitted JSON itself
// (Phase G Execution Handoff v1.0 section 4: determinism).
//
// Exit code: 0 when RunPersistenceSpike reports overall_passed == true and
// every step needed to reach that point (parsing, database creation, JSON
// serialization/writing) succeeded; non-zero otherwise, including on any
// std::exception/unknown exception observed at this boundary.

#include "bim/persistence/probe.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

namespace {

// --- minimal hand-rolled JSON serialization (no new JSON dependency,
// mirroring tests/integration/p0_t002_geometry_evidence.cpp) ---------------

[[nodiscard]] std::string JsonString(const std::string& key, const std::string& value) {
    std::string out;
    out.reserve(value.size() + key.size() + 8);
    out += '"';
    out += key;
    out += "\":\"";
    for (const char c : value) {
        // Every value passed to this in this executable is a fixed,
        // first-party-authored literal ("wal"), so a minimal escape set is
        // sufficient and deliberately does not attempt full JSON string
        // escaping generality.
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

[[nodiscard]] std::string JsonInt(const std::string& key, long long value) {
    std::ostringstream oss;
    oss << '"' << key << "\":" << value;
    return oss.str();
}

// Removes a database file and its SQLite -wal/-shm sidecars at `db_path`, if
// present. Best-effort only: this executable's own database files always
// live outside the source tree (OS temp directory, or a caller-supplied
// --db path the caller is responsible for), so a cleanup failure here is a
// hygiene nicety, never a correctness or determinism concern.
void RemoveDatabaseFiles(const std::filesystem::path& db_path) {
    std::error_code ec;
    std::filesystem::remove(db_path, ec);
    ec.clear();
    std::filesystem::remove(std::filesystem::path(db_path).concat("-wal"), ec);
    ec.clear();
    std::filesystem::remove(std::filesystem::path(db_path).concat("-shm"), ec);
}

// One evidence run touches several sibling scenario database files derived
// from a single base path (bim::persistence::RunPersistenceSpike's own
// ScenarioPath convention: "<base>.schema", "<base>.commit", ...) - this
// executable does not need to know those exact suffixes to clean them up;
// it simply also removes the base path itself (never used directly as a
// database by RunPersistenceSpike, but harmless to remove) plus the fixed
// suffix set, keeping this cleanup self-contained rather than depending on
// bim::persistence's private implementation detail.
constexpr std::string_view kScenarioSuffixes[] = {
    "schema", "commit", "rollback",     "duplicate",
    "binary", "order",  "newer-schema", "unrecognized-v0",
};

void RemoveAllEvidenceDatabaseFiles(const std::filesystem::path& base_path) noexcept {
    try {
        RemoveDatabaseFiles(base_path);
        for (const std::string_view suffix : kScenarioSuffixes) {
            std::filesystem::path scenario_path = base_path;
            scenario_path += ".";
            scenario_path += suffix;
            RemoveDatabaseFiles(scenario_path);
        }
    } catch (...) {
        return;
    }
}

// Deterministic-shape (not deterministic-valued - this is a filesystem path,
// which never appears in the emitted JSON) unique base path for this run's
// own private database files, used only when --db is not supplied. Mirrors
// tests/integration/persistence_journal_test_support.hpp's
// TempDatabasePath naming convention, reimplemented locally so this
// executable depends only on bim::persistence's public header, never on any
// tests/** support header or any src/persistence/src/detail/** private
// header.
[[nodiscard]] std::filesystem::path MakeDefaultDatabaseBasePath() {
    static std::atomic<std::uint64_t> counter{0};
    const auto now_ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::uint64_t unique = counter.fetch_add(1, std::memory_order_relaxed);
    std::ostringstream name;
    name << "bim_p0_t004_persistence_evidence_" << now_ticks << "_" << unique << ".db";
    return std::filesystem::temp_directory_path() / name.str();
}

[[nodiscard]] std::string BuildEvidenceJson(const bim::persistence::PersistenceSpikeEvidence& e) {
    std::ostringstream json;
    json << "{";
    json << JsonInt("schema_version", static_cast<long long>(e.schema_version)) << ",";
    json << JsonBool("foreign_keys_enabled", e.foreign_keys_enabled) << ",";
    json << JsonString("journal_mode", e.journal_mode) << ",";
    json << JsonBool("synchronous_mode_verified", e.synchronous_mode_verified) << ",";
    json << JsonBool("committed_transaction_visible_after_reopen",
                     e.committed_transaction_visible_after_reopen)
         << ",";
    json << JsonBool("rolled_back_transaction_absent_after_reopen",
                     e.rolled_back_transaction_absent_after_reopen)
         << ",";
    json << JsonBool("duplicate_transaction_rejected", e.duplicate_transaction_rejected) << ",";
    json << JsonBool("binary_payload_roundtrip_exact", e.binary_payload_roundtrip_exact) << ",";
    json << JsonBool("journal_order_preserved", e.journal_order_preserved) << ",";
    json << JsonBool("newer_schema_rejected", e.newer_schema_rejected) << ",";
    json << JsonBool("unrecognized_version_zero_database_rejected",
                     e.unrecognized_version_zero_database_rejected)
         << ",";
    json << JsonBool("overall_passed", e.overall_passed);
    json << "}";
    return json.str();
}

// Round-8-style extraction (see tests/integration/p0_t002_geometry_evidence.cpp
// for the identical rationale this executable follows): main() itself does
// nothing but forward into this function inside its own try/catch, so no
// exception - std::exception-derived or otherwise - can ever escape main().
int RunEvidenceMain(int argc, char** argv) {
    std::string db_arg;
    std::string json_path;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--db" && i + 1 < argc) {
            db_arg = argv[++i];
        } else if (arg == "--json" && i + 1 < argc) {
            json_path = argv[++i];
        }
    }

    const bool owns_default_db_path = db_arg.empty();
    const std::filesystem::path database_base_path =
        owns_default_db_path ? MakeDefaultDatabaseBasePath() : std::filesystem::path(db_arg);

    bim::persistence::PersistenceSpikeEvidence evidence;
    const bim::foundation::Status status =
        bim::persistence::RunPersistenceSpike(database_base_path, evidence);

    if (owns_default_db_path) {
        RemoveAllEvidenceDatabaseFiles(database_base_path);
    }

    const std::string json_text = BuildEvidenceJson(evidence);

    if (!json_path.empty()) {
        std::error_code mkdir_ec;
        const std::filesystem::path output_path(json_path);
        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path(), mkdir_ec);
            if (mkdir_ec) {
                std::cerr << "p0_t004_persistence_evidence: failed to create output directory '"
                          << output_path.parent_path().string() << "': " << mkdir_ec.message()
                          << "\n";
                return 1;
            }
        }

        std::ofstream out(json_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::cerr << "p0_t004_persistence_evidence: failed to open --json output path: "
                      << json_path << "\n";
            return 1;
        }
        out << json_text;
        out.close();
        if (!out) {
            std::cerr << "p0_t004_persistence_evidence: failed while writing --json output path: "
                      << json_path << "\n";
            return 1;
        }
        std::cerr << "p0_t004_persistence_evidence: wrote evidence to " << json_path << "\n";
    } else {
        // Controlled JSON channel: stdout carries the JSON document ONLY
        // (Phase G Execution Handoff v1.0 section 9). No trailing newline
        // prose, no summary line, nothing else.
        std::cout << json_text;
    }

    std::cerr << "p0_t004_persistence_evidence: overall_passed="
              << (evidence.overall_passed ? "true" : "false");
    if (!status.ok()) {
        std::cerr << " status=\"" << status.message() << "\"";
    }
    std::cerr << "\n";

    return (status.ok() && evidence.overall_passed) ? 0 : 1;
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
