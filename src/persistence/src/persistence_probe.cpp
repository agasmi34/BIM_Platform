#include "bim/persistence/probe.hpp"

#include "detail/journal_store.hpp"
#include "detail/schema_v1.hpp"
#include "detail/sqlite_connection.hpp"
#include "detail/sqlite_statement.hpp"

#include "bim/transactions/journal.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace bim::persistence {

bim::foundation::Status RunSqliteMemoryProbe() {
    detail::SqliteConnection connection;
    bim::foundation::Status status = detail::SqliteConnection::OpenInMemory(connection);
    if (!status.ok()) {
        return status;
    }

    detail::SqliteStatement statement;
    status = detail::SqliteStatement::Prepare(connection.handle(), "SELECT 1;", statement);
    if (!status.ok()) {
        return status;
    }

    bool has_row = false;
    status = statement.Step(has_row);
    if (!status.ok()) {
        return status;
    }
    if (!has_row) {
        return bim::foundation::Status::Error(
            "sqlite in-memory probe returned no row for SELECT 1");
    }

    const std::int64_t value = statement.ColumnInt64(0);
    if (value != 1) {
        return bim::foundation::Status::Error("sqlite in-memory probe returned unexpected value " +
                                              std::to_string(value));
    }

    return bim::foundation::Status::Ok();
}

// ---------------------------------------------------------------------------
// P0-T004 Phase G: standalone deterministic persistence evidence
// (Implementation Brief BIM-TASK-P0-T004-CLAUDE v1.0 section 22; Architecture
// Gate section 14; bim/persistence/probe.hpp::PersistenceSpikeEvidence /
// RunPersistenceSpike). This section adds evidence-collection logic only -
// it does not modify RunSqliteMemoryProbe above, and it does not touch the
// frozen Phase D/E files (schema_v1.*, journal_store.*, sqlite_statement.cpp)
// it calls into; it is a pure caller of their already-frozen, already-proven
// behavior via the same private src/persistence/src/detail/** surface every
// prior-phase integration test uses (Architecture Gate AG-P0T004-001/002:
// no public header exposes any of this).
// ---------------------------------------------------------------------------

namespace {

// Removes a database file and its SQLite -wal/-shm sidecars at `db_path`, if
// present. Used both before creating a fresh evidence database (so a stale
// prior run's on-disk state is never silently accepted as current evidence -
// Architecture Gate section 14: "No evidence file may be hand-authored or
// treated as PASS without an executed run"; Phase G handoff section 9:
// "avoid stale previous-run output being accepted as current evidence") and
// after each scenario (so this function never leaves DB/WAL/SHM artifacts
// behind beyond what the caller's own `database_path` argument implies).
// Mirrors tests/integration/persistence_journal_test_support.hpp's
// TempDatabasePath::RemoveSidecarFiles exactly, duplicated here (rather than
// included from a tests/** header, which production code must never do)
// because the two live in different translation units with different
// ownership boundaries.
void RemoveDatabaseFiles(const std::filesystem::path& db_path) noexcept {
    std::error_code ec;
    std::filesystem::remove(db_path, ec);
    ec.clear();
    std::filesystem::remove(std::filesystem::path(db_path).concat("-wal"), ec);
    ec.clear();
    std::filesystem::remove(std::filesystem::path(db_path).concat("-shm"), ec);
}

// Derives a fixed, deterministic sibling path for one evidence scenario's
// own private database file, so every scenario below gets full isolation
// from every other scenario without needing more than one caller-supplied
// path. Deterministic string concatenation only - no randomness, no
// wall-clock/PID/GUID component - so this function itself introduces no
// nondeterminism into the evidence run.
[[nodiscard]] std::filesystem::path ScenarioPath(const std::filesystem::path& base,
                                                 std::string_view suffix) {
    std::filesystem::path scenario_path = base;
    scenario_path += ".";
    scenario_path += suffix;
    return scenario_path;
}

// Executes a fixed, first-party-authored statement that returns no row.
// `sql` must never be built from caller-controlled input. Deliberately not
// shared with tests/integration/persistence_journal_test_support.hpp's
// identically-shaped helper - production code must never include a tests/**
// header.
[[nodiscard]] bim::foundation::Status RunFixedStatement(sqlite3* db, std::string_view sql) {
    detail::SqliteStatement statement;
    bim::foundation::Status status = detail::SqliteStatement::Prepare(db, sql, statement);
    if (!status.ok()) {
        return status;
    }
    bool has_row = false;
    return statement.Step(has_row);
}

// --- scenario 1: schema bootstrap + file runtime policy --------------------
// Populates schema_version, foreign_keys_enabled, journal_mode,
// synchronous_mode_verified. OpenOrCreateSchemaV1 routes through
// SqliteConnection::OpenFileWithPolicy internally and fails closed if
// PRAGMA foreign_keys/journal_mode=WAL/synchronous=FULL cannot be
// established and verified (src/persistence/src/detail/sqlite_connection.cpp,
// frozen) - so a successful open here is itself the proof for the latter
// three fields; this scenario does not re-implement that PRAGMA
// verification a second time.
[[nodiscard]] bim::foundation::Status RunSchemaAndPolicyScenario(const std::filesystem::path& path,
                                                                 PersistenceSpikeEvidence& out) {
    RemoveDatabaseFiles(path);
    detail::SqliteConnection connection;
    bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, connection);
    if (!status.ok()) {
        return status;
    }

