# BIM Platform Architecture Gate Package

## BIM-AG-P0-T003 v1.0

Task:

P0-T003 - Desktop + Viewport Spike

Status:

APPROVED + LOCKED

Date:

2026-09-08

Authoritative baseline:

3f2230be5fcd796c370f485975547112ad52d2e3

Authoritative baseline tree:

34b0d7f5c0bd75df58e8a12578fc37351e137ad8

## Executive decision

P0-T003 establishes the Windows desktop and viewport foundation.

Locked direction:

- Qt 6
- Qt Widgets desktop shell
- bgfx viewport-renderer abstraction candidate
- D3D11 authoritative Windows Phase-0 backend
- private Qt/native-window/bgfx integration bridge
- render-neutral mesh seam
- no Qt/bgfx/native/D3D leakage into BIM/domain public APIs
- no viewport-to-OCCT public coupling
- no performance SLA in the spike

Exact Qt and bgfx versions remain subject to read-only resolution against the
already-frozen vcpkg baseline.

Dependency-baseline changes are not authorized.

## Render-neutral seam

P0-T003 freezes the existence and minimum shape of one neutral mesh seam:

- triangle-list topology;
- float32 local-render-frame positions;
- optional float32 per-vertex normals;
- uint32 indices;
- one render unit equals one model unit;
- global survey coordinates are not passed directly to GPU vertex buffers;
- no BIM/OCCT/Qt/bgfx types in the neutral contract.

Handedness and winding must be resolved explicitly during Phase B before
production implementation.

## Desktop / viewport boundary

Qt Widgets owns the desktop shell.

The private bridge owns native surface acquisition and renderer/platform
integration.

bgfx owns the renderer abstraction below the viewport implementation
boundary.

D3D11 is authoritative for the Windows Phase-0 live-render proof.

## Lifecycle proof

P0-T003 specifically verifies:

- post-platform-window native-handle acquisition;
- surface recreation;
- resize;
- zero-size minimize;
- restore;
- devicePixelRatio changes;
- movement between displays with different DPI;
- physical-pixel renderer reset;
- surface destruction;
- renderer shutdown before QApplication teardown.

## Automated-test proof

Camera, ray and neutral-mesh logic must be testable without requiring a live
interactive GPU desktop.

Phase A verifies any frozen-baseline bgfx Noop/offscreen/non-presented
capability before it is relied upon.

The live rendering acceptance path remains Windows + D3D11.

## Independent consultation

Kimi targeted architecture consultation:

PASS

BLOCKER = 0

MAJOR = 0

MINOR = 4

NOTE = 6

All four MINOR items received minimum-delta Architecture Authority
dispositions and are incorporated into this Gate.

No architecture blocker remains.

## Final Gate status

BIM-AG-P0-T003 v1.0

APPROVED + COMMITTED + LOCKED upon successful architecture-record commit.

Implementation remains explicitly blocked until Architecture Authority
releases the implementation brief and subsequent authorization.