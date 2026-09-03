# P0-T002 - Implementation Brief Amendment 01

**Document ID:** BIM-TASK-P0-T002-CLAUDE-A01

**Version:** 1.0

**Date:** 2026-09-03

**Parent Implementation Brief:** BIM-TASK-P0-T002-CLAUDE v1.0

**Parent Brief SHA256:** `7958C0BE210E4EBFFD137EA102069B6332CE4BD26BA1D825FE8310CFB9DE0D64`

**Parent release commit:** `751c5832eb8ed759d13b035d011999e8e5c7becc`

**Architecture Authority:** Product Owner + ChatGPT

**Status:** APPROVED AND EFFECTIVE

## 1. Purpose

This Amendment resolves limited implementation-contract ambiguities discovered
during the read-only Phase A + Phase B Contract Design Check.

This is NOT an Architecture Clarification Request.

The locked P0-T002 Architecture Gate remains unchanged.

The original Implementation Brief remains byte-stable.

Where this Amendment conflicts with the parent Implementation Brief, this
Amendment controls only the specific provisions stated below.

## 2. AA-C01 - Architecture checker footprint

`tools/architecture_checker.cmake` is explicitly authorized for P0-T002.

Its omission from Implementation Brief section 26 was a documentation drafting
omission.

Also explicitly authorized:

`scripts/ci/architecture.ps1`

This script must be extended so the architecture verification job executes the
two new P0-T002 architecture tests in addition to the existing P0-T001
architecture tests.

No ACR is required for these two edits.

## 3. AA-C02 - RectangleProfile3 semantics

`RectangleProfile3` represents a rectangle, not a general parallelogram.

After finite-value and non-degenerate-vector validation:

- `u_axis` and `v_axis` must be orthogonal within the caller-supplied
  `angular_radians` tolerance;
- axis magnitudes may be normalized internally;
- a finite but non-orthogonal axis pair is `DegenerateGeometry`;
- the adapter must not silently rotate, project or orthogonalize a skewed
  caller-supplied axis pair.

The orthogonality comparison must use normalized direction vectors and an
angularly meaningful comparison.

Raw, unnormalized dot-product magnitude must not be compared directly to an
angular tolerance.

## 4. AA-C03 - Cut interaction qualification

`Cut(host, tool, tolerance)` requires positive volumetric intersection.

Before invoking the actual subtraction Boolean, the adapter may compute an
intersection/common result or equivalent neutral qualification.

Classification semantics are locked:

- no 3D common solid content -> `NoIntersection`;
- boundary-only contact with zero volumetric overlap -> `NoIntersection`;
- positive volumetric overlap -> attempt the actual Cut;
- actual Cut kernel failure after qualification -> `KernelOperationFailed`;
- completed Cut whose result fails platform validation -> `InvalidResult`;
- valid Cut -> `None`.

For P0-T002, when a volume epsilon is required for intersection
qualification, derive it from the caller-supplied linear tolerance as:

`volume_epsilon = linear_tolerance ^ 3`

Do not introduce a hidden platform-wide volume tolerance.

The exact OCCT API used to calculate common/intersection geometry remains an
adapter implementation detail and must be verified against installed OCCT
8.0.1 headers.

## 5. AA-C04 - Fuse interaction qualification

`Fuse(a, b, tolerance)` MUST NOT use positive common volume as its
precondition.

That would incorrectly reject face-contact case J04.

The required semantics are:

1. Determine the minimum geometric separation between the two solids or use
   an equivalent neutral proximity qualification.

2. If separation is greater than the caller-supplied linear tolerance,
   classify as `NoIntersection`.

3. If separation is less than or equal to the supplied linear tolerance,
   attempt the actual Fuse.

4. After Fuse:
   - kernel execution failure -> `KernelOperationFailed`;
   - invalid geometry -> `InvalidResult`;
   - more than one resulting 3D solid when connected union is required ->
     `InvalidResult`;
   - one valid connected resulting solid -> `None`.

Consequences for the locked corpus:

