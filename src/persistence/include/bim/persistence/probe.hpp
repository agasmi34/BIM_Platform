#pragma once

#include "bim/foundation/status.hpp"

#include <cstdint>
#include <filesystem>
#include <string>

namespace bim::persistence {

// Runs a minimal in-memory SQLite smoke probe (Implementation Brief Phase I,
// "Persistence"): opens ":memory:", executes one trivial statement, closes
// cleanly, and returns a project-owned status. No BIM/project schema and no
// on-disk database file are created. No SQLite type appears in this header
// or anywhere outside src/persistence/src/persistence_probe.cpp.
[[nodiscard]] bim::foundation::Status RunSqliteMemoryProbe();

// Deterministic, project-owned factual record of one P0-T004 Phase G
// persistence-spike evidence run (Implementation Brief
// BIM-TASK-P0-T004-CLAUDE v1.0 section 22; Architecture Gate section 14).
// Field names and semantics mirror the frozen evidence JSON contract 1:1.
// Every field is a plain project-owned value - no SQLite type appears here
// or anywhere else in this header (Architecture Gate AG-P0T004-001/002,
// mechanically enforced by tools/architecture_checker.cmake rule
// R13/PERSISTENCE_PUBLIC_NEUTRAL). JSON serialization is owned by the
// caller (tests/integration/p0_t004_persistence_evidence.cpp), not by this
// library, so this header and persistence_probe.cpp carry no JSON
// dependency of any kind (no new third-party library is introduced).
//
// This is a deliberate small alternative to the Implementation Brief's
// literally-preferred `RunPersistenceSpike(database_path,
// evidence_json_path)` signature (brief section 5: "Claude may choose a
// small alternative public shape if it is easier to test"): returning a
// typed struct instead of writing JSON directly keeps this library free of
// any JSON serialization/I/O concern, mirrors the existing repository
// evidence convention (P0-T002's p0_t002_geometry_evidence executable owns
// 100% of its own JSON serialization; the production geometry API it calls
// returns only plain project-owned types), and is independently unit/
// integration-testable without touching the filesystem for JSON.
struct PersistenceSpikeEvidence {
    std::int64_t schema_version = 0;
    bool foreign_keys_enabled = false;
    std::string journal_mode;
    bool synchronous_mode_verified = false;
    bool committed_transaction_visible_after_reopen = false;
    bool rolled_back_transaction_absent_after_reopen = false;
    bool duplicate_transaction_rejected = false;
    bool binary_payload_roundtrip_exact = false;
    bool journal_order_preserved = false;
    bool newer_schema_rejected = false;
    bool unrecognized_version_zero_database_rejected = false;
    bool overall_passed = false;
};

// Executes the frozen P0-T004 Phase G persistence-spike evidence scenarios
// (schema bootstrap + file runtime policy, commit/reopen durability,
// mid-transaction rollback, duplicate-transaction-ID rejection, binary
// payload round-trip, deterministic journal ordering, newer-schema
// rejection, unrecognized version-0 database rejection) and reports the
// factual outcome in `out_evidence`.
//
// `database_path` is the base path this call uses for its own private,
// disposable SQLite database file(s); the caller must supply a path outside
// the source tree (Implementation Brief section 22: "never write into the
// source tree"). This function creates every database it touches fresh
// (removing any pre-existing file and its SQLite -wal/-shm sidecars at each
// path it uses before creating), so a stale prior run's on-disk state is
// never silently accepted as current evidence. All transaction IDs and
// payloads used are fixed/deterministic - no wall-clock timestamps, no
// randomness, no environment-dependent values - so two calls against two
// fresh database paths always populate byte-identical `out_evidence`
// values.
//
// The returned Status is non-ok in exactly two cases: an infrastructure
// failure that makes evidence collection itself impossible (e.g. the
// supplied path cannot be opened/created at all), or a normal evidence
// scenario failing outright (`out_evidence.overall_passed == false`) -
// matching the Architecture Gate's fail-closed requirement ("evidence
// inconsistency -> non-zero executable exit"). In both cases `out_evidence`
// is still populated with whatever was determined before the failure, so a
// caller can inspect exactly which field(s) are false.
[[nodiscard]] bim::foundation::Status
RunPersistenceSpike(const std::filesystem::path& database_path,
                    PersistenceSpikeEvidence& out_evidence);

} // namespace bim::persistence
