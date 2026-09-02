# scripts/ci/

Provider-neutral CI-equivalent entrypoints for the four logical jobs
required by P0-T001 (Implementation Brief Phase L / IC-003; Architecture
Gate section 20):

| Script | Logical job |
|---|---|
| `format.ps1` | 1. format |
| `configure-build-test.ps1` | 2. configure-build-test |
| `architecture.ps1` | 3. architecture |
| `license-inventory.ps1` | 4. license-inventory |
| `run-all.ps1` | runs all four in order, stops at first failure |
| `_common.ps1` | shared `Invoke-Native`/`Write-CiSection`/`Assert-CiEnvVar` helpers (dot-sourced only, not a job) |

Each job script returns a non-zero exit code on failure and does not
require a user-specific absolute path; `configure-build-test.ps1`,
`architecture.ps1`, and `license-inventory.ps1` require `VCPKG_ROOT` (and an
active x64 MSVC developer environment for `configure-build-test.ps1`).

**Hosted CI provider (IC-003):** no hosted CI provider has been established
for this repository as of P0-T001. Per Implementation Brief clarification
IC-003 ("If no hosted provider is established, do not invent an external
account/service; record hosted-runner integration as operationally
deferred. This does not waive local CI-equivalent acceptance evidence."),
this directory intentionally contains only the provider-neutral scripts
above and no `.github/workflows/`, Azure Pipelines, or other hosted-runner
configuration. Wiring these entrypoints into a specific hosted provider
(with a pinned runner image, per Architecture Gate section 20) is deferred
to a future task once a provider is chosen by Product/Architecture
Authority.