    std::int64_t version = 0;
    status = detail::ReadUserVersion(connection.handle(), version);
    if (!status.ok()) {
        return status;
    }

    out.schema_version = version;
    out.foreign_keys_enabled = true;
    out.journal_mode = "wal";
    out.synchronous_mode_verified = true;
    return bim::foundation::Status::Ok();
}

// --- scenario 2: committed transaction visible after close/reopen ----------
[[nodiscard]] bim::foundation::Status RunCommitReopenScenario(const std::filesystem::path& path,
                                                              bool& out_visible) {
    RemoveDatabaseFiles(path);
    out_visible = false;

    bim::transactions::JournalTransaction transaction;
    transaction.id = "evidence-committed-transaction";
    transaction.records = {
        bim::transactions::JournalRecord{"evidence-kind-committed",
                                         std::vector<std::byte>{std::byte{0xAA}}},
    };

    {
        detail::SqliteConnection connection;
        bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, connection);
        if (!status.ok()) {
            return status;
        }
        status = detail::AppendJournalTransaction(connection.handle(), transaction);
        if (!status.ok()) {
            return status;
        }
    } // connection closes here.

    detail::SqliteConnection reopened;
    bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, reopened);
    if (!status.ok()) {
        return status;
    }
    std::vector<bim::transactions::JournalTransaction> read_back;
    status = detail::ReadJournalTransactions(reopened.handle(), read_back);
    if (!status.ok()) {
        return status;
    }

    for (const bim::transactions::JournalTransaction& candidate : read_back) {
        if (candidate.id == transaction.id) {
            out_visible = true;
            break;
        }
    }
    return bim::foundation::Status::Ok();
}

