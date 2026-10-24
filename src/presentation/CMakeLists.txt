# bim_presentation - P1-T005 Geometry Tessellation & Viewport Bridge (Architecture
# Gate BIM-AG-P1-T005; Implementation Brief P1-T005). The production presentation
# boundary: it converts the neutral LinearExtrusionSpec of a VS1 wall into a
# deterministic, renderer-neutral RenderMeshData by analytic rectangular-prism
# tessellation. No geometry kernel is involved in the rendering path.
#
# Dependency surface (frozen by the Architecture Gate):
#   PUBLIC bim::geometry_api - the neutral LinearExtrusionSpec / GeometryTolerance
#                              input values (public header)
#   PUBLIC bim::viewport     - the RenderMeshData output value (public header)
# Nothing else. In particular this target depends on no document, model, command,
# query, dependency-graph, transaction, persistence, geometry-kernel, renderer, UI,
# SQLite or interoperability target (rule R23, PRESENTATION_BRIDGE_BOUNDARY), and
# the viewport targets do not depend upward on it (rule R24,
# VIEWPORT_UPSTREAM_ISOLATION).

add_library(bim_presentation STATIC
    src/wall_mesh.cpp
)
add_library(bim::presentation ALIAS bim_presentation)

target_include_directories(bim_presentation
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(bim_presentation
    PUBLIC
        bim::geometry_api
        bim::viewport
)

target_compile_features(bim_presentation PUBLIC cxx_std_20)

bim_apply_warnings(bim_presentation)
bim_apply_sanitizers(bim_presentation)
