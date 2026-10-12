// P1-T004 Persistence Schema v2 - journal coordination proof (Architecture Gate
// BIM-AG-P1-T004 section 12; Implementation Brief BIM-IB-P1-T004 sections 6 and
// 13).
//
// The Document keeps a private, ordered queue of pending journal transactions.
// Each successful production mutation prepares exactly one transaction holding
// exactly one record; a failed mutation prepares none; Open and Save prepare
// none. The queue is not observable through the Document API, so this proof
// observes it where it becomes durable: it saves, then reads the durable journal
// back through the private journal store. It also proves the journal is
// bookkeeping and not authority: the snapshot alone defines the reopened
// document, and nothing is ever replayed.
#include "../../src/persistence/src/detail/journal_store.hpp"
#include "../../src/persistence/src/detail/schema_v2.hpp"
#include "../../src/persistence/src/detail/sqlite_connection.hpp"
#include "../../src/persistence/src/detail/sqlite_statement.hpp"

#include "bim/document/document.hpp"
#include "bim/persistence/document_store.hpp"
#include "bim/transactions/journal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

using bim::document::Document;
using bim::document::DocumentLifecycleResult;
using bim::document::DocumentResult;
using bim::document::DocumentResultCode;
using bim::model::ElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;
using bim::persistence::detail::SqliteConnection;
using bim::persistence::detail::SqliteStatement;
using bim::transactions::JournalTransaction;

constexpr std::string_view kAddLevel = "bim.document.add_level.v1";
constexpr std::string_view kUpdateLevelElevation = "bim.document.update_level_elevation.v1";
constexpr std::string_view kAddStraightWall = "bim.document.add_straight_wall.v1";
constexpr std::string_view kUpdateStraightWall = "bim.document.update_straight_wall.v1";
constexpr std::string_view kDeleteElement = "bim.document.delete_element.v1";

class TempDatabasePath {
public:
    TempDatabasePath() : path_(MakeUniquePath()) { RemoveSidecarFiles(path_); }
    ~TempDatabasePath() { RemoveSidecarFiles(path_); }

    TempDatabasePath(const TempDatabasePath&) = delete;
    TempDatabasePath& operator=(const TempDatabasePath&) = delete;
    TempDatabasePath(TempDatabasePath&&) = delete;
    TempDatabasePath& operator=(TempDatabasePath&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }

private:
    static std::filesystem::path MakeUniquePath() {
        static std::atomic<std::uint64_t> counter{0};
        const auto now_ticks = std::chrono::steady_clock::now().time_since_epoch().count();
        const std::uint64_t unique = counter.fetch_add(1, std::memory_order_relaxed);
        std::ostringstream name;
        name << "bim_p1_t004_journal_coordination_" << now_ticks << "_" << unique << ".db";
        return std::filesystem::temp_directory_path() / name.str();
    }

    static void RemoveSidecarFiles(const std::filesystem::path& db_path) {
        std::error_code ec;
        std::filesystem::remove(db_path, ec);
        ec.clear();
        std::filesystem::remove(std::filesystem::path(db_path).concat("-wal"), ec);
        ec.clear();
        std::filesystem::remove(std::filesystem::path(db_path).concat("-shm"), ec);
    }

    std::filesystem::path path_;
};

ElementId Id(std::uint8_t first, std::uint8_t last = 0x01) {
    ElementId id;
    id.bytes[0] = first;
    id.bytes[15] = last;
    return id;
}

StraightWall MakeWall(const ElementId& id, const ElementId& level_id, double length = 5.0) {
    return StraightWall{.id = id,
                        .level_id = level_id,
                        .start = Point2D{0.0, 0.0},
                        .end = Point2D{length, 0.0},
                        .thickness = 0.2,
                        .height = 3.0,
                        .base_offset = 0.0};
}

void RequireOk(const DocumentResult result) {
    INFO(bim::document::ToString(result.code));
    REQUIRE(result.ok());
}

void RequireLifecycleOk(const DocumentLifecycleResult result) {
    INFO(bim::document::ToString(result.code));
    REQUIRE(result.ok());
}

