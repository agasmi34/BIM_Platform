# bim_persistence - the ONLY target permitted to include SQLite headers or
# link SQLite (Architecture Gate AG-P0T004-001/002). SQLite is resolved via
# a preference order (Architecture Gate BIM-AG-P0-T004 v1.0 section 11):
# SQLite3::SQLite3, then the deprecated SQLite::SQLite3, then
# unofficial::sqlite3::sqlite3. Multi-path resolution is defensive:
# different vcpkg SQLite port configurations across triplets have
# historically exposed different CMake package/target names.

find_package(SQLite3 QUIET)

if(TARGET SQLite3::SQLite3)
    set(BIM_SQLITE_TARGET SQLite3::SQLite3)
elseif(TARGET SQLite::SQLite3)
    set(BIM_SQLITE_TARGET SQLite::SQLite3)
else()
    find_package(unofficial-sqlite3 CONFIG QUIET)
    if(TARGET unofficial::sqlite3::sqlite3)
        set(BIM_SQLITE_TARGET unofficial::sqlite3::sqlite3)
    endif()
endif()

if(NOT BIM_SQLITE_TARGET)
    message(FATAL_ERROR
        "bim_persistence: SQLite could not be resolved via find_package("
        "SQLite3) (SQLite3::SQLite3 or the deprecated SQLite::SQLite3) or "
        "find_package(unofficial-sqlite3 CONFIG). Verify the vcpkg "
        "'sqlite3' port resolved correctly for the active triplet before "
        "treating this as an architectural blocker requiring an ACR.")
endif()

add_library(bim_persistence STATIC
    src/persistence_probe.cpp
    src/detail/sqlite_status.cpp
    src/detail/sqlite_connection.cpp
    src/detail/sqlite_statement.cpp
    src/detail/schema_v1.cpp
    src/detail/journal_store.cpp
)
add_library(bim::persistence ALIAS bim_persistence)

target_include_directories(bim_persistence
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(bim_persistence
    PUBLIC
        bim::foundation
    PRIVATE
        bim::model
        bim::transactions
        ${BIM_SQLITE_TARGET}
)

target_compile_features(bim_persistence PUBLIC cxx_std_20)

bim_apply_warnings(bim_persistence)
bim_apply_sanitizers(bim_persistence)
