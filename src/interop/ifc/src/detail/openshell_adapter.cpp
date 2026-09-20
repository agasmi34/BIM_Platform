#include "detail/openshell_adapter.hpp"
#include "detail/scalar_property_codec.hpp"

// See this file's header (openshell_adapter.hpp) for the standing RISK
// DISCLOSURE covering every IfcOpenShell API call in this translation unit.
// CORR-004 (BIM-AA-P0-T005-CORR-004 v1.0) update: AA's Full Candidate
// Validation v1.4 provided the first real compile attempt of this file
// against the installed pinned dependency, and this correction was applied
// using the actual installed headers under
// C:\bimdeps\ifc\install\include\ifcparse\ as authority (per CORR-004
// section 4) - the entity-constructor argument types, the aggregate_of<T>
// API, and the optional-string accessor shapes touched by this file are no
// longer best-effort guesses for those specific call sites; the parts of
// this file not implicated by AA's v1.4 compiler diagnostics remain
// unverified per the standing RISK DISCLOSURE below.
//
// Include paths below follow IfcOpenShell's own public installed-header
// layout (ifcparse/*.h at the include root, e.g.
// <install-prefix>/include/ifcparse/IfcFile.h) as established by the
// accepted Dependency Resolution Phase C build/consume proof (Implementation
// Brief section 4 / section 19). Ifc4.h is the generated single-schema
// header produced because this dependency is built with SCHEMA_VERSIONS=4
// (IFC4 only, per ACR-P0-T005-001 / Brief section 4).
#include <ifcparse/Ifc4.h>
#include <ifcparse/IfcFile.h>

#include <boost/optional.hpp>

#include <exception>
#include <fstream>