std::vector<JournalTransaction> ReadJournal(const std::filesystem::path& path) {
    SqliteConnection connection;
    REQUIRE(SqliteConnection::OpenFileWithPolicy(path, connection).ok());
    std::vector<JournalTransaction> journal;
    REQUIRE(bim::persistence::detail::ReadJournalTransactions(connection.handle(), journal).ok());
    return journal;
}

void RunRaw(const std::filesystem::path& path, std::string_view sql) {
    SqliteConnection connection;
    REQUIRE(SqliteConnection::OpenFileWithPolicy(path, connection).ok());
    SqliteStatement statement;
    INFO(std::string(sql));
    REQUIRE(SqliteStatement::Prepare(connection.handle(), sql, statement).ok());
    bool has_row = false;
    REQUIRE(statement.Step(has_row).ok());
}

std::vector<std::byte> BytesOf(const ElementId& id) {
    std::vector<std::byte> bytes;
    for (const std::uint8_t byte : id.bytes) {
        bytes.push_back(static_cast<std::byte>(byte));
    }
    return bytes;
}

// The transaction is exactly one record of `kind` over the 16 bytes of `target`.
void RequireOneRecord(const JournalTransaction& transaction, std::string_view kind,
                      const ElementId& target) {
    INFO(transaction.id);
    REQUIRE(transaction.records.size() == 1);
    REQUIRE(transaction.records[0].kind == kind);
    REQUIRE(transaction.records[0].payload.size() == 16);
    REQUIRE(transaction.records[0].payload == BytesOf(target));
}

// Canonical lowercase UUID-v4 text: 36 characters, hyphens at 8/13/18/23,
// version nibble 4, variant nibble one of 8, 9, a, b.
bool IsCanonicalUuidV4(const std::string& text) {
    if (text.size() != 36) {
        return false;
    }
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') {
                return false;
            }
        } else if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    if (text[14] != '4') {
        return false;
    }
    const char variant = text[19];
    return variant == '8' || variant == '9' || variant == 'a' || variant == 'b';
}

} // namespace

TEST_CASE(
    "each production mutation prepares exactly one transaction with the frozen kind and payload",
    "[p1_t004][journal_coordination]") {
    const TempDatabasePath db_path;
    Document document;
    const ElementId level = Id(0x10);
    const ElementId wall = Id(0x31);
    const ElementId doomed = Id(0x32);

    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    RequireOk(document.AddStraightWall(MakeWall(wall, level)));
    RequireOk(document.AddStraightWall(MakeWall(doomed, level, 7.0)));
    RequireOk(document.UpdateLevelElevation(level, 2.0));
    RequireOk(document.UpdateStraightWall(MakeWall(wall, level, 6.0)));
    RequireOk(document.DeleteElement(doomed));
    RequireLifecycleOk(document.Save(db_path.path()));

    const std::vector<JournalTransaction> journal = ReadJournal(db_path.path());
    REQUIRE(journal.size() == 6); // one per successful mutation, oldest first
    RequireOneRecord(journal[0], kAddLevel, level);
    RequireOneRecord(journal[1], kAddStraightWall, wall);
    RequireOneRecord(journal[2], kAddStraightWall, doomed);
    RequireOneRecord(journal[3], kUpdateLevelElevation, level);
    RequireOneRecord(journal[4], kUpdateStraightWall, wall);
    RequireOneRecord(journal[5], kDeleteElement, doomed);
}

TEST_CASE("transaction ids are canonical lowercase UUID-v4 text and are all distinct",
          "[p1_t004][journal_coordination]") {
    const TempDatabasePath db_path;
    Document document;
    const ElementId level = Id(0x10);
    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    for (std::uint8_t i = 0; i < 20; ++i) {
        RequireOk(
            document.AddStraightWall(MakeWall(Id(static_cast<std::uint8_t>(0x40 + i)), level)));
    }
    for (std::uint8_t i = 0; i < 10; ++i) {
        RequireOk(document.UpdateLevelElevation(level, static_cast<double>(i)));
    }
    RequireLifecycleOk(document.Save(db_path.path()));

    const std::vector<JournalTransaction> journal = ReadJournal(db_path.path());
    REQUIRE(journal.size() == 31);
    std::set<std::string> seen;
    for (const JournalTransaction& transaction : journal) {
        INFO(transaction.id);
        REQUIRE(IsCanonicalUuidV4(transaction.id));
        REQUIRE(seen.insert(transaction.id).second);
    }
    // The transaction id is independent of the affected ElementId.
    for (const JournalTransaction& transaction : journal) {
        REQUIRE(transaction.id != bim::model::ToString(level));
    }
}

