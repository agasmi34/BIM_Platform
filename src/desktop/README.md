# src/desktop/ (boundary placeholder)

Reserved for the Qt desktop application shell. **Not built in P0-T001** -
this directory intentionally contains no `CMakeLists.txt` and is not added
via `add_subdirectory()` from the root `CMakeLists.txt`.

Scope owner: **P0-T003 - Desktop + Viewport Spike** (Qt stable/LTS version
lock; see `docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md` section 23,
AG-008/AG-009). Qt is deliberately not linked anywhere in this repository as
of P0-T001. When built, this module MUST reach the rest of the system only
through application-facing public APIs and must never hold a raw DB or raw
OCCT dependency (Architecture Gate section 8.1).