// --- scenario 3: mid-transaction rollback leaves nothing after reopen ------
// Uses the same test-only-trigger fault-injection technique Phase E's
// rollback proof established and Architecture Authority accepted
// (tests/integration/integration_persistence_journal_rollback.cpp): a
// trigger scoped to one fixed, never-otherwise-used transaction_id forces
// an ordinary SQLite failure partway through one AppendJournalTransaction
// call, so the ROLLBACK path is exercised via genuine SQLite behavior
// rather than a production test hook. The trigger lives only in this
// scenario's own disposable database file and is never part of schema_v1.cpp
// (unmodified, frozen).
[[nodiscard]] bim::foundation::Status RunRollbackScenario(const std::filesystem::path& path,
                                                          bool& out_absent) {
    RemoveDatabaseFiles(path);
    out_absent = false;

    constexpr std::string_view kRollbackTransactionId = "evidence-rollback-transaction";

    {
        detail::SqliteConnection connection;
        bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, connection);
        if (!status.ok()) {
            return status;
        }

        status = RunFixedStatement(connection.handle(),
                                   "CREATE TRIGGER bim_evidence_force_entry_failure "
                                   "BEFORE INSERT ON journal_entries "
                                   "WHEN NEW.transaction_id = 'evidence-rollback-transaction' "
                                   "BEGIN SELECT RAISE(ABORT, 'induced evidence failure'); END;");
        if (!status.ok()) {
            return status;
        }

        bim::transactions::JournalTransaction transaction;
        transaction.id = std::string(kRollbackTransactionId);
        transaction.records = {
            bim::transactions::JournalRecord{"evidence-kind-rollback-1",
                                             std::vector<std::byte>{std::byte{0x01}}},
            bim::transactions::JournalRecord{"evidence-kind-rollback-2",
                                             std::vector<std::byte>{std::byte{0x02}}},
        };

        const bim::foundation::Status append_status =
            detail::AppendJournalTransaction(connection.handle(), transaction);
        if (append_status.ok()) {
            // The induced failure did not occur - the fault-injection setup
            // itself is broken, not the rollback path being proven. Fail
            // closed rather than silently reporting a false positive.
            return bim::foundation::Status::Error(
                "persistence evidence: rollback scenario's induced failure did not occur");
        }
    } // connection (and its evidence-only trigger) closes here.

    detail::SqliteConnection reopened;
    bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, reopened);
    if (!status.ok()) {
        return status;
    }
    std::vector<bim::transactions::JournalTransaction> read_back;
    status = detail::ReadJournalTransactions(reopened.handle(), read_back);
    if (!status.ok()) {
        return status;
    }

    bool found = false;
    for (const bim::transactions::JournalTransaction& candidate : read_back) {
        if (candidate.id == kRollbackTransactionId) {
            found = true;
            break;
        }
    }
    out_absent = !found;
    return bim::foundation::Status::Ok();
}

// --- scenario 4: duplicate transaction ID is rejected -----------------------
[[nodiscard]] bim::foundation::Status RunDuplicateIdScenario(const std::filesystem::path& path,
                                                             bool& out_rejected) {
    RemoveDatabaseFiles(path);
    out_rejected = false;

    detail::SqliteConnection connection;
    bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, connection);
    if (!status.ok()) {
        return status;
    }

    bim::transactions::JournalTransaction transaction;
    transaction.id = "evidence-duplicate-transaction";
    transaction.records = {
        bim::transactions::JournalRecord{"evidence-kind-duplicate",
                                         std::vector<std::byte>{std::byte{0x07}}},
    };

    status = detail::AppendJournalTransaction(connection.handle(), transaction);
    if (!status.ok()) {
        return status; // the FIRST append must succeed; this is not itself the check.
    }

    const bim::foundation::Status second_status =
        detail::AppendJournalTransaction(connection.handle(), transaction);
    out_rejected = !second_status.ok();
    return bim::foundation::Status::Ok();
}

