#pragma once

// Public, vendor-neutral P0-T005 IFC probe contract (Implementation Brief
// BIM-TASK-P0-T005-CLAUDE v1.0 section 8; Execution Packet v1.1 section 6;
// ACR-P0-T005-001 section 8). This is a small, first-party, Phase-0
// diagnostic/round-trip contract local to bim::ifc - it is NOT the future
// IFC subsystem/model contract (Brief section 2) and must never grow into
// one without a separate Architecture Authority decision.
//
// No IfcOpenShell type, generated IFC4 schema class, Boost type, filesystem
// implementation detail, or third-party exception may ever appear in this
// header or be reachable through it (Brief section 8; R15/IFC_PUBLIC_NEUTRAL
// below). Every third-party error is translated at the private adapter
// boundary (src/interop/ifc/src/detail/openshell_adapter.cpp) into the
// project-owned bim::foundation::Status type used throughout this header.

#include "bim/foundation/status.hpp"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace bim::ifc {

// Deterministic, geometry-free IFC4 seed for the P0-T005 controlled
// round-trip scenario (Brief section 9, items 1-6). Every field is a fixed,
// first-party-chosen literal - never derived from wall-clock time, machine
// state, or random values - so every run against the same seed value is
// reproducible (Brief section 15 / Packet section 12: evidence determinism).
//
// `project_global_id` / `element_global_id` are fixed 22-character
// compressed IFC GlobalId literals (the standard base64-family IFC GUID
// compression alphabet), chosen as first-party literals rather than
// programmatically derived from a UUID at construction time - Brief section
// 9 item 3 permits "a fixed canonical UUID OR ANOTHER DETERMINISTIC,
// STANDARDS-VALID MECHANISM"; a fixed literal is the simplest such
// mechanism and avoids introducing an unverified GUID-compression
// implementation into this Phase-0 spike.
struct IfcProjectSeed {
    std::string project_global_id;
    std::string project_name;
    std::string element_global_id;
    std::string element_name;
    std::string element_object_type;
    // Deterministic (name, value) scalar property pairs, preserved in
    // insertion order. Phase-0 simplification (disclosed in
    // src/interop/ifc/src/detail/scalar_property_codec.hpp): encoded as a
    // single delimited IfcText value on the element rather than a full
    // IfcPropertySet/IfcRelDefinesByProperties graph, to keep this spike's
    // IfcOpenShell entity-construction surface minimal. A later task may
    // replace this with a real property-set graph without changing this
    // public contract's shape.
    std::vector<std::pair<std::string, std::string>> scalar_properties;
};

// The fixed P0-T005 seed (Brief section 9 items 1-6). Exposed so every test
// and the standalone evidence executable
// (tests/integration/p0_t005_ifc_evidence.cpp) share exactly one seed
// definition rather than each hand-rolling their own.
[[nodiscard]] IfcProjectSeed MakeDefaultSeed();

// Outcome of one construct -> export -> close -> reopen -> locate -> compare
// round trip (Brief sections 7, 9, 10). A plain project-owned struct - see
// this header's own top comment on what must never appear here.
struct RoundTripEvidence {
    std::string schema_expected;
    std::string schema_reopened;
    bool export_success = false;
    bool reopen_success = false;
    bool project_found = false;
    bool project_identity_preserved = false;
    bool project_name_preserved = false;
    bool element_found = false;
    bool element_global_id_preserved = false;
    bool element_name_preserved = false;
    bool element_object_type_preserved = false;
    bool scalar_properties_preserved = false;
    bool overall_passed = false;
};

// Runs the full controlled round trip (Brief section 9): constructs `seed`,
// exports it to a fresh IFC4 file at `export_path`, closes the writer state,
// reopens the exported file, locates the expected project and element, and
// compares every required invariant (Brief section 10). Returns
// bim::foundation::Status::Ok() only when every mandatory step (seed
// construction, export, reopen, project/element location) itself succeeded
// without a first-party error; `out_evidence.overall_passed` separately
// records whether every semantic invariant held even when the mechanical
// steps all succeeded. Never throws: every IfcOpenShell exception is caught
// and translated to Status at the adapter boundary (Brief section 8).
[[nodiscard]] bim::foundation::Status RunIfcRoundTripProbe(const IfcProjectSeed& seed,
                                                           const std::filesystem::path& export_path,
                                                           RoundTripEvidence& out_evidence);

// First-party rejection contract (Brief section 11 / section 12's
// integration_ifc_missing_file / integration_ifc_malformed_input /
// integration_ifc_unsupported_schema). Returns Status::Ok() only if `path`
// exists, parses as a well-formed STEP/IFC file, and its FILE_SCHEMA is
// exactly "IFC4"; otherwise returns a first-party Status::Error() carrying a
// project-owned diagnostic message. Never an IfcOpenShell exception type,
// message format, or object crosses this boundary.
[[nodiscard]] bim::foundation::Status OpenAndValidateIfcFile(const std::filesystem::path& path);

} // namespace bim::ifc
