#pragma once

// Public, vendor-neutral P0-T006 DWG probe contract (Execution Packet
// BIM-AA-P0-T006 v1.0 sections 4, 6, 10). This is a small, first-party,
// Phase-0 diagnostic/round-trip contract local to bim::dwg - it is NOT a
// general DWG model/import architecture (Packet section 4) and must never
// grow into one without a separate Architecture Authority decision.
//
// No ODA type (OdDb*, OdGe*, OdGi*, OdRx*, OdString, OdError, OdResult,
// smart-pointer typedefs, or any other Open Design Alliance symbol) may
// ever appear in this header or be reachable through it (Packet section 3;
// R17/DWG_PUBLIC_NEUTRAL below). Every third-party error is translated at
// the private adapter boundary (src/interop/dwg/src/detail/oda_adapter.cpp)
// into the project-owned bim::foundation::Status type used throughout this
// header.
//
// Unlike bim::ifc's P0-T005 spike (which constructs a fresh seed entity
// graph before exporting it), this module's controlled round trip starts
// from an existing, vendor-supplied AC1018 fixture (Packet section 8): read
// the fixture -> write it back out with an explicit AC1018 target version
// (Packet section 9: OdDb::vAC18, never "current") -> reopen -> verify that
// a specific, semantically-located MTEXT entity's content and geometry
// survived (Packet section 10). This module never authors new DWG entities.

#include "bim/foundation/status.hpp"

#include <filesystem>
#include <string>

namespace bim::dwg {

// Outcome of one read(source) -> write(export, AC1018) -> reopen(export) ->
// locate -> compare round trip (Packet sections 6, 9, 10). A plain
// project-owned struct - see this header's own top comment on what must
// never appear here. Every *_preserved field compares the located MTEXT's
// property in the reopened file against the same property captured from
// the original source file, using the fixture-local absolute numeric
// tolerance 1e-9 (Packet section 10) for every floating-point comparison;
// `text_content_preserved` is an exact string comparison.
struct DwgRoundTripEvidence {
    std::string source_version;   // DWG version header read from the source file, e.g. "AC1018".
    std::string reopened_version; // DWG version header read back from the reopened export.
    bool read_success = false;
    bool write_success = false;
    bool reopen_success = false;
    bool source_mtext_found = false;
    bool reopened_mtext_found = false;
    bool text_content_preserved = false;
    bool position_preserved = false;
    bool normal_preserved = false;
    bool direction_preserved = false;
    bool text_height_preserved = false;
    bool width_preserved = false;
    bool attachment_preserved = false;
    bool text_style_preserved = false;
    bool overall_passed = false;
};

// Runs the full controlled round trip (Packet sections 6, 9, 10): opens the
// existing DWG file at `source_path`, locates the known user-visible MTEXT
// entity whose content contains "External Reference" (located semantically
// by content, never by a hardcoded handle - Packet section 10), captures
// its properties, writes the loaded database back out to `export_path` with
// an explicit OdDb::vAC18 target version, reopens `export_path`, re-locates
// the same MTEXT there, and compares every required invariant. Returns
// bim::foundation::Status::Ok() only when every mandatory mechanical step
// (source open, MTEXT located in the source, write, reopen, MTEXT located
// in the reopened file) itself succeeded without a first-party error;
// `out_evidence.overall_passed` separately records whether every semantic
// invariant held even when every mechanical step succeeded. Never throws:
// every ODA exception is caught and translated to Status at the adapter
// boundary (Packet section 11).
//
// Rejected up front, before any ODA call, with a first-party
// Status::Error() and no write of any kind:
//   - `source_path` does not exist or is not a regular file;
//   - `source_path` and `export_path` are the same path (Packet section 11:
//     "input path equal to output path" - the original fixture must never
//     be written in place, Packet section 7);
//   - `export_path` already exists (Packet section 11: "unsafe input
//     overwrite" - this probe never overwrites an existing file).
[[nodiscard]] bim::foundation::Status RunDwgRoundTripProbe(const std::filesystem::path& source_path,
                                                            const std::filesystem::path& export_path,
                                                            DwgRoundTripEvidence& out_evidence);

// First-party validation contract (Packet section 11 / section 13's
// integration_dwg_missing_file). Returns Status::Ok() only if `path` exists,
// opens as a well-formed DWG file, and its header reports exactly the
// controlled accepted source version "AC1018" (Packet section 9 - this
// probe does not claim general support for other DWG versions);
// `out_version` is set to the version actually found whenever the file at
// least opened, even if the version check subsequently fails, so callers
// can report the actual version read. Otherwise returns a first-party
// Status::Error() carrying a project-owned diagnostic message. Never an ODA
// exception type, message format, or object crosses this boundary.
[[nodiscard]] bim::foundation::Status OpenAndValidateDwgFile(const std::filesystem::path& path,
                                                              std::string& out_version);

} // namespace bim::dwg
