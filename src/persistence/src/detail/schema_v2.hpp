#pragma once

#include "sqlite_connection.hpp"

#include "bim/foundation/status.hpp"

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace bim::persistence::detail {

// P1-T004 Persistence Schema v2 (Architecture Gate BIM-AG-P1-T004 sections 3-5
// and 14; Implementation Brief BIM-IB-P1-T004 section 9). PRAGMA user_version
// remains the ONLY schema-version authority; no metadata field competes with
// it. Schema v2 keeps the two frozen schema-v1 journal tables unchanged and adds
// the authoritative BIM tables document_metadata, elements, levels and
// straight_walls. Everything here is private to src/persistence/src/detail/:
// no public persistence header exposes it and no raw SQLite handle ever leaves
// this directory's callers.
inline constexpr std::int64_t kSchemaV2Version = 2;

// The single document_metadata.format_tag value schema v2 accepts. It names the
// document format; it is NOT a schema-version claim (user_version is).
inline constexpr std::string_view kDocumentFormatTag = "bim.document";

// element_type encoding of the elements table (Architecture Gate section 5).
inline constexpr std::int64_t kElementTypeLevel = 1;
inline constexpr std::int64_t kElementTypeStraightWall = 2;

// Opens a local file-backed database at `path` (frozen file runtime policy via
// SqliteConnection::OpenFileWithPolicy), then applies the frozen schema-v2
// authority rule:
//
//   user_version == 0, no pre-existing user table -> direct, atomic 0 -> 2
//     bootstrap (journal-v1 tables + the complete v2 BIM tables + metadata +
//     user_version = 2 in ONE transaction; version 1 is never observable)
//   user_version == 0, a pre-existing user table  -> reject, zero mutation
//   user_version == 1 -> validate the frozen schema-v1 journal structure, then
//     migrate atomically 1 -> 2 (all committed journal rows preserved)
//   user_version == 2 -> validate the complete schema v2; accept only if valid
//   user_version  > 2 -> reject as an unsupported newer schema, no mutation
//   user_version  < 0 -> reject as invalid, fail closed
//
// On any failure `out` is left unmodified and the connection (if one was
// opened) is closed.
[[nodiscard]] bim::foundation::Status OpenOrCreateSchemaV2(const std::filesystem::path& path,
                                                           SqliteConnection& out);

// Structurally validates that `db` holds exactly the complete schema v2: the
// frozen schema-v1 journal structure; the four BIM tables with exactly their
// frozen DDL (columns, types, NOT NULL, primary keys, CHECK constraints and
// foreign keys with their ON DELETE actions); no other user table, no view and
// no trigger; and exactly one well-formed document_metadata row. Read-only.
[[nodiscard]] bim::foundation::Status ValidateSchemaV2(sqlite3* db);

// Direct atomic 0 -> 2 bootstrap in one BEGIN IMMEDIATE / COMMIT transaction.
// Re-checks inside the transaction that user_version is 0 and that no user
// table exists, builds the journal-v1 tables through CreateSchemaV1Tables, the
// four BIM tables and the metadata row, validates the complete result, and only
// then sets user_version = 2. Any failure rolls back, leaving user_version 0
// and no table observable.
[[nodiscard]] bim::foundation::Status BootstrapSchemaV2(sqlite3* db);

// Atomic 1 -> 2 migration in one BEGIN IMMEDIATE / COMMIT transaction.
// Re-checks inside the transaction that user_version is 1 and that the frozen
// schema-v1 journal structure is valid, creates ONLY the new BIM state
// (document_metadata, elements, levels, straight_walls, the metadata row),
// validates the complete result and only then sets user_version = 2. Any
// failure rolls back, leaving version 1 and every committed journal row
// exactly as they were. Because v1 held no BIM state, the migrated BIM snapshot
// is empty: nothing is inferred from journal payloads.
[[nodiscard]] bim::foundation::Status MigrateSchemaV1ToV2(sqlite3* db);

} // namespace bim::persistence::detail
