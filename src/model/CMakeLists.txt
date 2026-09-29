# bim_model - private compilation anchor only in P0-T001 (IC-004). As of
# P0-T008 (Topological Reference Spike; Implementation Brief
# BIM-TASK-P0-T008-CLAUDE v1.0), bim_model gains its first real public
# contract: PersistentFaceReference / FaceRole / FeatureOwnerId
# (include/bim/model/persistent_face_reference.hpp) and
# ResolveFaceReference() (include/bim/model/face_reference_resolver.hpp,
# src/face_reference_resolver.cpp). Every public header here MUST stay free
# of OCCT/Qt/SQLite/ODA/IfcOpenShell types (Architecture Gate section 8.2 /
# AG-006) - checked mechanically by tools/architecture_checker.cmake
# (arch_repository_boundaries, rule R1). bim_model depends on
# bim::geometry_api (its resolver's public signature uses
# LinearExtrusionSpec/GeometryTolerance from that neutral contract) but NOT
# on bim::geometry_occt and not on the CAD kernel directly - that boundary
# is checked mechanically by the same tool's R7/GEOMETRY_OCCT_ONLY_KERNEL_OWNER
# rule, which scans every first-party file repository-wide outside
# src/geometry/occt/** for a kernel-owned token, bim_model's own new files
# included.

add_library(bim_model STATIC
    src/model_anchor.cpp
    src/face_reference_resolver.cpp
)
add_library(bim::model ALIAS bim_model)

target_link_libraries(bim_model
    PUBLIC
        bim::geometry_api
    PRIVATE
        bim::foundation
)

target_include_directories(bim_model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(bim_model PUBLIC cxx_std_20)

bim_apply_warnings(bim_model)
bim_apply_sanitizers(bim_model)
