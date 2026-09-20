# src/interop/ifc/ - bim_ifc / bim::ifc

P0-T005 IFC Spike (Implementation Brief BIM-TASK-P0-T005-CLAUDE v1.0;
Execution Packet v1.1; ACR-P0-T005-001). The sole first-party owner of
IfcOpenShell APIs (enforced mechanically by
`tools/architecture_checker.cmake` R14/IFC_OPEN_SHELL_ONLY_IFC_OWNER and
R15/IFC_PUBLIC_NEUTRAL).

```text
include/bim/ifc/probe.hpp   - public, vendor-neutral P0-T005 probe contract
src/ifc_probe.cpp           - public-facing orchestration (seed / round trip / validate)
src/detail/openshell_adapter.{hpp,cpp} - PRIVATE IfcOpenShell C++ adapter
src/detail/scalar_property_codec.{hpp,cpp} - private, dependency-free property codec
```

Only added to the build when the root `CMakeLists.txt`'s `BIM_ENABLE_IFC`
option is turned on, because - unlike OCCT/SQLite/Qt/bgfx - IfcOpenShell is
not available through the project's vcpkg baseline (ACR-P0-T005-001 section
1). Bootstrap the pinned external dependency first via
`scripts/dependencies/ifcopenshell/bootstrap-ifcopenshell.ps1`, then
configure with `-DBIM_ENABLE_IFC=ON -DIfcOpenShell_DIR=<install-prefix>/lib/cmake/IfcOpenShell`.

Consumes only `IfcOpenShell::IfcParse` (IFC4 schema only, geometry
conversion disabled - `BUILD_IFCGEOM=OFF`, `WITH_OPENCASCADE=OFF`,
`WITH_CGAL=OFF`, `BUILD_IFCPYTHON=OFF`). No OCCT coupling is authorized in
P0-T005 (ACR-P0-T005-001 section 5); this module must never depend on
`bim::geometry_occt` or `bim::geometry_api`.

`vcpkg.json` gained the 16 `boost-*` packages plus `eigen3` at the
project's existing frozen baseline - the exact vcpkg support-dependency
closure the accepted Dependency Resolution Phase C build/consume proof
established as required to compile a first-party C++20 consumer against
the installed `IfcOpenShell::IfcParse` headers (Implementation Brief
section 6; Execution Packet v1.1 section 5). `IfcOpenShell` itself is
deliberately NOT in `vcpkg.json` - it is not in the frozen vcpkg baseline
(ACR-P0-T005-001 section 1) and is consumed only from the external prefix
described above. The vcpkg baseline in `vcpkg-configuration.json` is
unchanged.

This supersedes the P0-T001-era boundary placeholder (this directory
previously contained only this README, per Architecture Gate Package
section 23 and Implementation Brief P0-T001 Phase G).