- J01 volumetric overlap reaches Fuse;
- J02 reaches Fuse;
- J03 reaches Fuse;
- J04 exact face contact reaches Fuse and remains expected `None`, volume 2.4;
- J05 gap 0.001 remains `NoIntersection` for every mandatory linear candidate,
  because the largest candidate is 0.0001;
- J06 remains observational.

The exact OCCT distance/proximity API remains provisional until verified
against installed OCCT 8.0.1 headers.

No specific unverified OCCT class name is locked by this Amendment.

## 6. AA-C05 - Result-type disposition

The following P0-T002 public result names are accepted:

`SolidResult`

`MetricsResult`

No generic template-result abstraction is required.

The public opaque-solid design is accepted as:

`struct Solid;`

`using SolidHandle = std::shared_ptr<const Solid>;`

The complete `Solid` definition may exist only in adapter-private
implementation code.

No public caller may retrieve or mutate a `TopoDS_Shape` or any other OCCT
object.

## 7. AA-C06 - Spike history diagnostic callable contract

The P0-only header:

`src/geometry/occt/include/bim/geometry_occt/spike_diagnostics.hpp`

must contain both neutral data types and a callable diagnostic operation.

Required conceptual declarations:

    enum class HistoryOperation {
        Cut,
        Fuse
    };

    enum class HistoryInputSide {
        First,
        Second
    };

    struct FaceHistoryRecord {
        HistoryInputSide input_side;
        int input_face_ordinal;
        int generated_count;
        int modified_count;
        bool deleted;
    };

    struct HistoryRunResult {
        bim::geometry_api::GeometryError error;
        int history_repeat;
        std::vector<FaceHistoryRecord> records;
    };

    [[nodiscard]] HistoryRunResult CaptureHistory(
        HistoryOperation operation,
        const bim::geometry_api::SolidHandle& first,
        const bim::geometry_api::SolidHandle& second,
        const bim::geometry_api::GeometryTolerance& tolerance,
        int history_repeat);

Exact const/reference formatting may follow repository C++ style.

The header remains OCCT-free.

`input_face_ordinal` is a transient observation label only.

It is not persistent topology identity.

## 8. AA-C07 - Evidence CTest invocation

The required CTest:

`integration_geometry_occt_evidence`

must invoke:

`p0_t002_geometry_evidence`

with an explicit:

`--json <build-tree-path>`

argument.

The CTest must not invoke the executable without its required output path.

A path under the integration-test binary directory is acceptable.

The authoritative `geometry-spike.ps1` job continues to write its evidence to:

`build/ci-win-msvc/evidence/P0-T002/`

or the equivalent configured build-root path.

## 9. AA-C08 - API unit-test boundary

`unit_geometry_api_contract`

must link to:

- `bim::geometry_api`;
- Catch2.

It should not require `bim::geometry_occt`.

Its purpose includes proving that the project-owned public geometry contract
is independently consumable without an OCCT adapter dependency.

Behavior requiring actual kernel execution belongs in integration tests.

## 10. AA-C09 - Architecture checker execution model

`tools/architecture_checker.cmake` may gain an optional rule selector:

`BIM_ARCH_CHECK_RULE`

Supported conceptual modes:

- `ALL`;
- `GEOMETRY_API_NO_OCCT_LEAK`;
- `GEOMETRY_OCCT_ONLY_KERNEL_OWNER`.

Default behavior is `ALL`.

Existing P0-T001 architecture tests must retain their current behavior when no
selector is supplied.

The new rule:

`GEOMETRY_API_NO_OCCT_LEAK`

must verify the complete production `src/geometry/api` surface contains no
OCCT implementation token, include or linkage.

The new rule:

`GEOMETRY_OCCT_ONLY_KERNEL_OWNER`

must verify both:

1. no production source/header outside the authorized private OCCT adapter
   implementation owns or includes OCCT implementation types; and

2. no production CMake target outside
   `src/geometry/occt/CMakeLists.txt`
   links or discovers OpenCASCADE.

The public P0-only header:

`src/geometry/occt/include/bim/geometry_occt/spike_diagnostics.hpp`