// --- scenario 5: binary payload round-trips exactly -------------------------
[[nodiscard]] bim::foundation::Status RunBinaryPayloadScenario(const std::filesystem::path& path,
                                                               bool& out_exact) {
    RemoveDatabaseFiles(path);
    out_exact = false;

    const std::vector<std::byte> boundary_payload{
        std::byte{0x00}, std::byte{0x01}, std::byte{0x7F},
        std::byte{0x80}, std::byte{0xFE}, std::byte{0xFF},
    };

    bim::transactions::JournalTransaction transaction;
    transaction.id = "evidence-binary-payload-transaction";
    transaction.records = {
        bim::transactions::JournalRecord{"evidence-kind-boundary", boundary_payload},
        bim::transactions::JournalRecord{"evidence-kind-empty", std::vector<std::byte>{}},
    };

    {
        detail::SqliteConnection connection;
        bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, connection);
        if (!status.ok()) {
            return status;
        }
        status = detail::AppendJournalTransaction(connection.handle(), transaction);
        if (!status.ok()) {
            return status;
        }
    }

    detail::SqliteConnection reopened;
    bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, reopened);
    if (!status.ok()) {
        return status;
    }
    std::vector<bim::transactions::JournalTransaction> read_back;
    status = detail::ReadJournalTransactions(reopened.handle(), read_back);
    if (!status.ok()) {
        return status;
    }

    out_exact = read_back.size() == 1 && read_back[0].id == transaction.id &&
                read_back[0].records.size() == 2 &&
                read_back[0].records[0].kind == "evidence-kind-boundary" &&
                read_back[0].records[0].payload == boundary_payload &&
                read_back[0].records[1].kind == "evidence-kind-empty" &&
                read_back[0].records[1].payload.empty();
    return bim::foundation::Status::Ok();
}

// --- scenario 6: persisted journal ordering is deterministic ---------------
[[nodiscard]] bim::foundation::Status RunJournalOrderScenario(const std::filesystem::path& path,
                                                              bool& out_preserved) {
    RemoveDatabaseFiles(path);
    out_preserved = false;

    bim::transactions::JournalTransaction transaction;
    transaction.id = "evidence-order-transaction";
    transaction.records = {
        bim::transactions::JournalRecord{"evidence-order-0", std::vector<std::byte>{std::byte{0}}},
        bim::transactions::JournalRecord{"evidence-order-1", std::vector<std::byte>{std::byte{1}}},
        bim::transactions::JournalRecord{"evidence-order-2", std::vector<std::byte>{std::byte{2}}},
        bim::transactions::JournalRecord{"evidence-order-3", std::vector<std::byte>{std::byte{3}}},
    };

    {
        detail::SqliteConnection connection;
        bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, connection);
        if (!status.ok()) {
            return status;
        }
        status = detail::AppendJournalTransaction(connection.handle(), transaction);
        if (!status.ok()) {
            return status;
        }
    }

    detail::SqliteConnection reopened;
    bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, reopened);
    if (!status.ok()) {
        return status;
    }
    std::vector<bim::transactions::JournalTransaction> read_back;
    status = detail::ReadJournalTransactions(reopened.handle(), read_back);
    if (!status.ok()) {
        return status;
    }

    bool preserved =
        read_back.size() == 1 && read_back[0].records.size() == transaction.records.size();
    if (preserved) {
        for (std::size_t i = 0; i < transaction.records.size(); ++i) {
            if (read_back[0].records[i].kind != transaction.records[i].kind) {
                preserved = false;
                break;
            }
        }
    }
    out_preserved = preserved;
    return bim::foundation::Status::Ok();
}

// --- scenario 7: newer unsupported schema is rejected -----------------------
[[nodiscard]] bim::foundation::Status
RunNewerSchemaRejectedScenario(const std::filesystem::path& path, bool& out_rejected) {
    RemoveDatabaseFiles(path);
    out_rejected = false;

    {
        detail::SqliteConnection setup;
        bim::foundation::Status status = detail::SqliteConnection::OpenFileWithPolicy(path, setup);
        if (!status.ok()) {
            return status;
        }
        status = RunFixedStatement(setup.handle(), "PRAGMA user_version = 2;");
        if (!status.ok()) {
            return status;
        }
    }

    detail::SqliteConnection connection;
    const bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, connection);
    out_rejected = !status.ok();
    return bim::foundation::Status::Ok();
}

