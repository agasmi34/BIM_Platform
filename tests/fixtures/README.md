# tests/fixtures/

Controlled test data for tests that need a known, deliberately-broken input
rather than the real repository tree.

- `bad_architecture/` - a minimal mirror of the `src/` module tree
  containing exactly one deliberate architecture-boundary violation (a raw
  OCCT type in a simulated `model` public header), consumed only by the
  `arch_checker_detects_violation` CTest test
  (`tests/architecture/CMakeLists.txt`). Nothing under this directory is
  compiled; it exists purely as scan input for
  `tools/architecture_checker.cmake`.
- `p0_t005_ifc/` - controlled negative *input* fixtures for the IFC Spike
  (P0-T005): `malformed_input.ifc` (deliberately broken STEP/IFC syntax)
  and `unsupported_schema.ifc` (a well-formed STEP header declaring
  `FILE_SCHEMA(('IFC2X3'))`). Consumed by `integration_ifc_malformed_input`,
  `integration_ifc_unsupported_schema`, and `integration_ifc_evidence`
  (`tests/integration/CMakeLists.txt`) - these ARE read by `bim::ifc` at
  test run time (unlike the architecture-boundary fixtures below, they are
  not scan input for `tools/architecture_checker.cmake`).
- `p0_t005_bad_ifc_owner/` - a minimal mirror of the `src/` module tree
  containing exactly one deliberate R14/IFC_OPEN_SHELL_ONLY_IFC_OWNER
  violation (a raw IfcOpenShell token under `model/src/`, outside
  `interop/ifc/**`), consumed only by
  `arch_p0_t005_ifc_owner_fixture_rejected`. Nothing under this directory
  is compiled.
- `p0_t005_bad_ifc_public/` - a minimal mirror of the `src/` module tree
  containing exactly one deliberate R15/IFC_PUBLIC_NEUTRAL violation (a raw
  IfcOpenShell token in a simulated `interop/ifc/include/**` public
  header), consumed only by `arch_p0_t005_ifc_public_fixture_rejected`.
  Nothing under this directory is compiled.
