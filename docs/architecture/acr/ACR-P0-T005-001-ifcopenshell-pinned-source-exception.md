# ACR-P0-T005-001 — Scoped IfcOpenShell Pinned-Source Dependency Exception

**Status:** APPROVED
**Date:** 2026-09-16
**Task:** P0-T005 — IFC Spike
**Architecture Gate:** BIM-AG-P0-T005 v1.0
**Product Authority:** APPROVED
**Architecture Authority:** Product Authority + ChatGPT
**Implementation Engineer:** Claude
**Independent Reviewer:** Kimi

## 1. Trigger

The authoritative P0-T005 dependency-resolution Phase A proved that the
project's frozen vcpkg baseline:

```text
f89a4a1da4e3176a8d1a14c1825b9b2f98e48843
```

does not contain an `ifcopenshell` port.

Authoritative result:

```text
Classification:
FROZEN_BASELINE_METADATA_RESOLUTION_FAILED

vcpkg diagnostic:
the baseline does not contain an entry for port ifcopenshell

Repository modification:
NONE
```

The approved Architecture Gate requires a separate Architecture Authority
decision before changing the dependency-delivery policy.

## 2. Approved exception

P0-T005 may consume IfcOpenShell from an **exact pinned upstream source
revision**, built and installed into an external/local dependency prefix
outside the tracked BIM Platform source tree.

This exception applies to IfcOpenShell only.

The existing project vcpkg baseline remains unchanged for all existing
vcpkg-managed dependencies.

## 3. Approved release line

The dependency proof shall examine the upstream IfcOpenShell **0.8.5** release
line.

The exact upstream Git commit SHA is not frozen by this ACR alone.

Before the P0-T005 Implementation Brief may be released, dependency evidence
must identify and freeze the exact upstream repository/ref/commit used.

Moving branch or unpinned HEAD consumption is prohibited.

## 4. C++-only constraint

P0-T005 uses the IfcOpenShell C++ core only.

Prohibited as the BIM Platform integration mechanism:

- IfcOpenShell Python bindings;
- Python runtime;
- PyPI;
- Conda;
- Docker;
- SWIG-generated Python integration.

## 5. Geometry / OCCT isolation

P0-T005 requires IFC schema parse/write and controlled round-trip behavior,
not IfcGeom geometry conversion.

The dependency proof shall attempt to isolate the minimum C++ parse/write core
without IfcGeom.

P0-T005 shall not silently introduce a second OpenCascade/OCCT runtime.

If the selected upstream revision cannot provide the required IFC4 parse/write
core without an additional/conflicting OCCT dependency, STOP and return to
Architecture Authority.

The BIM Platform's existing OCCT boundary remains unchanged.

## 6. Delivery policy

Allowed dependency-delivery shape:

```text
exact upstream repository
+ exact pinned commit
+ explicit proven build options
-> external dependency build directory
-> external install prefix
-> private consumption by src/interop/ifc/**
```

Not allowed:

- automatic vcpkg baseline advancement;
- `FetchContent` of a moving or unpinned revision;
- copying the upstream source tree into this repository;
- committed prebuilt DLL/lib artifacts;
- hidden system-global installation prerequisites;
- Git submodule without another explicit Architecture Authority decision;
- ad-hoc package registry.

## 7. Ownership boundary

The Architecture Gate remains unchanged:

```text
src/interop/ifc/** = sole first-party IfcOpenShell owner
```

IfcOpenShell headers/types must not leak into any other first-party production
module.

R14 and R15 remain mandatory:

```text
R14 = IFC_OPEN_SHELL_ONLY_IFC_OWNER
R15 = IFC_PUBLIC_NEUTRAL
```

## 8. Public API boundary

The future `bim::ifc` public surface remains project-owned and vendor-neutral.

No IfcOpenShell type, namespace, handle, header, exception, or ownership model
may cross the public first-party boundary.

This ACR does not alter the no-model-expansion rule for P0-T005.

## 9. Build-contract evidence required

Before Implementation Brief release, external dependency evidence must prove:

1. exact upstream repository URL;
2. exact release/ref identity for the selected 0.8.5 C++ source line;
3. exact resolved Git commit SHA;
4. exact upstream CMake entry point;
5. exact relevant CMake options at that revision;
6. exact supported schema-selection mechanism;
7. IFC4 generation/build support;
8. C++ core build path without Python;
9. whether parse/write core can be built without IfcGeom/extra OCCT;
10. exact output include/library layout;
11. exact transitive-link requirements;
12. Debug/Release/RelWithDebInfo implications on Windows x64;
13. exact applicable upstream license files and SHA256 identities.

No package/target/option name may be guessed.

## 10. License control

The dependency proof must capture the exact license files from the pinned
upstream revision and record their SHA256 identities.

The later P0-T005 candidate must include the dependency in applicable project
license inventory/evidence.

This ACR does not approve a final commercial distribution mechanism; it only
authorizes dependency engineering for the Phase-0 spike.

## 11. Reproducibility

A future controlled bootstrap must reproduce the dependency from:

```text
upstream repository
+ exact commit SHA
+ exact build options
+ controlled Windows toolchain
+ external build/install prefix
```

A previously installed global IfcOpenShell must not be required.

## 12. Scope unchanged

This ACR changes only the IfcOpenShell dependency-delivery policy.

It does not change:

- IFC4 as the P0-T005 spike schema;
- the round-trip proof;
- sole-owner/public-neutral boundaries;
- no final model-contract expansion;
- no Python;
- R14/R15;
- P0-T004 or earlier architecture;
- the frozen vcpkg baseline used by existing dependencies.

## 13. Approval state

```text
ACR-P0-T005-001                 = APPROVED
Project vcpkg baseline          = UNCHANGED
IfcOpenShell delivery           = EXACT PINNED SOURCE / EXTERNAL PREFIX
IfcOpenShell release line       = 0.8.5
Exact upstream commit           = PENDING DEPENDENCY PROOF
Implementation Brief            = NOT RELEASED
Implementation                  = NOT AUTHORIZED
```

## 14. Next authorized phase

```text
P0-T005 Dependency Resolution Phase B
Upstream Source Identity + Build-Contract Introspection
```

Phase B is external/read-only with respect to the BIM Platform repository.
