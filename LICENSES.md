# Third-party license inventory - P0-T001 (extended P0-T003)

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

| Dependency | Role | Required version | License family / SPDX | Homepage | Copyright capture |
|---|---|---|---|---|---|
| Open CASCADE Technology (OCCT) | Geometry kernel (`bim_geometry_occt` only) | 8.0.1 | LGPL-2.1 with the Open CASCADE linking exception (vcpkg reports `LGPL-2.1-only`; the OCCT-specific exception clause should be confirmed against the captured copyright file, not assumed from the SPDX tag alone) | https://dev.opencascade.org | `third_party/licenses/opencascade.LICENSE.txt` (populated by license-inventory job) |
| SQLite | Persistence (`bim_persistence` only) | vcpkg baseline-resolved | Public domain (no formal SPDX identifier; see SQLite's own copyright-release statement) | https://www.sqlite.org | `third_party/licenses/sqlite3.LICENSE.txt` |
| Catch2 | Test framework (major version 3, locked) | vcpkg baseline-resolved, `>=3.0.0` | BSL-1.0 | https://github.com/catchorg/Catch2 | `third_party/licenses/catch2.LICENSE.txt` |
| fmt | Formatting utility (behind project logging facade) | vcpkg baseline-resolved | MIT | https://github.com/fmtlib/fmt | `third_party/licenses/fmt.LICENSE.txt` |
| spdlog | Logging (behind project logging facade) | vcpkg baseline-resolved | MIT | https://github.com/gabime/spdlog | `third_party/licenses/spdlog.LICENSE.txt` |
| qtbase (P0-T003) | Desktop application shell, Widgets only (`bim_desktop_spike` only) | 6.11.1#1 (Phase A fact, Architecture Gate `cf7a971903d8103143b2b94e94a4d185d86ce42b`) | LGPL-3.0-only per vcpkg's `qtbase` port (Qt's dual-licensing means the actual applicable terms depend on the captured copyright file, not assumed from the SPDX tag alone - do not treat this cell as legal advice) | https://www.qt.io | `third_party/licenses/qtbase.LICENSE.txt` (currently a disclosed **placeholder** - see that file; populated by license-inventory job on the Operator's first real run) |
| bgfx (P0-T003) | Renderer adapter, D3D11 backend only (`bim_viewport_bgfx` only) | 1.129.8940-496#1, `default-features=false` (Phase A fact) | BSD-2-Clause (public, long-standing project knowledge; not asserted as evidentiary fact here - see the placeholder file's own disclosure) | https://github.com/bkaradzic/bgfx | `third_party/licenses/bgfx.LICENSE.txt` (currently a disclosed **placeholder** - see that file; populated by license-inventory job on the Operator's first real run) |

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

## Direct dependency completeness rule

The license-inventory CI job (`scripts/ci/license-inventory.ps1`) fails if
any of the seven direct dependencies above (five from P0-T001 plus qtbase
and bgfx, added P0-T003) lacks a corresponding
`third_party/licenses/<port>.LICENSE.txt` file. See that script and
`third_party/licenses/README.md` for the exact mechanism.

## P0-T003 addition note

`qtbase` and `bgfx` were added to this table and to
`scripts/ci/license-inventory.ps1`'s `$DirectDependencies` array as part of
the P0-T003 Desktop + Viewport Spike (Implementation Brief
BIM-TASK-P0-T003-CLAUDE v1.0 section 9). Their `third_party/licenses/*.LICENSE.txt`
files are currently disclosed placeholders, not real captured copyright
text (this session had no execution channel to run a real vcpkg
install/configure) - see each placeholder file's own header for the exact
mechanism by which the Operator's first real license-inventory.ps1 run
replaces it with the genuine copyright file.
