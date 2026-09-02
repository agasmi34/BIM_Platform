# bim_transactions - private compilation anchor only in P0-T001 (IC-004).
# Depends on model + foundation, per the locked module dependency law.

add_library(bim_transactions STATIC
    src/transactions_anchor.cpp
)
add_library(bim::transactions ALIAS bim_transactions)

target_link_libraries(bim_transactions PRIVATE bim::model bim::foundation)
target_compile_features(bim_transactions PUBLIC cxx_std_20)

bim_apply_warnings(bim_transactions)
bim_apply_sanitizers(bim_transactions)
