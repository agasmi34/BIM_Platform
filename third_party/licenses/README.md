# third_party/licenses/

This directory holds the **verbatim** copyright/license notice files for
every direct dependency, copied (not retyped) from the resolved vcpkg build
tree by `scripts/ci/license-inventory.ps1`.

Mechanism:

1. After a successful `cmake --preset ci-win-msvc` configure (or any preset),
   vcpkg installs each resolved port under
   `build/<preset>/vcpkg_installed/<triplet>/share/<port-name>/`, including a
   `copyright` file (and sometimes `usage`).
2. `scripts/ci/license-inventory.ps1` copies each direct dependency's
   `copyright` file into this directory as `<port-name>.LICENSE.txt`, and
   copies `usage` as `<port-name>.USAGE.txt` when present.
3. The script then verifies that all seven direct dependencies
   (`opencascade`, `sqlite3`, `catch2`, `fmt`, `spdlog`, and, as of P0-T003,
   `qtbase` and `bgfx`) have a corresponding `<port-name>.LICENSE.txt` file
   in this directory, and exits non-zero if any is missing.

As of this P0-T003 authoring pass, this directory contains this README plus
two DISCLOSED PLACEHOLDER files - `qtbase.LICENSE.txt` and
`bgfx.LICENSE.txt` - each clearly labeled "PLACEHOLDER - NOT YET POPULATED"
in its own content, not fabricated or hand-retyped license text (see each
file). Both are REMOVED and REPLACED by `scripts/ci/license-inventory.ps1`
on the Operator's first real run against a real vcpkg_installed tree (that
script's "remove previously generated outputs for direct dependencies" step
deletes any file at these exact filenames before regenerating them from the
current run - see that script). No other dependency license file is
fabricated or hand-retyped here. See `LICENSES.md` at the repository root
for the summary table and Implementation Brief Phase M for the requirement.