must remain OCCT-free even though it belongs to the adapter target.

The two mandatory CTests:

`arch_geometry_api_no_occt_leak`

and:

`arch_geometry_occt_only_kernel_owner`

must invoke the appropriate rule mode against the real production source tree.

The existing:

`arch_repository_boundaries`

and:

`arch_checker_detects_violation`

must remain present and retain their accepted P0-T001 behavior.

No new negative-fixture file is required by this Amendment.

Do not add a new bad-architecture fixture merely to satisfy P0-T002.

## 11. AA-C10 - architecture.ps1

`scripts/ci/architecture.ps1`

must execute the exact-name architecture tests:

- `arch_repository_boundaries`;
- `arch_checker_detects_violation`;
- `arch_geometry_api_no_occt_leak`;
- `arch_geometry_occt_only_kernel_owner`.

Missing tests must fail the architecture job.

The accepted existing `--no-tests=error` discipline remains.

## 12. Exact authorized implementation manifest

The following is the exact currently authorized implementation footprint.

### ADD

1. `src/geometry/api/include/bim/geometry_api/geometry.hpp`
2. `src/geometry/occt/include/bim/geometry_occt/spike_diagnostics.hpp`
3. `src/geometry/occt/src/solid_impl.hpp`
4. `src/geometry/occt/src/geometry_occt_adapter.cpp`
5. `src/geometry/occt/src/spike_diagnostics.cpp`
6. `tests/integration/geometry_occt_test_constants.hpp`
7. `tests/unit/unit_geometry_api_contract.cpp`
8. `tests/integration/integration_geometry_occt_primitive.cpp`
9. `tests/integration/integration_geometry_occt_opening_cut.cpp`
10. `tests/integration/integration_geometry_occt_join.cpp`
11. `tests/integration/integration_geometry_occt_failure_corpus.cpp`
12. `tests/integration/integration_geometry_occt_tolerance_matrix.cpp`
13. `tests/integration/integration_geometry_occt_coordinate_matrix.cpp`
14. `tests/integration/integration_geometry_occt_history.cpp`
15. `tests/integration/integration_geometry_occt_repeatability.cpp`
16. `tests/integration/p0_t002_geometry_evidence.cpp`
17. `scripts/ci/geometry-spike.ps1`
18. `Verification-RunbookC-v1.0.ps1`

### MODIFY

19. `src/geometry/occt/CMakeLists.txt`
20. `tests/unit/CMakeLists.txt`
21. `tests/integration/CMakeLists.txt`
22. `tests/architecture/CMakeLists.txt`
23. `tools/architecture_checker.cmake`
24. `scripts/ci/architecture.ps1`

No other implementation path is authorized.

Specifically NOT authorized for modification:

- root `CMakeLists.txt`;
- `tests/CMakeLists.txt`;
- `src/geometry/api/CMakeLists.txt`;
- `vcpkg.json`;
- `vcpkg-configuration.json`;
- model;
- commands;
- query;
- transactions;
- persistence;
- desktop;
- interop.

If implementation proves that another path is materially required, stop and
raise an ACR before editing that path.

## 13. OCCT API verification

The current Phase A/B review established that only the APIs already used in
the accepted P0-T001 probe are presently verified from existing compiled
repository source.

All additional OCCT APIs proposed for:

- profile construction;
- extrusion;
- Cut;
- Fuse;
- validity checking;
- bounding boxes;
- topology traversal;
- minimum-distance/proximity calculation;
- Generated/Modified/Deleted history;

remain provisional until verified against the installed OCCT 8.0.1 headers
or settled by authoritative Windows/MSVC compilation.

Do not represent a remembered OCCT API name as verified evidence.

## 14. Phase A/B disposition

The read-only Phase A + Phase B Contract Design Check is accepted.

No ACR is currently required.

The architecture remains locked.

Phase C-M implementation is NOT authorized by this Amendment alone.

A separate Architecture Authority implementation-authorization record will
follow after this Amendment is committed and the implementation transport /
OCCT-header verification channel is confirmed.