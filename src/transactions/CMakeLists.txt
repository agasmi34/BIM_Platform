# bim_transactions - neutral, SQLite-independent transaction-journal
# contract (Architecture Gate BIM-AG-P0-T004 v1.0 section 5 / AG-P0T004-003;
# Implementation Brief BIM-TASK-P0-T004-CLAUDE v1.0 Phase B). Depends on
# model + foundation, per the locked module dependency law. Must not link
# SQLite or depend on bim_persistence.

add_library(bim_transactions STATIC
    src/journal.cpp
)
add_library(bim::transactions ALIAS bim_transactions)

target_include_directories(bim_transactions
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(bim_transactions
    PUBLIC
        bim::foundation
    PRIVATE
        bim::model
)
target_compile_features(bim_transactions PUBLIC cxx_std_20)

bim_apply_warnings(bim_transactions)
bim_apply_sanitizers(bim_transactions)