TEST_CASE("failed mutations prepare no transaction", "[p1_t004][journal_coordination]") {
    const TempDatabasePath db_path;
    Document document;
    const ElementId level = Id(0x10);
    const ElementId wall = Id(0x31);
    constexpr double kHuge = 1.7e308;

    // On an empty document every one of these fails; none may leave a record.
    REQUIRE_FALSE(document.AddStraightWall(MakeWall(wall, level)).ok());    // LevelNotFound
    REQUIRE_FALSE(document.UpdateLevelElevation(level, 1.0).ok());          // ElementNotFound
    REQUIRE_FALSE(document.UpdateStraightWall(MakeWall(wall, level)).ok()); // ElementNotFound
    REQUIRE_FALSE(document.DeleteElement(level).ok());                      // ElementNotFound
    REQUIRE_FALSE(document.AddLevel(Level{.id = ElementId{}, .elevation = 0}).ok()); // invalid id
    RequireLifecycleOk(document.Save(db_path.path()));
    REQUIRE(ReadJournal(db_path.path()).empty());

    // Now a model with exactly two successful mutations, then a barrage of failures.
    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    RequireOk(document.AddStraightWall(MakeWall(wall, level)));

    REQUIRE(document.AddLevel(Level{.id = level, .elevation = 5.0}).code ==
            DocumentResultCode::DuplicateElementId);
    REQUIRE(document.AddStraightWall(MakeWall(wall, level)).code ==
            DocumentResultCode::DuplicateElementId);
    REQUIRE(document.AddStraightWall(MakeWall(Id(0x32), Id(0x99))).code ==
            DocumentResultCode::LevelNotFound);
    REQUIRE(document.AddLevel(Level{.id = Id(0x11), .elevation = std::nan("")}).code ==
            DocumentResultCode::InvalidElement);
    REQUIRE(document.UpdateLevelElevation(wall, 1.0).code ==
            DocumentResultCode::ElementKindMismatch);
    REQUIRE(document.UpdateStraightWall(MakeWall(level, level)).code ==
            DocumentResultCode::ElementKindMismatch);
    RequireOk(document.AddLevel(Level{.id = Id(0x12), .elevation = 9.0}));
    REQUIRE(document.UpdateStraightWall(MakeWall(wall, Id(0x12))).code ==
            DocumentResultCode::RehostNotAllowed);
    REQUIRE(document.DeleteElement(level).code == DocumentResultCode::ElementHasDependents);
    REQUIRE(document.DeleteElement(Id(0x77)).code == DocumentResultCode::ElementNotFound);
    REQUIRE(document
                .AddStraightWall(StraightWall{.id = Id(0x34),
                                              .level_id = level,
                                              .start = Point2D{-kHuge, 0.0},
                                              .end = Point2D{kHuge, 0.0},
                                              .thickness = 0.2,
                                              .height = 3.0,
                                              .base_offset = 0.0})
                .code == DocumentResultCode::DerivedGeometryInvalid);
    REQUIRE(document.UpdateLevelElevation(level, std::nan("")).code ==
            DocumentResultCode::InvalidElement);

    RequireLifecycleOk(document.Save(db_path.path()));
    const std::vector<JournalTransaction> journal = ReadJournal(db_path.path());
    // add level, add wall, and the one successful "add level 0x12" - nothing else.
    REQUIRE(journal.size() == 3);
    RequireOneRecord(journal[0], kAddLevel, level);
    RequireOneRecord(journal[1], kAddStraightWall, wall);
    RequireOneRecord(journal[2], kAddLevel, Id(0x12));
}

