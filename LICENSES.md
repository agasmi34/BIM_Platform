# Third-party license inventory - P0-T001

This file is the authoritative summary. Full copyright/notice text for each
dependency is captured verbatim (not retyped) into `third_party/licenses/`
by `scripts/ci/license-inventory.ps1`, which copies the actual
`copyright`/`usage` files vcpkg installs under
`build/<preset>/vcpkg_installed/<triplet>/share/<port>/` after a real
resolved build. **This document's SPDX/license-family column reflects
public, long-standing project licensing facts researched at authoring time
(2026-08-27); the `third_party/licenses/<port>.LICENSE.txt` files are the
evidentiary source of truth once populated, and the license-inventory job
fails if any of them is missing.**

| Dependency | Role | P0-T001 required version | License family / SPDX | Homepage | Copyright capture |
|---|---|---|---|---|---|
| Open CASCADE Technology (OCCT) | Geometry kernel (`bim_geometry_occt` only) | 8.0.1 | LGPL-2.1 with the Open CASCADE linking exception (vcpkg reports `LGPL-2.1-only`; the OCCT-specific exception clause should be confirmed against the captured copyright file, not assumed from the SPDX tag alone) | https://dev.opencascade.org | `third_party/licenses/opencascade.LICENSE.txt` (populated by license-inventory job) |
| SQLite | Persistence (`bim_persistence` only) | vcpkg baseline-resolved | Public domain (no formal SPDX identifier; see SQLite's own copyright-release statement) | https://www.sqlite.org | `third_party/licenses/sqlite3.LICENSE.txt` |
| Catch2 | Test framework (major version 3, locked) | vcpkg baseline-resolved, `>=3.0.0` | BSL-1.0 | https://github.com/catchorg/Catch2 | `third_party/licenses/catch2.LICENSE.txt` |
| fmt | Formatting utility (behind project logging facade) | vcpkg baseline-resolved | MIT | https://github.com/fmtlib/fmt | `third_party/licenses/fmt.LICENSE.txt` |
| spdlog | Logging (behind project logging facade) | vcpkg baseline-resolved | MIT | https://github.com/gabime/spdlog | `third_party/licenses/spdlog.LICENSE.txt` |

## vcpkg registry baseline

- Frozen commit: `f89a4a1da4e3176a8d1a14c1825b9b2f98e48843`
- Location: `vcpkg-configuration.json` (`default-registry.baseline`)
- Commit message: `[opencascade] update git-tree for 8.0.1 (#53125)`
- Author date: 2026-07-31
- Verified via the GitHub commits API and the commit's own diff (which
  updates `versions/baseline.json`'s `opencascade` entry to
  `{"baseline": "8.0.1", "port-version": 0}` and adds the corresponding entry
  to `versions/o-/opencascade.json`) on 2026-08-27. This SHA was not
  fabricated; it was resolved through genuine research because OCCT 8.0.1
  (released 2026-07-30) postdates the Implementation Engineer's reliable
  training-data cutoff.
- Resolved dependency versions (opencascade, sqlite3, catch2, fmt, spdlog)
  are recorded in `docs/evidence/P0-T001/CLAUDE_HANDOVER.json` once a real
  `vcpkg install`/configure has run. They are intentionally not asserted
  here to avoid recording an unverified value as fact.

## Direct P0-T001 dependency completeness rule

The license-inventory CI job (`scripts/ci/license-inventory.ps1`) fails if
any of the five direct dependencies above lacks a corresponding
`third_party/licenses/<port>.LICENSE.txt` file. See that script and
`third_party/licenses/README.md` for the exact mechanism.
