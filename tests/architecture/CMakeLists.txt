# arch_repository_boundaries - runs the mechanical checker
# (tools/architecture_checker.cmake) against the real src/ tree. Must PASS
# (exit 0) for this CTest test to pass. No -DBIM_ARCH_CHECK_RULE is passed,
# so this runs every rule (R1-R7; default BIM_ARCH_CHECK_RULE=ALL), matching
# its pre-P0-T002 behavior of exercising the complete checker against the
# real tree.
add_test(
    NAME arch_repository_boundaries
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_checker_detects_violation - runs the SAME checker against a
# deliberately broken fixture (tests/fixtures/bad_architecture/). The
# checker itself must FAIL (non-zero exit) here; WILL_FAIL TRUE inverts that
# so CTest reports this test as passing only when the checker correctly
# rejects the fixture (Implementation Brief Phase J). Unmodified by P0-T002 -
# no new bad-architecture fixture was added for R6/R7 (Amendment 01 AA-C09).
add_test(
    NAME arch_checker_detects_violation
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/bad_architecture
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_checker_detects_violation PROPERTIES WILL_FAIL TRUE)

# arch_geometry_api_no_occt_leak - P0-T002 Phase C-M (Implementation Brief
# section 24; Amendment 01 AA-C09/AA-C10). Runs ONLY rule R6 (via the new
# BIM_ARCH_CHECK_RULE selector) against the real src/ tree: zero OCCT tokens
# under src/geometry/api/include/**.
add_test(
    NAME arch_geometry_api_no_occt_leak
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=GEOMETRY_API_NO_OCCT_LEAK
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_geometry_occt_only_kernel_owner - P0-T002 Phase C-M (Implementation
# Brief section 24; Amendment 01 AA-C09/AA-C10). Runs ONLY rule R7 against
# the real src/ tree: no OCCT token appears anywhere under src/ outside
# src/geometry/occt/** - bim_geometry_occt remains the sole OCCT owner.
add_test(
    NAME arch_geometry_occt_only_kernel_owner
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=GEOMETRY_OCCT_ONLY_KERNEL_OWNER
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# --- P0-T003 Desktop + Viewport Spike additions (Implementation Brief
# BIM-TASK-P0-T003-CLAUDE v1.0 section 20 "Architecture enforcement" -
# corrected here per AA Source Review Round 1 MINOR fix; the prior comment
# cited section 24, which is actually "Verification Runbook D", not the
# architecture-enforcement section; Implementation Authorization section
# 6/9) - rules R8-R11 (tools/architecture_checker.cmake), each run
# both against the real src/ tree (must PASS) and against its own
# deliberately-broken fixture under tests/fixtures/ (the checker must FAIL
# there; WILL_FAIL TRUE inverts that, matching the arch_checker_detects_violation
# pattern above).

# arch_viewport_public_neutral - R8 against the real src/ tree: zero
# Qt/bgfx/D3D/Windows/OCCT/BIM-semantic tokens under
# src/viewport/include/** and src/viewport/bgfx/include/**.
add_test(
    NAME arch_viewport_public_neutral
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=VIEWPORT_PUBLIC_NEUTRAL
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t003_viewport_fixture_rejected - R8 against
# tests/fixtures/p0_t003_bad_viewport_api/, which deliberately leaks a Qt
# token into a viewport/include/** header. The checker must FAIL here.
add_test(
    NAME arch_p0_t003_viewport_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t003_bad_viewport_api
            -DBIM_ARCH_CHECK_RULE=VIEWPORT_PUBLIC_NEUTRAL
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t003_viewport_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# arch_qt_desktop_only - R9 against the real src/ tree: no Qt token
# anywhere under src/ outside src/desktop/**.
add_test(
    NAME arch_qt_desktop_only
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=QT_DESKTOP_ONLY
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t003_qt_fixture_rejected - R9 against
# tests/fixtures/p0_t003_bad_qt_owner/, which deliberately puts a Qt token
# under model/src/ (outside desktop/**). The checker must FAIL here.
add_test(
    NAME arch_p0_t003_qt_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t003_bad_qt_owner
            -DBIM_ARCH_CHECK_RULE=QT_DESKTOP_ONLY
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t003_qt_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# arch_bgfx_viewport_owner - R10 against the real src/ tree: no bgfx token
# anywhere under src/ outside src/viewport/bgfx/**.
add_test(
    NAME arch_bgfx_viewport_owner
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=BGFX_VIEWPORT_OWNER
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t003_bgfx_fixture_rejected - R10 against
# tests/fixtures/p0_t003_bad_bgfx_owner/, which deliberately puts a bgfx
# token under desktop/src/ (outside viewport/bgfx/**). The checker must
# FAIL here.
add_test(
    NAME arch_p0_t003_bgfx_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t003_bad_bgfx_owner
            -DBIM_ARCH_CHECK_RULE=BGFX_VIEWPORT_OWNER
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t003_bgfx_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# arch_no_direct_d3d - R11 against the real src/ tree: no D3D11/DXGI token
# anywhere under src/, with no exception directory.
add_test(
    NAME arch_no_direct_d3d
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=NO_DIRECT_D3D
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t003_d3d_fixture_rejected - R11 against
# tests/fixtures/p0_t003_bad_direct_d3d/, which deliberately includes
# <d3d11.h>/<dxgi.h> directly. The checker must FAIL here.
add_test(
    NAME arch_p0_t003_d3d_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t003_bad_direct_d3d
            -DBIM_ARCH_CHECK_RULE=NO_DIRECT_D3D
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t003_d3d_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# --- P0-T004 Persistence Spike Phase F additions (Architecture Enforcement) -
# rules R12-R13 (tools/architecture_checker.cmake), each run both against the
# real src/ tree (must PASS) and against its own deliberately-broken fixture
# under tests/fixtures/ (the checker must FAIL there; WILL_FAIL TRUE inverts
# that, matching the P0-T003 pattern above).

# arch_sqlite_persistence_only - R12 against the real src/ tree: no raw
# SQLite API token (sqlite3_*, SQLITE_*, sqlite3*, #include <sqlite3.h>)
# appears anywhere under src/ outside src/persistence/** - bim_persistence
# remains the sole raw-SQLite owner.
add_test(
    NAME arch_sqlite_persistence_only
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=SQLITE_PERSISTENCE_ONLY
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t004_sqlite_fixture_rejected - R12 against
# tests/fixtures/p0_t004_bad_sqlite_outside_persistence/, which deliberately
# puts a raw SQLite token under model/src/ (outside persistence/**). The
# checker must FAIL here.
add_test(
    NAME arch_p0_t004_sqlite_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t004_bad_sqlite_outside_persistence
            -DBIM_ARCH_CHECK_RULE=SQLITE_PERSISTENCE_ONLY
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t004_sqlite_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# arch_persistence_public_neutral - R13 against the real src/ tree: zero raw
# SQLite tokens under src/persistence/include/** - bim::persistence's public
# contract stays vendor-neutral.
add_test(
    NAME arch_persistence_public_neutral
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=PERSISTENCE_PUBLIC_NEUTRAL
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t004_persistence_public_fixture_rejected - R13 against
# tests/fixtures/p0_t004_bad_persistence_public_sqlite/, which deliberately
# leaks a raw SQLite token into a persistence/include/** header. The checker
# must FAIL here.
add_test(
    NAME arch_p0_t004_persistence_public_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t004_bad_persistence_public_sqlite
            -DBIM_ARCH_CHECK_RULE=PERSISTENCE_PUBLIC_NEUTRAL
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t004_persistence_public_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# --- P0-T005 IFC Spike additions (Architecture Enforcement) - rules R14-R15
# (tools/architecture_checker.cmake), each run both against the real src/
# tree (must PASS) and against its own deliberately-broken fixture under
# tests/fixtures/ (the checker must FAIL there; WILL_FAIL TRUE inverts
# that, matching the P0-T003/P0-T004 pattern above). Registered
# unconditionally (not gated by BIM_ENABLE_IFC): both rules are pure text
# scans over tools/architecture_checker.cmake and the real src/ tree /
# fixtures, requiring no compiled IfcOpenShell dependency at all.

# arch_ifc_openshell_only_ifc_owner - R14 against the real src/ tree: no
# IfcOpenShell/IfcParse/IfcUtil/Ifc4 token appears anywhere under src/
# outside src/interop/ifc/** - bim_ifc remains the sole IfcOpenShell owner.
add_test(
    NAME arch_ifc_openshell_only_ifc_owner
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=IFC_OPEN_SHELL_ONLY_IFC_OWNER
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t005_ifc_owner_fixture_rejected - R14 against
# tests/fixtures/p0_t005_bad_ifc_owner/, which deliberately puts raw
# IfcOpenShell tokens under model/src/ (outside interop/ifc/**). The
# checker must FAIL here.
add_test(
    NAME arch_p0_t005_ifc_owner_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t005_bad_ifc_owner
            -DBIM_ARCH_CHECK_RULE=IFC_OPEN_SHELL_ONLY_IFC_OWNER
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t005_ifc_owner_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# arch_ifc_public_neutral - R15 against the real src/ tree: zero
# IfcOpenShell/IfcParse/IfcUtil/Ifc4 tokens under
# src/interop/ifc/include/** - bim::ifc's public contract stays
# vendor-neutral.
add_test(
    NAME arch_ifc_public_neutral
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=IFC_PUBLIC_NEUTRAL
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t005_ifc_public_fixture_rejected - R15 against
# tests/fixtures/p0_t005_bad_ifc_public/, which deliberately leaks a raw
# IfcOpenShell token into a simulated interop/ifc/include/** header. The
# checker must FAIL here.
add_test(
    NAME arch_p0_t005_ifc_public_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t005_bad_ifc_public
            -DBIM_ARCH_CHECK_RULE=IFC_PUBLIC_NEUTRAL
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t005_ifc_public_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# --- P0-T006 DWG / ODA Evaluation additions (Architecture Enforcement) -
# rules R16-R17 (tools/architecture_checker.cmake), each run both against
# the real src/ tree (must PASS) and against its own deliberately-broken
# fixture under tests/fixtures/ (the checker must FAIL there; WILL_FAIL
# TRUE inverts that, matching the P0-T003/P0-T004/P0-T005 pattern above).
# Registered unconditionally (not gated by BIM_ENABLE_DWG): both rules are
# pure text scans over tools/architecture_checker.cmake and the real src/
# tree / fixtures, requiring no compiled ODA Drawings dependency at all -
# mirroring R14/R15's own unconditional registration exactly. Test names
# are exactly as specified by Execution Packet BIM-AA-P0-T006 v1.0's AA
# clarification response (arch_p0_t006_oda_owner /
# arch_p0_t006_oda_owner_fixture_rejected / arch_p0_t006_dwg_public_neutral
# / arch_p0_t006_dwg_public_fixture_rejected), not the arch_<rule-name>
# style R6-R15 used, since that AA response is the authoritative source for
# this task's required test names.

# arch_p0_t006_oda_owner - R16 against the real src/ tree: no ODA Drawings
# token appears anywhere under src/ outside src/interop/dwg/** - bim_dwg
# remains the sole ODA Drawings owner.
add_test(
    NAME arch_p0_t006_oda_owner
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=ODA_DRAWINGS_ONLY_DWG_OWNER
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t006_oda_owner_fixture_rejected - R16 against
# tests/fixtures/p0_t006_bad_dwg_owner/, which deliberately puts a raw ODA
# Drawings token under model/src/ (outside interop/dwg/**). The checker
# must FAIL here.
add_test(
    NAME arch_p0_t006_oda_owner_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t006_bad_dwg_owner
            -DBIM_ARCH_CHECK_RULE=ODA_DRAWINGS_ONLY_DWG_OWNER
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t006_oda_owner_fixture_rejected PROPERTIES WILL_FAIL TRUE)

# arch_p0_t006_dwg_public_neutral - R17 against the real src/ tree: zero
# ODA Drawings tokens under src/interop/dwg/include/** - bim::dwg's public
# contract stays vendor-neutral.
add_test(
    NAME arch_p0_t006_dwg_public_neutral
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/src
            -DBIM_ARCH_CHECK_RULE=DWG_PUBLIC_NEUTRAL
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)

# arch_p0_t006_dwg_public_fixture_rejected - R17 against
# tests/fixtures/p0_t006_bad_dwg_public/, which deliberately leaks a raw
# ODA Drawings token into a simulated interop/dwg/include/** header. The
# checker must FAIL here.
add_test(
    NAME arch_p0_t006_dwg_public_fixture_rejected
    COMMAND ${CMAKE_COMMAND}
            -DBIM_ARCH_CHECK_ROOT=${CMAKE_SOURCE_DIR}/tests/fixtures/p0_t006_bad_dwg_public
            -DBIM_ARCH_CHECK_RULE=DWG_PUBLIC_NEUTRAL
            -P ${CMAKE_SOURCE_DIR}/tools/architecture_checker.cmake
)
set_tests_properties(arch_p0_t006_dwg_public_fixture_rejected PROPERTIES WILL_FAIL TRUE)
