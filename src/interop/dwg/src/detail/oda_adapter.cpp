// Private ODA Drawings C++ adapter implementation - see detail/oda_adapter.hpp
// for the full grounding disclosure and ODA-runtime-lifetime discipline this
// file follows. This is the ONLY .cpp permitted to include an ODA header or
// name an ODA type (R16/ODA_DRAWINGS_ONLY_DWG_OWNER).

#include "detail/oda_adapter.hpp"

// ODA headers - grounded directly against the locked toolkit (see
// detail/oda_adapter.hpp's top comment). Include set mirrors the vendor's
// own Drawing/Examples/SimpleReadWriteEx/SimpleReadWrite.cpp exactly, plus
// the additional headers this adapter needs beyond that minimal example
// (DbMText.h/DbTextStyleTableRecord.h for the semantic-fidelity comparison;
// DbBlockTable.h/DbBlockTableRecord.h/DbSymbolTable.h to walk every block -
// model space, paper space, and every block definition - since the MTEXT
// must be located semantically rather than assumed to live in a specific
// space; OdAnsiString.h for the one wide-to-narrow string conversion this
// file needs).
#include "OdaCommon.h"
#include "RxDynamicModule.h"
#include "StaticRxObject.h"
#include "DbDatabase.h"
#include "DbBlockTable.h"
#include "DbBlockTableRecord.h"
#include "DbSymbolTable.h"
#include "DbMText.h"
#include "DbTextStyleTableRecord.h"
#include "ExSystemServices.h"
#include "ExHostAppServices.h"
#include "OdFileBuf.h"
#include "OdAnsiString.h"
#include "OdError.h"

#include <cmath>
#include <fstream>

namespace bim::dwg::detail {

namespace {

// Combined system/host-app services object, exactly mirroring the vendor's
// own SimpleReadWrite.cpp MyServices class (see detail/oda_adapter.hpp's
// grounding disclosure).
class OdaServices : public ExSystemServices, public ExHostAppServices {
protected:
    ODRX_USING_HEAP_OPERATORS(ExSystemServices);
};

// RAII bracket for exactly one odInitialize()/odUninitialize() pair (see
// detail/oda_adapter.hpp's "ODA runtime lifetime discipline" comment on why
// this adapter never nests or repeats these calls within one process).
// odUninitialize()'s own failure is swallowed rather than allowed to escape
// a destructor - mirroring SimpleReadWrite.cpp's own
// catch(OdError&)/catch(...) wrapping of that same call, adapted for RAII
// since a destructor has no Status to return.
class OdaRuntimeGuard {
public:
    explicit OdaRuntimeGuard(OdStaticRxObject<OdaServices>& svcs) { odInitialize(&svcs); }

    ~OdaRuntimeGuard() {
        try {
            odUninitialize();
        } catch (...) {
            // Deliberately swallowed - see class comment above.
        }
    }

