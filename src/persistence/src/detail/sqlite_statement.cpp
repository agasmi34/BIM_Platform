#include "sqlite_statement.hpp"

#include "sqlite_status.hpp"

#include <sqlite3.h>

#include <cstddef>

namespace bim::persistence::detail {

SqliteStatement::~SqliteStatement() {
    Finalize();
}

SqliteStatement::SqliteStatement(SqliteStatement&& other) noexcept : stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

SqliteStatement& SqliteStatement::operator=(SqliteStatement&& other) noexcept {
    if (this != &other) {
        Finalize();
        stmt_ = other.stmt_;
        other.stmt_ = nullptr;
    }
    return *this;
}

void SqliteStatement::Finalize() noexcept {
    if (stmt_ != nullptr) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
}

bim::foundation::Status SqliteStatement::Prepare(sqlite3* db, std::string_view sql,
                                                 SqliteStatement& out) {
    sqlite3_stmt* stmt = nullptr;
    const int rc = sqlite3_prepare_v2(db, sql.data(), static_cast<int>(sql.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return TranslateSqliteError("sqlite3_prepare_v2", rc, db);
    }
    out = SqliteStatement(stmt);
    return bim::foundation::Status::Ok();
}

bim::foundation::Status SqliteStatement::BindText(int index, std::string_view value) {
    const int rc = sqlite3_bind_text(stmt_, index, value.data(), static_cast<int>(value.size()),
                                     SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        return TranslateSqliteError("sqlite3_bind_text", rc, sqlite3_db_handle(stmt_));
    }
    return bim::foundation::Status::Ok();
}

bim::foundation::Status SqliteStatement::BindBlob(int index, const std::vector<std::byte>& value) {
    if (value.empty()) {
        const int rc = sqlite3_bind_zeroblob(stmt_, index, 0);
        if (rc != SQLITE_OK) {
            return TranslateSqliteError("sqlite3_bind_zeroblob", rc, sqlite3_db_handle(stmt_));
        }
        return bim::foundation::Status::Ok();
    }

    const int rc = sqlite3_bind_blob(stmt_, index, static_cast<const void*>(value.data()),
                                     static_cast<int>(value.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        return TranslateSqliteError("sqlite3_bind_blob", rc, sqlite3_db_handle(stmt_));
    }
    return bim::foundation::Status::Ok();
}

bim::foundation::Status SqliteStatement::BindInt64(int index, std::int64_t value) {
    const int rc = sqlite3_bind_int64(stmt_, index, static_cast<sqlite3_int64>(value));
    if (rc != SQLITE_OK) {
        return TranslateSqliteError("sqlite3_bind_int64", rc, sqlite3_db_handle(stmt_));
    }
    return bim::foundation::Status::Ok();
}

bim::foundation::Status SqliteStatement::Step(bool& out_has_row) {
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) {
        out_has_row = true;
        return bim::foundation::Status::Ok();
    }
    if (rc == SQLITE_DONE) {
        out_has_row = false;
        return bim::foundation::Status::Ok();
    }
    return TranslateSqliteError("sqlite3_step", rc, sqlite3_db_handle(stmt_));
}

std::string SqliteStatement::ColumnText(int column) const {
    const unsigned char* text = sqlite3_column_text(stmt_, column);
    const int bytes = sqlite3_column_bytes(stmt_, column);
    if (text == nullptr || bytes <= 0) {
        return std::string{};
    }
    return std::string(reinterpret_cast<const char*>(text), static_cast<std::size_t>(bytes));
}

std::vector<std::byte> SqliteStatement::ColumnBlob(int column) const {
    const void* blob = sqlite3_column_blob(stmt_, column);
    const int bytes = sqlite3_column_bytes(stmt_, column);
    if (blob == nullptr || bytes <= 0) {
        return {};
    }
    const auto* data = static_cast<const std::byte*>(blob);
    return std::vector<std::byte>(data, data + bytes);
}

std::int64_t SqliteStatement::ColumnInt64(int column) const {
    return static_cast<std::int64_t>(sqlite3_column_int64(stmt_, column));
}

bim::foundation::Status SqliteStatement::Reset() {
    const int rc = sqlite3_reset(stmt_);
    if (rc != SQLITE_OK) {
        return TranslateSqliteError("sqlite3_reset", rc, sqlite3_db_handle(stmt_));
    }
    return bim::foundation::Status::Ok();
}

} // namespace bim::persistence::detail
