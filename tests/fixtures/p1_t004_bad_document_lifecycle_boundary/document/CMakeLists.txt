# DELIBERATELY BROKEN FIXTURE - never configured, never built. It is scanned only
# by tools/architecture_checker.cmake (rule R22, DOCUMENT_LIFECYCLE_BOUNDARY,
# P1-T004), which MUST reject it (CTest
# arch_p1_t004_document_lifecycle_fixture_rejected, WILL_FAIL TRUE).
#
# The document lifecycle is a private implementation detail of bim::document.
# This simulated document/CMakeLists.txt makes persistence and transactions PUBLIC
# usage requirements, so every consumer of bim::document would inherit them.
add_library(bim_document STATIC
    src/document.cpp
)

target_link_libraries(bim_document
    PUBLIC
        bim::model
        bim::persistence
        bim::transactions
)