    OdaRuntimeGuard(const OdaRuntimeGuard&) = delete;
    OdaRuntimeGuard& operator=(const OdaRuntimeGuard&) = delete;
};

// Converts an OdString (ODA's internal wide-character string type) to a
// narrow std::string via OdAnsiString - the ODA-provided conversion path
// (Kernel/Include/OdAnsiString.h), rather than a hand-rolled wide/narrow
// conversion. CP_UNDEFINED is used because every string this adapter reads
// (MTEXT content in the locked fixture, OdError::description() messages,
// text style names) is expected to be plain ASCII; this is not a general
// Unicode-DWG-content conversion path.
[[nodiscard]] std::string OdStringToStd(const OdString& value) {
    const OdAnsiString ansi(value, CP_UNDEFINED);
    return std::string(ansi.c_str());
}

// A plain, first-party snapshot of the subset of an OdDbMText entity's
// properties this spike verifies (Packet section 10). Holds no ODA type -
// this is what crosses out of this ODA-owning translation unit into the
// rest of the adapter and, ultimately, into bim::dwg::DwgRoundTripEvidence.
struct MTextSnapshot {
    std::string content;
    double location_x = 0.0;
    double location_y = 0.0;
    double location_z = 0.0;
    double normal_x = 0.0;
    double normal_y = 0.0;
    double normal_z = 0.0;
    double direction_x = 0.0;
    double direction_y = 0.0;
    double direction_z = 0.0;
    double text_height = 0.0;
    double width = 0.0;
    int attachment = 0;
    std::string text_style_name;
};

// Fills `out` from `pMText`'s real accessors (every one confirmed to exist
// with this name/signature against Drawing/Include/DbMText.h - see
// detail/oda_adapter.hpp). textStyle() returns an OdDbObjectId that must be
// resolved to the OdDbTextStyleTableRecord it points to and its name read
// (Packet section 10: "resolved text-style semantics", not just the raw
// object id) - a null/unresolvable style id leaves text_style_name empty
// rather than throwing, since Packet section 10 asks this adapter to
// *report* resolved text-style semantics, not to treat a missing style
// resolution as a mechanical failure of the whole probe.
void CaptureMTextSnapshot(const OdDbMTextPtr& pMText, MTextSnapshot& out) {
    out.content = OdStringToStd(pMText->contents());

    const OdGePoint3d location = pMText->location();
    out.location_x = location.x;
    out.location_y = location.y;
    out.location_z = location.z;

    const OdGeVector3d normal = pMText->normal();
    out.normal_x = normal.x;
    out.normal_y = normal.y;
    out.normal_z = normal.z;

    const OdGeVector3d direction = pMText->direction();
    out.direction_x = direction.x;
    out.direction_y = direction.y;
    out.direction_z = direction.z;

    out.text_height = pMText->textHeight();
    out.width = pMText->width();
    out.attachment = static_cast<int>(pMText->attachment());

    out.text_style_name.clear();
    const OdDbObjectId style_id = pMText->textStyle();
    if (!style_id.isNull()) {
        const OdDbObjectPtr style_obj = style_id.safeOpenObject(OdDb::kForRead, true);
        if (!style_obj.isNull()) {
            const OdDbTextStyleTableRecordPtr style_record =
                OdDbTextStyleTableRecord::cast(style_obj);
            if (!style_record.isNull()) {
                out.text_style_name = OdStringToStd(style_record->getName());
            }
        }
    }
}

// Walks every block table record in `db` (model space, paper space, and
// every block definition - Packet section 10 does not constrain which
// space/block owns the target MTEXT, and the "External Reference" content
// this probe searches for may legitimately live inside an Xref-related
// block rather than model space directly) and returns the first MTEXT
// entity whose content contains the substring `needle`. Returns
// Status::Error() if none is found; never throws (every ODA call in this
// function's own walk is expected to succeed against a database ODA itself
// already loaded without error - a corrupt block/entity encountered mid-walk
// is still translated to Status::Error() by this function's own try/catch,
// mirroring every other adapter boundary in this file).
[[nodiscard]] bim::foundation::Status FindMTextContaining(const OdDbDatabasePtr& db,
                                                           const char* needle,
                                                           MTextSnapshot& out_snapshot) {
    try {
        const OdDbBlockTablePtr block_table =
            OdDbBlockTable::cast(db->getBlockTableId().safeOpenObject(OdDb::kForRead));
        if (block_table.isNull()) {
            return bim::foundation::Status::Error(
                "bim::dwg: could not open the database's block table");
        }

        for (OdDbSymbolTableIteratorPtr bt_iter = block_table->newIterator(); !bt_iter->done();
             bt_iter->step()) {
            const OdDbBlockTableRecordPtr block_record =
                OdDbBlockTableRecord::cast(bt_iter->getRecordId().safeOpenObject(OdDb::kForRead));
            if (block_record.isNull()) {
                continue;
            }

            for (OdDbObjectIteratorPtr ent_iter = block_record->newIterator(); !ent_iter->done();
                 ent_iter->step()) {
                const OdDbObjectPtr entity_obj = ent_iter->objectId().safeOpenObject(OdDb::kForRead);
                if (entity_obj.isNull()) {
                    continue;
                }
                const OdDbMTextPtr mtext = OdDbMText::cast(entity_obj);
                if (mtext.isNull()) {
                    continue;
                }
                const OdAnsiString content(mtext->contents(), CP_UNDEFINED);
                if (content.find(needle) >= 0) {
                    CaptureMTextSnapshot(mtext, out_snapshot);
                    return bim::foundation::Status::Ok();
                }
            }
        }
    } catch (const OdError& e) {
        return bim::foundation::Status::Error("bim::dwg: ODA error while searching for MTEXT: " +
                                               OdStringToStd(e.description()));
    } catch (...) {
        return bim::foundation::Status::Error(
            "bim::dwg: unknown error while searching for MTEXT");
    }

    return bim::foundation::Status::Error(
        std::string("bim::dwg: no MTEXT entity found containing \"") + needle + "\"");
}

constexpr double kFixtureLocalAbsoluteTolerance = 1e-9; // Packet section 10.

[[nodiscard]] bool NearlyEqual(double a, double b) {
    return std::fabs(a - b) <= kFixtureLocalAbsoluteTolerance;
}

// Compares `before` (captured from the source fixture) against `after`
// (captured from the reopened AC18 export) and fills every *_preserved /
// overall_passed field of `out_evidence` (Packet section 10). Every
// floating-point comparison uses the fixture-local absolute tolerance
// above; text_content_preserved and text_style_preserved are exact string
// comparisons (a text style NAME is a symbol-table identity, not a
// measurement, so it is compared exactly rather than with numeric
// tolerance).
void CompareSnapshots(const MTextSnapshot& before, const MTextSnapshot& after,
                       DwgRoundTripEvidence& out_evidence) {
    out_evidence.text_content_preserved = (before.content == after.content);
    out_evidence.position_preserved = NearlyEqual(before.location_x, after.location_x) &&
                                      NearlyEqual(before.location_y, after.location_y) &&
                                      NearlyEqual(before.location_z, after.location_z);
    out_evidence.normal_preserved = NearlyEqual(before.normal_x, after.normal_x) &&
                                    NearlyEqual(before.normal_y, after.normal_y) &&
                                    NearlyEqual(before.normal_z, after.normal_z);
    out_evidence.direction_preserved = NearlyEqual(before.direction_x, after.direction_x) &&
                                       NearlyEqual(before.direction_y, after.direction_y) &&
                                       NearlyEqual(before.direction_z, after.direction_z);
    out_evidence.text_height_preserved = NearlyEqual(before.text_height, after.text_height);
    out_evidence.width_preserved = NearlyEqual(before.width, after.width);
    out_evidence.attachment_preserved = (before.attachment == after.attachment);
    out_evidence.text_style_preserved = (before.text_style_name == after.text_style_name);

    out_evidence.overall_passed = out_evidence.text_content_preserved &&
                                  out_evidence.position_preserved && out_evidence.normal_preserved &&
                                  out_evidence.direction_preserved &&
                                  out_evidence.text_height_preserved && out_evidence.width_preserved &&
                                  out_evidence.attachment_preserved &&
                                  out_evidence.text_style_preserved;
}

} // namespace

bim::foundation::Status ReadDwgHeaderVersion(const std::filesystem::path& path,
                                              std::string& out_version) {
    out_version.clear();

    // Deliberately a raw first-6-ASCII-byte header read, not an ODA call:
    // this is the SAME method the P0-T006 execution-baseline checkpoint
    // already used to independently confirm the locked fixture is AC1018
    // (Packet section 8), so this adapter's own version gate is grounded in
    // an already-operator-verified technique rather than an unverified
    // assumption about OdDb::DwgVersionToStr()'s exact output format (that
    // ODA utility exists - DbDatabase.h line ~2767 - but this file was never
    // compiled and run to confirm its return string is byte-identical to
    // the raw file header). This also lets a missing/non-DWG file be
    // rejected without paying for an ODA runtime spin-up at all (Packet
    // section 11's "missing input" / "unreadable/non-DWG input").
    std::error_code exists_ec;
    if (!std::filesystem::exists(path, exists_ec) || exists_ec) {
        return bim::foundation::Status::Error("bim::dwg: file does not exist: " + path.string());
    }
    if (!std::filesystem::is_regular_file(path, exists_ec) || exists_ec) {
        return bim::foundation::Status::Error("bim::dwg: not a regular file: " + path.string());
    }

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return bim::foundation::Status::Error("bim::dwg: could not open file: " + path.string());
    }