TEST_CASE("a successful save clears the queue and history accumulates across saves",
          "[p1_t004][journal_coordination]") {
    const TempDatabasePath db_path;
    Document document;
    const ElementId level = Id(0x10);

    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    RequireLifecycleOk(document.Save(db_path.path()));
    REQUIRE(ReadJournal(db_path.path()).size() == 1);

    // Saving again with nothing pending adds nothing, however often it happens.
    RequireLifecycleOk(document.Save(db_path.path()));
    RequireLifecycleOk(document.Save(db_path.path()));
    const std::vector<JournalTransaction> after_idle_saves = ReadJournal(db_path.path());
    REQUIRE(after_idle_saves.size() == 1);

    RequireOk(document.AddStraightWall(MakeWall(Id(0x31), level)));
    RequireOk(document.UpdateLevelElevation(level, 1.5));
    RequireLifecycleOk(document.Save(db_path.path()));
    const std::vector<JournalTransaction> after_second = ReadJournal(db_path.path());
    REQUIRE(after_second.size() == 3);
    // Old history is preserved verbatim and in place; the new ones follow it.
    REQUIRE(after_second[0].id == after_idle_saves[0].id);
    RequireOneRecord(after_second[0], kAddLevel, level);
    RequireOneRecord(after_second[1], kAddStraightWall, Id(0x31));
    RequireOneRecord(after_second[2], kUpdateLevelElevation, level);

    RequireOk(document.DeleteElement(Id(0x31)));
    RequireLifecycleOk(document.Save(db_path.path()));
    const std::vector<JournalTransaction> after_third = ReadJournal(db_path.path());
    REQUIRE(after_third.size() == 4);
    for (std::size_t i = 0; i < after_second.size(); ++i) {
        REQUIRE(after_third[i].id == after_second[i].id);
    }
    RequireOneRecord(after_third[3], kDeleteElement, Id(0x31));
}

TEST_CASE("open and save create no journal records", "[p1_t004][journal_coordination]") {
    const TempDatabasePath first;
    const TempDatabasePath second;
    Document original;
    const ElementId level = Id(0x10);
    RequireOk(original.AddLevel(Level{.id = level, .elevation = 1.0}));
    RequireOk(original.AddStraightWall(MakeWall(Id(0x31), level)));
    RequireLifecycleOk(original.Save(first.path()));
    const std::vector<JournalTransaction> saved = ReadJournal(first.path());
    REQUIRE(saved.size() == 2);

    // Reopening a populated document builds the whole model but prepares nothing:
    // saving it elsewhere writes an empty journal, and saving it back over the same
    // file leaves the journal exactly as it was.
    Document reopened;
    RequireLifecycleOk(Document::Open(first.path(), reopened));
    RequireLifecycleOk(reopened.Save(second.path()));
    REQUIRE(ReadJournal(second.path()).empty());
    RequireLifecycleOk(reopened.Save(first.path()));
    const std::vector<JournalTransaction> resaved = ReadJournal(first.path());
    REQUIRE(resaved.size() == saved.size());
    for (std::size_t i = 0; i < saved.size(); ++i) {
        REQUIRE(resaved[i].id == saved[i].id);
        REQUIRE(resaved[i].records.size() == saved[i].records.size());
        REQUIRE(resaved[i].records[0].kind == saved[i].records[0].kind);
        REQUIRE(resaved[i].records[0].payload == saved[i].records[0].payload);
    }

    // A failed Open prepares nothing either: the target keeps what it had.
    Document target;
    RequireOk(target.AddLevel(Level{.id = Id(0x50), .elevation = 0.0}));
    const TempDatabasePath missing;
    REQUIRE_FALSE(Document::Open(missing.path(), target).ok());
    RequireLifecycleOk(target.Save(second.path()));
    const std::vector<JournalTransaction> target_journal = ReadJournal(second.path());
    REQUIRE(target_journal.size() == 1);
    RequireOneRecord(target_journal[0], kAddLevel, Id(0x50));
}

