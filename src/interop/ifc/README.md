# src/interop/ifc/ (boundary placeholder)

Reserved for the IFC interoperability module. **Not built in P0-T001** - this
directory intentionally contains no `CMakeLists.txt` and is not added via
`add_subdirectory()` from the root `CMakeLists.txt`.

Scope owner: **P0-T005 - IFC Spike** (IfcOpenShell boundary and round-trip
proof; see `docs/gates/P0-T001_Architecture_Gate_Package_v1.0.md` section 23).
IfcOpenShell is not linked anywhere in this repository as of P0-T001
(Implementation Brief section 2.2). When built, this module MUST consume
only public `model`/`query`/`command` contracts (Architecture Gate section
8.1) and must not become a native model owner.
