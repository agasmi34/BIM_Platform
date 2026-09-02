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
