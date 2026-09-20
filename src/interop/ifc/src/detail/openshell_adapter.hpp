#pragma once

// Private IfcOpenShell C++ adapter (Implementation Brief
// BIM-TASK-P0-T005-CLAUDE v1.0 section 7; ACR-P0-T005-001 sections 2, 6, 7).
// This header - and only its corresponding .cpp - may include an
// IfcOpenShell header (R14/IFC_OPEN_SHELL_ONLY_IFC_OWNER). It is never
// installed and never reachable from src/interop/ifc/include/** (R15's
// public-neutrality boundary).
//
// RISK DISCLOSURE (read before modifying openshell_adapter.cpp): this
// session has no execution channel against the pinned IfcOpenShell
// 0.8.5 / commit 16723d11 C++ core (no device_bash on the Windows
// worktree; building IfcOpenShell+Boost from source in this cloud sandbox
// is infeasible within this session). The generated-schema entity
// constructor signatures used in openshell_adapter.cpp (Ifc4::IfcProject,
// Ifc4::IfcBuildingElementProxy, IfcGloballyUniqueId, IfcLabel, IfcText,
// IfcParse::IfcFile's own construction/serialization/lookup surface) are
// authored from IfcOpenShell's well-known, long-stable public API shape at
// best-effort confidence, NOT independently verified against this exact
// pinned commit's generated headers. This is a materially higher-risk area
// than this module's own CMake wiring, architecture rules, and PowerShell
// bootstrap (all of which are either dependency-free or scriptable/testable
// without the third-party library itself). The Windows Execution Operator's
// first real `cmake --build` against the bootstrapped dependency is this
// file's first actual compilation - treat any build failure here as an
// ordinary dependency-adapter defect to fix minimum-delta (Brief section
// 20 item 6 / Execution Packet v1.1 section 14), not evidence the spike
// itself is infeasible, unless the failure matches one of Brief section
// 22's stop conditions.

#include "bim/ifc/probe.hpp"

#include <filesystem>

namespace bim::ifc::detail {

// Constructs `seed` as a fresh, geometry-free IFC4 entity graph (one
// IfcProject, one IfcBuildingElementProxy carrying the encoded scalar
// properties in its Description - see scalar_property_codec.hpp) and writes
// it to `out_path`, then closes the writer state. Never throws.
[[nodiscard]] bim::foundation::Status WriteSeedToIfc4File(const IfcProjectSeed& seed,
                                                          const std::filesystem::path& out_path);

// Reopens the IFC4 file at `path`, locates the expected IfcProject and
// IfcBuildingElementProxy, decodes the scalar properties, and fills every
// field of `out_evidence` (schema_reopened, *_found, *_preserved,
// overall_passed) by comparing what was read against `expected_seed`. Never
// throws.
[[nodiscard]] bim::foundation::Status ReadIfc4RoundTrip(const std::filesystem::path& path,
                                                        const IfcProjectSeed& expected_seed,
                                                        RoundTripEvidence& out_evidence);

// Parses `path` as a STEP/IFC file and reports its FILE_SCHEMA name (e.g.
// "IFC4", "IFC2X3") without requiring the file to contain any specific
// entity - used by OpenAndValidateIfcFile() (bim::ifc's public contract)
// and by ReadIfc4RoundTrip() above to populate `schema_reopened`. Returns
// Status::Error() for a missing file, a malformed/unparseable file, or a
// file whose schema is not exactly "IFC4"; `out_schema` is set to the
// parsed schema name whenever parsing itself succeeded, even if the schema
// check subsequently fails, so callers can report the actual schema found.
[[nodiscard]] bim::foundation::Status ParseAndCheckIfc4Schema(const std::filesystem::path& path,
                                                              std::string& out_schema);

} // namespace bim::ifc::detail
