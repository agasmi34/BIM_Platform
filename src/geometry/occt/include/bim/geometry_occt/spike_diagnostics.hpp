#pragma once

// P0-only OCCT operation-history diagnostic boundary (Implementation Brief
// section 19; Amendment 01 AA-C06). May be consumed only by P0-T002
// tests/evidence tooling - never by production BIM code.
//
// This header is OCCT-free even though it physically lives inside the
// bim_geometry_occt adapter target: it must expose no TopoDS_*, BRep*,
// gp_*, Handle(...) or other OCCT type (Brief section 19; Amendment 01
// AA-C09, which requires this specific header stay OCCT-free even though it
// belongs to the adapter target). GEOMETRY_API_NO_OCCT_LEAK does not scan
// this path (it is not under src/geometry/api/), but this header follows
// the same OCCT-free discipline voluntarily because Amendment 01 requires it
// directly.
//
// `input_face_ordinal` below is a TRANSIENT OBSERVATION LABEL ONLY, assigned
// by deterministic pre-operation face-traversal order on a single run. It is
// NOT persistent topology identity, is not stable across separate solid
// constructions, and must never be treated as a stable reference. Persistent
// topology reference design remains out of scope for P0-T002 (P0-T008).

#include "bim/geometry_api/geometry.hpp"

#include <vector>

namespace bim::geometry_occt::spike_diagnostics {

// Keep the default enum representation for these Phase-0 diagnostic enums;
// changing the underlying representation is outside P0-T002.
enum class HistoryOperation { Cut, Fuse }; // NOLINT(performance-enum-size)

enum class HistoryInputSide { First, Second }; // NOLINT(performance-enum-size)

struct FaceHistoryRecord {
    HistoryInputSide input_side;
    int input_face_ordinal;
    int generated_count;
    int modified_count;
    bool deleted;
};

struct HistoryRunResult {
    bim::geometry_api::GeometryError error;
    int history_repeat = 0;
    std::vector<FaceHistoryRecord> records;
};

// Captures one Cut/Fuse history run: builds `operation` from `first` and
// `second` under `tolerance`, then records a FaceHistoryRecord for every
// pre-operation face of both inputs (deterministic traversal order within
// this single run). `history_repeat` is carried through unchanged into the
// result purely as a diagnostic label for the caller's own repeat-loop
// bookkeeping; this function itself performs exactly one build+capture per
// call - callers implement the "repeat at least 10 times" requirement
// (Brief section 19) by calling this function repeatedly with freshly
// constructed `first`/`second` handles.
[[nodiscard]] HistoryRunResult CaptureHistory(HistoryOperation operation,
                                              const bim::geometry_api::SolidHandle& first,
                                              const bim::geometry_api::SolidHandle& second,
                                              const bim::geometry_api::GeometryTolerance& tolerance,
                                              int history_repeat);

} // namespace bim::geometry_occt::spike_diagnostics