// --- scenario 8: unrecognized version-0 database (pre-existing user table)
// is rejected ----------------------------------------------------------------
[[nodiscard]] bim::foundation::Status
RunUnrecognizedVersionZeroRejectedScenario(const std::filesystem::path& path, bool& out_rejected) {
    RemoveDatabaseFiles(path);
    out_rejected = false;

    {
        detail::SqliteConnection setup;
        bim::foundation::Status status = detail::SqliteConnection::OpenFileWithPolicy(path, setup);
        if (!status.ok()) {
            return status;
        }
        status = RunFixedStatement(
            setup.handle(), "CREATE TABLE evidence_unrelated_table (id INTEGER PRIMARY KEY);");
        if (!status.ok()) {
            return status;
        }
    }

    detail::SqliteConnection connection;
    const bim::foundation::Status status = detail::OpenOrCreateSchemaV1(path, connection);
    out_rejected = !status.ok();
    return bim::foundation::Status::Ok();
}

} // namespace

bim::foundation::Status RunPersistenceSpike(const std::filesystem::path& database_path,
                                            PersistenceSpikeEvidence& out_evidence) {
    out_evidence = PersistenceSpikeEvidence{};

    try {
        bim::foundation::Status status =
            RunSchemaAndPolicyScenario(ScenarioPath(database_path, "schema"), out_evidence);
        if (!status.ok()) {
            return status;
        }

        status = RunCommitReopenScenario(ScenarioPath(database_path, "commit"),
                                         out_evidence.committed_transaction_visible_after_reopen);
        if (!status.ok()) {
            return status;
        }

        status = RunRollbackScenario(ScenarioPath(database_path, "rollback"),
                                     out_evidence.rolled_back_transaction_absent_after_reopen);
        if (!status.ok()) {
            return status;
        }

        status = RunDuplicateIdScenario(ScenarioPath(database_path, "duplicate"),
                                        out_evidence.duplicate_transaction_rejected);
        if (!status.ok()) {
            return status;
        }

        status = RunBinaryPayloadScenario(ScenarioPath(database_path, "binary"),
                                          out_evidence.binary_payload_roundtrip_exact);
        if (!status.ok()) {
            return status;
        }

        status = RunJournalOrderScenario(ScenarioPath(database_path, "order"),
                                         out_evidence.journal_order_preserved);
        if (!status.ok()) {
            return status;
        }

        status = RunNewerSchemaRejectedScenario(ScenarioPath(database_path, "newer-schema"),
                                                out_evidence.newer_schema_rejected);
        if (!status.ok()) {
            return status;
        }

        status = RunUnrecognizedVersionZeroRejectedScenario(
            ScenarioPath(database_path, "unrecognized-v0"),
            out_evidence.unrecognized_version_zero_database_rejected);
        if (!status.ok()) {
            return status;
        }
    } catch (const std::exception& ex) {
        return bim::foundation::Status::Error(
            std::string("persistence evidence: unhandled std::exception: ") + ex.what());
    } catch (...) {
        return bim::foundation::Status::Error("persistence evidence: unhandled unknown exception");
    }

    out_evidence.overall_passed =
        out_evidence.schema_version == detail::kSchemaV1Version &&
        out_evidence.foreign_keys_enabled && out_evidence.journal_mode == "wal" &&
        out_evidence.synchronous_mode_verified &&
        out_evidence.committed_transaction_visible_after_reopen &&
        out_evidence.rolled_back_transaction_absent_after_reopen &&
        out_evidence.duplicate_transaction_rejected &&
        out_evidence.binary_payload_roundtrip_exact && out_evidence.journal_order_preserved &&
        out_evidence.newer_schema_rejected &&
        out_evidence.unrecognized_version_zero_database_rejected;

    if (!out_evidence.overall_passed) {
        return bim::foundation::Status::Error(
            "persistence evidence: one or more mandatory evidence checks failed");
    }
    return bim::foundation::Status::Ok();
}

} // namespace bim::persistence