namespace bim::ifc::detail {

namespace {

constexpr const char* kIfc4SchemaName = "IFC4";

// Produces a boost::optional<T> for an optional string-valued generated
// constructor argument: `boost::none` for an unset attribute, otherwise a
// constructed T wrapping `value`. Always instantiated as
// OptionalAttr<std::string> in this file (CORR-004
// BIM-AA-P0-T005-CORR-004 v1.0, section 2.2/2.4): the installed
// IfcOpenShell 0.8.5 generated constructors (Ifc4::IfcProject,
// Ifc4::IfcBuildingElementProxy - verified against the installed headers
// under C:\bimdeps\ifc\install\include\ifcparse\Ifc4.h) expose
// Name/Description/ObjectType directly as boost::optional<std::string>,
// never as a schema wrapper type such as IfcLabel/IfcText.
template <typename T> boost::optional<T> OptionalAttr(const std::string& value, bool set) {
    if (!set) {
        return boost::none;
    }
    return boost::optional<T>(T(value));
}

// Builds the fresh, geometry-free IFC4 entity graph for `seed` inside
// `file` and registers every entity via file.addEntity(...). Isolated from
// WriteSeedToIfc4File() below so the entity-construction step (the highest
// -risk portion of this adapter - see this file's own header RISK
// DISCLOSURE) is a single, small, reviewable function.
void BuildSeedEntities(IfcParse::IfcFile& file, const IfcProjectSeed& seed) {
    // IfcProject (IfcRoot: GlobalId, OwnerHistory, Name, Description;
    // IfcObject: ObjectType; IfcContext: LongName, Phase,
    // RepresentationContexts, UnitsInContext). Geometry-free: no
    // RepresentationContexts/UnitsInContext are populated (both optional in
    // IFC4 - Brief section 9: "The seed is geometry-free").
    // CORR-004 (BIM-AA-P0-T005-CORR-004 v1.0, section 2.1/2.2): the installed
    // generated constructor is
    // IfcProject(std::string, IfcOwnerHistory*, boost::optional<std::string>,
    // boost::optional<std::string>, boost::optional<std::string>,
    // boost::optional<std::string>, boost::optional<std::string>,
    // boost::optional<aggregate_of<IfcRepresentationContext>::ptr>,
    // IfcUnitAssignment*) - GlobalId is a plain std::string (never wrapped in
    // Ifc4::IfcGloballyUniqueId), and OwnerHistory/UnitsInContext are raw
    // nullable pointers (nullptr), never boost::none.
    auto* project = new Ifc4::IfcProject(seed.project_global_id, // GlobalId (std::string)
                                         nullptr,                // OwnerHistory (IfcOwnerHistory*)
                                         OptionalAttr<std::string>(seed.project_name, true), // Name
                                         boost::none, // Description
                                         boost::none, // ObjectType (N/A for IfcProject; left unset)
                                         boost::none, // LongName
                                         boost::none, // Phase
                                         boost::none, // RepresentationContexts
                                         nullptr      // UnitsInContext (IfcUnitAssignment*)
    );
    file.addEntity(project);

    // Phase-0 simplification (see probe.hpp's IfcProjectSeed doc comment
    // and scalar_property_codec.hpp): the deterministic scalar properties
    // are encoded into a single IfcText and carried on the element's
    // Description attribute, rather than a full
    // IfcPropertySet/IfcRelDefinesByProperties graph.
    const std::string encoded_properties = EncodeScalarProperties(seed.scalar_properties);

    // IfcBuildingElementProxy (IfcRoot: GlobalId, OwnerHistory, Name,
    // Description; IfcObject: ObjectType; IfcProduct: ObjectPlacement,
    // Representation; IfcElement: Tag; IfcBuildingElementProxy:
    // PredefinedType). ObjectPlacement and Representation are both left
    // unset - this element is intentionally non-geometric (Brief section 9
    // item 2: "one simple non-geometric IFC element").
    // CORR-004 (section 2.1/2.2): the installed generated constructor is
    // IfcBuildingElementProxy(std::string, IfcOwnerHistory*,
    // boost::optional<std::string>, boost::optional<std::string>,
    // boost::optional<std::string>, IfcObjectPlacement*,
    // IfcProductRepresentation*, boost::optional<std::string>,
    // boost::optional<IfcBuildingElementProxyTypeEnum::Value>) - same rule:
    // GlobalId is a plain std::string, OwnerHistory/ObjectPlacement/
    // Representation are raw nullable pointers (nullptr), never boost::none.
    auto* element = new Ifc4::IfcBuildingElementProxy(
        seed.element_global_id,                              // GlobalId (std::string)
        nullptr,                                             // OwnerHistory (IfcOwnerHistory*)
        OptionalAttr<std::string>(seed.element_name, true),  // Name
        OptionalAttr<std::string>(encoded_properties, true), // Description (encoded properties)
        OptionalAttr<std::string>(seed.element_object_type, true), // ObjectType
        nullptr,     // ObjectPlacement (IfcObjectPlacement*)
        nullptr,     // Representation (IfcProductRepresentation*)
        boost::none, // Tag
        boost::none  // PredefinedType
    );
    file.addEntity(element);
}

} // namespace

bim::foundation::Status WriteSeedToIfc4File(const IfcProjectSeed& seed,
                                            const std::filesystem::path& out_path) {
    try {
        // Constructs a fresh, empty, in-memory IFC4-schema file. Deliberately
        // NOT IfcParse::IfcFile(kIfc4SchemaName): the installed IfcFile.h
        // exposes both IfcFile(const std::string& path, filetype, bool
        // readonly) and IfcFile(const schema_definition*, filetype, const
        // std::string& path) - a bare const char*/std::string argument binds
        // to the PATH overload (open a file literally named "IFC4"), not the
        // schema-construction overload, which requires a
        // const IfcParse::schema_definition*. This additional defect was
        // found while applying CORR-004 by reading the installed header
        // (C:\bimdeps\ifc\install\include\ifcparse\IfcFile.h) as its
        // required authority: it compiles without any diagnostic (both
        // overloads are viable), so it was not among AA v1.4's compiler
        // -proven mismatches, but it would have silently tried to open a
        // nonexistent path instead of creating a new IFC4 file.
        // IfcParse::schema_by_name(...) is declared in IfcSchema.h, already
        // transitively included via IfcFile.h.
        IfcParse::IfcFile file(IfcParse::schema_by_name(kIfc4SchemaName));
        if (!file.good()) {
            return bim::foundation::Status::Error("bim::ifc: failed to construct a new in-memory "
                                                  "IFC4 file (IfcParse::IfcFile::good() "
                                                  "was false immediately after construction)");
        }

        BuildSeedEntities(file, seed);

        std::error_code mkdir_ec;
        if (out_path.has_parent_path()) {
            std::filesystem::create_directories(out_path.parent_path(), mkdir_ec);
        }

        std::ofstream out(out_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            return bim::foundation::Status::Error(
                "bim::ifc: failed to open export path for writing: " + out_path.string());
        }
        out << file;
        out.close();
        if (!out) {
            return bim::foundation::Status::Error(
                "bim::ifc: failed while writing exported IFC4 file: " + out_path.string());
        }

        return bim::foundation::Status::Ok();
    } catch (const std::exception& e) {
        return bim::foundation::Status::Error(std::string("bim::ifc: IfcOpenShell export threw: ") +
                                              e.what());
    } catch (...) {
        return bim::foundation::Status::Error(
            "bim::ifc: IfcOpenShell export threw an unknown exception");
    }
}

bim::foundation::Status ParseAndCheckIfc4Schema(const std::filesystem::path& path,
                                                std::string& out_schema) {
    out_schema.clear();

    // Detected first-party, before ever handing the path to IfcOpenShell
    // (Brief section 11's "missing IFC file" case): a missing file is a
    // first-party-detected condition, never an IfcOpenShell parse outcome.
    std::error_code exists_ec;
    if (!std::filesystem::exists(path, exists_ec) || exists_ec) {
        return bim::foundation::Status::Error("bim::ifc: IFC file does not exist: " +
                                              path.string());
    }

    try {
        IfcParse::IfcFile file(path.string());
        if (!file.good()) {
            return bim::foundation::Status::Error(
                "bim::ifc: IFC file failed to parse (IfcParse::IfcFile::good() was false): " +
                path.string());
        }

        const IfcParse::schema_definition* schema = file.schema();
        if (schema == nullptr) {
            return bim::foundation::Status::Error(
                "bim::ifc: IFC file parsed but reported no schema: " + path.string());
        }

        out_schema = schema->name();

        if (out_schema != kIfc4SchemaName) {
            return bim::foundation::Status::Error(
                "bim::ifc: unsupported IFC schema '" + out_schema + "', expected '" +
                std::string(kIfc4SchemaName) + "': " + path.string());
        }

        return bim::foundation::Status::Ok();
    } catch (const std::exception& e) {
        return bim::foundation::Status::Error(
            std::string("bim::ifc: IfcOpenShell parse threw (malformed input): ") + e.what() +
            " (" + path.string() + ")");
    } catch (...) {
        return bim::foundation::Status::Error(
            "bim::ifc: IfcOpenShell parse threw an unknown exception (malformed input): " +
            path.string());
    }
}

bim::foundation::Status ReadIfc4RoundTrip(const std::filesystem::path& path,
                                          const IfcProjectSeed& expected_seed,
                                          RoundTripEvidence& out_evidence) {
    // Deliberately does NOT reset `out_evidence` to a fresh RoundTripEvidence{}
    // here: the caller (bim::ifc::RunIfcRoundTripProbe(), ifc_probe.cpp) sets
    // out_evidence.export_success (and schema_expected) BEFORE calling this
    // function, and overall_passed below folds that already-set field in.
    // Only the reopen/locate/compare-side fields are written by this
    // function.
    out_evidence.schema_expected = kIfc4SchemaName;

    std::string reopened_schema;
    bim::foundation::Status schema_status = ParseAndCheckIfc4Schema(path, reopened_schema);
    out_evidence.schema_reopened = reopened_schema;
    if (!schema_status.ok()) {
        // reopen_success stays false; propagate the first-party diagnostic
        // rather than re-parsing (avoids a second, redundant IfcOpenShell
        // parse attempt).
        return schema_status;
    }
    out_evidence.reopen_success = true;

    try {
        IfcParse::IfcFile file(path.string());
        if (!file.good()) {
            return bim::foundation::Status::Error(
                "bim::ifc: IFC file re-parse failed on the second (evidence-comparison) pass: " +
                path.string());
        }

        // CORR-004 section 2.3: the installed aggregate_of<T>
        // (aggregate_of_instance.h) exposes size()/begin()/end() but has no
        // empty() member - use a cardinality check instead, preserving the
        // same "found at least one" semantics as the prior !empty() check.
        auto projects = file.instances_by_type<Ifc4::IfcProject>();
        if (projects && projects->size() > 0) {
            out_evidence.project_found = true;
            const Ifc4::IfcProject* project = *projects->begin();
            // IfcRoot::GlobalId() returns std::string directly (installed
            // Ifc4.h) - this cast is an identity cast, kept for clarity.
            out_evidence.project_identity_preserved =
                (static_cast<std::string>(project->GlobalId()) == expected_seed.project_global_id);

            // CORR-004 section 2.4: IfcRoot::Name() returns
            // boost::optional<std::string>; there is no hasName() helper in
            // the installed generated API. Check optional presence
            // explicitly, then read the contained value - never cast the
            // optional itself to std::string.
            const boost::optional<std::string> project_name = project->Name();
            out_evidence.project_name_preserved =
                static_cast<bool>(project_name) && (*project_name == expected_seed.project_name);
        }

        auto elements = file.instances_by_type<Ifc4::IfcBuildingElementProxy>();
        if (elements && elements->size() > 0) {
            out_evidence.element_found = true;
            const Ifc4::IfcBuildingElementProxy* element = *elements->begin();

            out_evidence.element_global_id_preserved =
                (static_cast<std::string>(element->GlobalId()) == expected_seed.element_global_id);

            const boost::optional<std::string> element_name = element->Name();
            out_evidence.element_name_preserved =
                static_cast<bool>(element_name) && (*element_name == expected_seed.element_name);

            // IfcObject::ObjectType() likewise returns
            // boost::optional<std::string>; there is no hasObjectType()
            // helper.
            const boost::optional<std::string> element_object_type = element->ObjectType();
            out_evidence.element_object_type_preserved =
                static_cast<bool>(element_object_type) &&
                (*element_object_type == expected_seed.element_object_type);

            // IfcRoot::Description() likewise returns
            // boost::optional<std::string>; there is no hasDescription()
            // helper. CORR-004 section 2.5: the prior expression here
            // (casting the optional directly to std::string as the
            // hasDescription()-guarded argument to DecodeScalarProperties)
            // is what produced the compiler's secondary "decode_ok: const
            // object must be initialized" diagnostic - fixing the optional
            // handling resolves it without touching decode-failure
            // semantics: scalar_properties_preserved stays at its
            // RoundTripEvidence default (false) whenever Description is
            // absent or decoding fails, exactly as before.
            const boost::optional<std::string> element_description = element->Description();
            if (element_description) {
                ScalarProperties decoded;
                const bool decode_ok = DecodeScalarProperties(*element_description, decoded);
                out_evidence.scalar_properties_preserved =
                    decode_ok && (decoded == expected_seed.scalar_properties);
            }
        }

        out_evidence.overall_passed =
            out_evidence.export_success && out_evidence.reopen_success &&
            out_evidence.project_found && out_evidence.project_identity_preserved &&
            out_evidence.project_name_preserved && out_evidence.element_found &&
            out_evidence.element_global_id_preserved && out_evidence.element_name_preserved &&
            out_evidence.element_object_type_preserved &&
            out_evidence.scalar_properties_preserved &&
            (out_evidence.schema_reopened == out_evidence.schema_expected);

        return bim::foundation::Status::Ok();
    } catch (const std::exception& e) {
        return bim::foundation::Status::Error(
            std::string("bim::ifc: IfcOpenShell evidence-comparison pass threw: ") + e.what());
    } catch (...) {
        return bim::foundation::Status::Error(
            "bim::ifc: IfcOpenShell evidence-comparison pass threw an unknown exception");
    }
}

} // namespace bim::ifc::detail