TEST_CASE("a mutation after open appends after the old history",
          "[p1_t004][journal_coordination]") {
    const TempDatabasePath db_path;
    const ElementId level = Id(0x10);
    {
        Document original;
        RequireOk(original.AddLevel(Level{.id = level, .elevation = 0.0}));
        RequireOk(original.AddStraightWall(MakeWall(Id(0x31), level)));
        RequireLifecycleOk(original.Save(db_path.path()));
    }
    const std::vector<JournalTransaction> history = ReadJournal(db_path.path());
    REQUIRE(history.size() == 2);

    Document reopened;
    RequireLifecycleOk(Document::Open(db_path.path(), reopened));
    RequireOk(reopened.UpdateStraightWall(MakeWall(Id(0x31), level, 8.0)));
    RequireOk(reopened.AddStraightWall(MakeWall(Id(0x32), level)));
    RequireLifecycleOk(reopened.Save(db_path.path()));

    const std::vector<JournalTransaction> journal = ReadJournal(db_path.path());
    REQUIRE(journal.size() == 4);
    REQUIRE(journal[0].id == history[0].id);
    REQUIRE(journal[1].id == history[1].id);
    RequireOneRecord(journal[2], kUpdateStraightWall, Id(0x31));
    RequireOneRecord(journal[3], kAddStraightWall, Id(0x32));
}

TEST_CASE("a failed save keeps the whole queue pending for the next save",
          "[p1_t004][journal_coordination]") {
    const TempDatabasePath db_path;
    const ElementId level = Id(0x10);
    Document document;
    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    RequireLifecycleOk(document.Save(db_path.path()));

    RequireOk(document.AddStraightWall(MakeWall(Id(0x31), level)));
    RequireOk(document.UpdateLevelElevation(level, 4.0));

    // Make the next save fail by pointing it at a path that cannot be a database.
    std::error_code ec;
    const std::filesystem::path blocked =
        std::filesystem::temp_directory_path() / "bim_p1_t004_no_such_directory" / "doc.db";
    REQUIRE_FALSE(document.Save(blocked).ok());
    REQUIRE_FALSE(std::filesystem::exists(blocked, ec));

    // The durable file was not touched by the failed attempt ...
    REQUIRE(ReadJournal(db_path.path()).size() == 1);
    // ... and both prepared transactions are still pending: the real save writes them.
    RequireLifecycleOk(document.Save(db_path.path()));
    const std::vector<JournalTransaction> journal = ReadJournal(db_path.path());
    REQUIRE(journal.size() == 3);
    RequireOneRecord(journal[1], kAddStraightWall, Id(0x31));
    RequireOneRecord(journal[2], kUpdateLevelElevation, level);
}

TEST_CASE("the journal is not authority and is never replayed", "[p1_t004][journal_coordination]") {
    const TempDatabasePath db_path;
    const ElementId level = Id(0x10);
    Document original;
    RequireOk(original.AddLevel(Level{.id = level, .elevation = 0.0}));
    RequireOk(original.AddStraightWall(MakeWall(Id(0x31), level)));
    RequireOk(original.AddStraightWall(MakeWall(Id(0x32), level)));
    RequireOk(original.UpdateLevelElevation(level, 3.5));
    RequireOk(original.DeleteElement(Id(0x32)));
    RequireLifecycleOk(original.Save(db_path.path()));
    REQUIRE(ReadJournal(db_path.path()).size() == 5);

    const auto expected_levels = original.ListLevels();
    const auto expected_walls = original.ListStraightWalls();

    // A journal that disagrees with the snapshot (here: every record deleted) does
    // not change what Open builds. The snapshot alone defines the document.
    RunRaw(db_path.path(), "DELETE FROM journal_entries;");
    RunRaw(db_path.path(), "DELETE FROM journal_transactions;");
    Document reopened;
    RequireLifecycleOk(Document::Open(db_path.path(), reopened));
    REQUIRE(reopened.ListLevels() == expected_levels);
    REQUIRE(reopened.ListStraightWalls() == expected_walls);
    REQUIRE(ReadJournal(db_path.path()).empty());

    // Opening did not replay or rewrite anything.
    RequireLifecycleOk(reopened.Save(db_path.path()));
    REQUIRE(ReadJournal(db_path.path()).empty());
}
