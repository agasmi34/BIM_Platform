# third_party/licenses/

This directory holds the **verbatim** copyright/license notice files for
every direct P0-T001 dependency, copied (not retyped) from the resolved
vcpkg build tree by `scripts/ci/license-inventory.ps1`.

Mechanism:

1. After a successful `cmake --preset ci-win-msvc` configure (or any preset),
   vcpkg installs each resolved port under
   `build/<preset>/vcpkg_installed/<triplet>/share/<port-name>/`, including a
   `copyright` file (and sometimes `usage`).
2. `scripts/ci/license-inventory.ps1` copies each direct dependency's
   `copyright` file into this directory as `<port-name>.LICENSE.txt`, and
   copies `usage` as `<port-name>.USAGE.txt` when present.
3. The script then verifies that all five direct P0-T001 dependencies
   (`opencascade`, `sqlite3`, `catch2`, `fmt`, `spdlog`) have a corresponding
   `<port-name>.LICENSE.txt` file in this directory, and exits non-zero if
   any is missing.

Until a real build has run, this directory intentionally contains **only**
this README - no dependency license file is fabricated or hand-retyped
here. See `LICENSES.md` at the repository root for the summary table and
Implementation Brief Phase M for the requirement.