    char header[6] = {};
    in.read(header, sizeof(header));
    if (in.gcount() != static_cast<std::streamsize>(sizeof(header))) {
        return bim::foundation::Status::Error(
            "bim::dwg: file is too small to contain a DWG version header: " + path.string());
    }

    // Every real DWG version header is of the form "AC" followed by four
    // digits (AC1018, AC1032, ...) - a cheap, format-only sanity check
    // before accepting the bytes as a version string, so an arbitrary
    // non-DWG file is rejected here rather than silently reported as some
    // garbage "version".
    if (header[0] != 'A' || header[1] != 'C') {
        return bim::foundation::Status::Error(
            "bim::dwg: file does not begin with a recognizable DWG version header: " +
            path.string());
    }

    out_version.assign(header, sizeof(header));
    return bim::foundation::Status::Ok();
}

bim::foundation::Status RunControlledRoundTrip(const std::filesystem::path& source_path,
                                                const std::filesystem::path& export_path,
                                                DwgRoundTripEvidence& out_evidence) {
    try {
        OdStaticRxObject<OdaServices> svcs;
        OdaRuntimeGuard guard(svcs); // Single odInitialize/odUninitialize pair for this entire call.

        // ----- read source -----
        OdDbDatabasePtr source_db;
        try {
            source_db = svcs.readFile(source_path.wstring().c_str());
        } catch (const OdError& e) {
            return bim::foundation::Status::Error("bim::dwg: failed to read source file: " +
                                                   OdStringToStd(e.description()));
        }
        if (source_db.isNull()) {
            return bim::foundation::Status::Error(
                "bim::dwg: ODA returned a null database reading source file: " +
                source_path.string());
        }
        out_evidence.read_success = true;
        out_evidence.source_version = OdStringToStd(
            OdString(OdDb::DwgVersionToStr(source_db->originalFileVersion())));

        // ----- locate the source MTEXT -----
        MTextSnapshot source_snapshot;
        const bim::foundation::Status source_locate_status =
            FindMTextContaining(source_db, "External Reference", source_snapshot);
        if (!source_locate_status.ok()) {
            return source_locate_status;
        }
        out_evidence.source_mtext_found = true;

        // ----- write the SAME loaded database back out as AC18 -----
        // Packet section 9: explicit OdDb::vAC18 target version, never
        // OdDb::kDHL_CURRENT. Packet section 7 / section 11: the export
        // path must not already exist - this probe never overwrites.
        std::error_code export_exists_ec;
        if (std::filesystem::exists(export_path, export_exists_ec)) {
            return bim::foundation::Status::Error(
                "bim::dwg: refusing to overwrite an existing export path: " +
                export_path.string());
        }
        try {
            OdWrFileBuf export_buf(export_path.wstring().c_str());
            source_db->writeFile(&export_buf, OdDb::kDwg, OdDb::vAC18, true /* save preview */);
        } catch (const OdError& e) {
            return bim::foundation::Status::Error("bim::dwg: failed to write AC18 export: " +
                                                   OdStringToStd(e.description()));
        }
        out_evidence.write_success = true;

        // ----- reopen the export -----
        OdDbDatabasePtr reopened_db;
        try {
            reopened_db = svcs.readFile(export_path.wstring().c_str());
        } catch (const OdError& e) {
            return bim::foundation::Status::Error("bim::dwg: failed to reopen AC18 export: " +
                                                   OdStringToStd(e.description()));
        }
        if (reopened_db.isNull()) {
            return bim::foundation::Status::Error(
                "bim::dwg: ODA returned a null database reopening the export file: " +
                export_path.string());
        }
        out_evidence.reopen_success = true;
        out_evidence.reopened_version = OdStringToStd(
            OdString(OdDb::DwgVersionToStr(reopened_db->originalFileVersion())));

        // ----- locate the reopened MTEXT -----
        MTextSnapshot reopened_snapshot;
        const bim::foundation::Status reopened_locate_status =
            FindMTextContaining(reopened_db, "External Reference", reopened_snapshot);
        if (!reopened_locate_status.ok()) {
            return reopened_locate_status;
        }
        out_evidence.reopened_mtext_found = true;

        // ----- compare -----
        CompareSnapshots(source_snapshot, reopened_snapshot, out_evidence);

        return bim::foundation::Status::Ok();
    } catch (const OdError& e) {
        return bim::foundation::Status::Error("bim::dwg: unexpected ODA error: " +
                                               OdStringToStd(e.description()));
    } catch (const std::exception& e) {
        return bim::foundation::Status::Error(std::string("bim::dwg: unexpected error: ") +
                                               e.what());
    } catch (...) {
        return bim::foundation::Status::Error("bim::dwg: unknown unexpected error");
    }
}

} // namespace bim::dwg::detail
