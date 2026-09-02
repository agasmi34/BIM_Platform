# bim_persistence - the ONLY target permitted to include SQLite headers or
# link SQLite (Architecture Gate section 8.1/8.2). P0-T001 implements only an
# in-memory smoke probe: no project schema, no on-disk database file
# (Implementation Brief Phase I, "Persistence"). Dual-path SQLite resolution
# is defensive: different vcpkg SQLite port configurations across triplets
# have historically exposed either an "official" SQLite3 CMake package name
# or an "unofficial-sqlite3" one.

find_package(SQLite3 QUIET)

if(TARGET SQLite::SQLite3)
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
        "SQLite3) or find_package(unofficial-sqlite3 CONFIG). Verify the "
        "vcpkg 'sqlite3' port resolved correctly for the active triplet "
        "before treating this as an architectural blocker requiring an ACR.")
endif()

add_library(bim_persistence STATIC
    src/persistence_probe.cpp
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
