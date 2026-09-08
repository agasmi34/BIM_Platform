# P0-T003 - Task Record

## Identity

Task:

P0-T003 - Desktop + Viewport Spike

Architecture Gate:

BIM-AG-P0-T003 v1.0

Date:

2026-09-08

Authoritative parent:

3f2230be5fcd796c370f485975547112ad52d2e3

Authoritative parent tree:

34b0d7f5c0bd75df58e8a12578fc37351e137ad8

Branch:

task/P0-T003-desktop-viewport-spike

Worktree:

D:\Projects\BIM-Platform-WT-P0-T003

## Authority

Product Authority:

APPROVED

Architecture Authority:

APPROVED

Independent targeted architecture consultation:

PASS

BLOCKER:

0

MAJOR:

0

MINOR:

4

NOTE:

6

## Purpose

Prove a Windows-first desktop and viewport foundation suitable for the BIM
Platform without introducing BIM semantics into the desktop or rendering
layers.

The task must prove:

- Qt 6 desktop-shell viability;
- bgfx viewport-abstraction viability;
- D3D11 as the authoritative Windows Phase-0 backend;
- lifecycle, resize and HiDPI behavior;
- camera and input foundations;
- render-neutral mesh handling;
- enforceable isolation between desktop/rendering and BIM/domain layers.

## Current state

ARCHITECTURE GATE APPROVED + LOCKED

Implementation:

NOT AUTHORIZED

The next allowed activity is Phase A read-only dependency and capability
resolution from the already-frozen vcpkg baseline, followed by Phase B
Contract Design Check.

No dependency version change is authorized by this record.