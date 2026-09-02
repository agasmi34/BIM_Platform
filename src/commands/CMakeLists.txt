# bim_commands - private compilation anchor only in P0-T001 (IC-004).
# Depends on transactions only; model is not linked directly because
# transactions already provides the path to model, per the target table in
# Implementation Brief Phase G ("transactions (and model only if necessary)")
# - it is not necessary here. bim_commands MUST NOT link SQLite/bim_persistence
# (Architecture Gate section 8.2).

add_library(bim_commands STATIC
    src/commands_anchor.cpp
)
add_library(bim::commands ALIAS bim_commands)

target_link_libraries(bim_commands PRIVATE bim::transactions)
target_compile_features(bim_commands PUBLIC cxx_std_20)

bim_apply_warnings(bim_commands)
bim_apply_sanitizers(bim_commands)
