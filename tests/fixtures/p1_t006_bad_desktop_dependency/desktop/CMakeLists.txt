# Negative fixture for architecture rule R26 (DESKTOP_VS1_DEPENDENCY_BOUNDARY).
# Simulated desktop/CMakeLists.txt: the Qt-free VS1 workflow target links the
# OCCT kernel adapter directly, which the architecture forbids. Never configured.

add_library(bim_desktop_vs1_workflow STATIC
    src/vs1_workflow.cpp
)

target_link_libraries(bim_desktop_vs1_workflow
    PUBLIC
        bim::commands
        bim::query
        bim::document
        bim::presentation
        bim::geometry_occt
)
