#pragma once

#include "bim/geometry_api/geometry.hpp"
#include "bim/model/element_id.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"
#include "bim/model/validation.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

// P1-T002 Document Runtime & Dependency Recompute - the first production
// bim_document coordination layer (Architecture Gate BIM-AG-P1-T002; Task
// Record P1-T002; Implementation Brief P1-T002-IB).
//
// A Document owns the live Level / StraightWall state of one model, the
// private runtime identity association used for dependency evaluation, the
// Level -> StraightWall semantic dependencies, and the derived neutral wall
// geometry. All of that machinery is private: this header exposes only
// project-owned value types (bim::model values, the neutral
// bim::geometry_api extrusion specification) and a small result contract.
// No runtime graph identity, graph type, recompute plan, persistence type,
// transaction type or third-party type appears in this public surface; the
// DOCUMENT_PUBLIC_RUNTIME_NEUTRAL architecture rule (tools/
// architecture_checker.cmake) enforces that mechanically.
//
// Durable identity is bim::model::ElementId, and only ElementId is
// query-facing. Every mutation is strongly atomic: it is evaluated against a
// private staged copy of the whole runtime and published only when every
// validation, graph, recompute and derived-geometry step has succeeded. A
// failed mutation leaves every observable value, and the private runtime
// state, exactly as it was. No exception crosses a mutation boundary and no
// failure report allocates.
//
// P1-T003 extends this contract by exactly four public capabilities: the
// ElementHasDependents result code, DeleteElement, ListLevels and
// ListStraightWalls (Architecture Gate BIM-AG-P1-T003 section 8). Deletion
// has no cascade and no automatic re-host, and the list reads return
// committed value copies in ElementId ascending order. Nothing else about the
// public surface changed, and no private runtime identity is exposed.
//
// P1-T004 adds the document lifecycle: Save and Open (Architecture Gate
// BIM-AG-P1-T004 sections 10-13). Their outcome is a separate result contract,
// DocumentLifecycleResult, so DocumentResultCode - the contract every mutation
// and every command maps - is not extended and the command layer is untouched.
// Save writes the authoritative Level / StraightWall state and the document's
// pending transaction journal in one atomic durable step; Open rebuilds a
// complete, ready document (fresh private runtime identities, rebuilt
// dependencies, regenerated derived geometry) from a durable file and publishes
// it only when every step succeeded. How the document is stored, and the
// private journal bookkeeping behind it, never appear in this header: the
// lifecycle surface is a path and a result code.

namespace bim::document {

// Machine-readable outcome of a document mutation. Callers branch on the
// code; there is no diagnostic text to parse.
enum class DocumentResultCode : std::uint8_t {
    Ok,
    InvalidElement,         // the element value failed structural model validation
    DuplicateElementId,     // the ElementId is already used by a Level or a StraightWall
    LevelNotFound,          // the referenced hosting Level does not exist
    ElementNotFound,        // the update target does not exist
    ElementKindMismatch,    // the update target exists, but as the other element kind
    RehostNotAllowed,       // a StraightWall update tried to change its hosting Level
    NodeIdExhausted,        // the private runtime identity space is exhausted
    GraphRejected,          // the private dependency graph refused a registration or edge
    DerivedGeometryInvalid, // derived wall geometry overflowed or was not finite
    RecomputeFailed,        // dependency recompute failed or found inconsistent runtime state
    InternalFailure,        // an allocation, library or internal-invariant failure was contained
    ElementHasDependents,   // a Level cannot be deleted while any StraightWall is hosted by it
};

// Stable diagnostic name of a code; an out-of-range value yields "Unknown".
[[nodiscard]] const char* ToString(DocumentResultCode code) noexcept;

// Result of a mutation. `validation` is meaningful only when `code` is
// InvalidElement, where it carries the first structural rule the supplied
// value violated; otherwise it is ValidationCode::Ok. Trivially copyable and
// allocation-free, so it is always safe to build while reporting a failure.
struct DocumentResult {
    DocumentResultCode code = DocumentResultCode::Ok;
    bim::model::ValidationCode validation = bim::model::ValidationCode::Ok;

    [[nodiscard]] constexpr bool ok() const noexcept { return code == DocumentResultCode::Ok; }

    friend constexpr bool operator==(const DocumentResult&,
                                     const DocumentResult&) noexcept = default;
};

// Machine-readable outcome of a lifecycle operation (Save / Open). Callers
// branch on the code; there is no diagnostic text to parse. This contract is
// deliberately separate from DocumentResultCode.
enum class DocumentLifecycleResultCode : std::uint8_t {
    Ok,
    PersistenceFailure,       // the durable file could not be written, read or validated
    RuntimeCapacityExhausted, // the private runtime identity space is exhausted
    DerivedGeometryInvalid,   // derived wall geometry overflowed or was not finite
    RecomputeFailed,          // dependency recompute failed or found inconsistent runtime state
    InternalFailure,          // an allocation, library or internal-invariant failure was contained
};

// Stable diagnostic name of a code; an out-of-range value yields "Unknown".
[[nodiscard]] const char* ToString(DocumentLifecycleResultCode code) noexcept;

// Result of Save / Open. Trivially copyable and allocation-free, so it is
// always safe to build while reporting a failure.
struct DocumentLifecycleResult {
    DocumentLifecycleResultCode code = DocumentLifecycleResultCode::Ok;

    [[nodiscard]] constexpr bool ok() const noexcept {
        return code == DocumentLifecycleResultCode::Ok;
    }

    friend constexpr bool operator==(const DocumentLifecycleResult&,
                                     const DocumentLifecycleResult&) noexcept = default;
};

namespace detail {
struct State; // private runtime state; defined only in the implementation
} // namespace detail

// In-memory, single-threaded document. Not copyable (it owns private runtime
// state); movable. A default-constructed Document is empty and its
// construction cannot fail.
class Document {
public:
    Document() noexcept;
    ~Document();

    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;
    Document(Document&&) noexcept;
    Document& operator=(Document&&) noexcept;

    // Adds a Level. The Level starts with no dependent geometry.
    // Fails with InvalidElement (structural validation), or
    // DuplicateElementId when the id is already used by any Level or wall.
    [[nodiscard]] DocumentResult AddLevel(const bim::model::Level& level) noexcept;

    // Adds a StraightWall hosted by an existing Level and derives its neutral
    // extrusion specification. Fails with InvalidElement, DuplicateElementId,
    // LevelNotFound, or DerivedGeometryInvalid when the derived values are
    // not finite.
    [[nodiscard]] DocumentResult AddStraightWall(const bim::model::StraightWall& wall) noexcept;

    // Changes the elevation of an existing Level and recomputes the derived
    // geometry of every wall it hosts. Fails with ElementNotFound,
    // ElementKindMismatch (the id names a wall), InvalidElement (non-finite
    // elevation), or DerivedGeometryInvalid. A Level that hosts no wall
    // simply has nothing to recompute.
    [[nodiscard]] DocumentResult UpdateLevelElevation(const bim::model::ElementId& id,
                                                      double elevation) noexcept;

    // Replaces the defining values of an existing StraightWall (identified by
    // wall.id) and recomputes that wall's derived geometry. The hosting Level
    // cannot change: a different level_id fails with RehostNotAllowed. Other
    // failures: ElementNotFound, ElementKindMismatch (the id names a Level),
    // InvalidElement, DerivedGeometryInvalid.
    [[nodiscard]] DocumentResult UpdateStraightWall(const bim::model::StraightWall& wall) noexcept;

    // Deletes an existing StraightWall, or an existing Level that hosts no
    // StraightWall. There is no cascade: a Level that still hosts one or more
    // walls is rejected with ElementHasDependents, and nothing is re-hosted.
    // Fails with ElementNotFound when the id names no element. Every failure
    // leaves all committed state exactly as it was, and the survivors keep
    // their private runtime association unchanged.
    [[nodiscard]] DocumentResult DeleteElement(const bim::model::ElementId& id) noexcept;

    // Committed value reads. Each returns a copy of the committed value, or
    // an empty optional when no such element (of that kind) exists. No read
    // exposes internal containers, and no read ever observes a partially
    // applied mutation.
    [[nodiscard]] std::optional<bim::model::Level>
    FindLevel(const bim::model::ElementId& id) const noexcept;
    [[nodiscard]] std::optional<bim::model::StraightWall>
    FindStraightWall(const bim::model::ElementId& id) const noexcept;

    // The committed derived neutral extrusion specification of a wall.
    [[nodiscard]] std::optional<bim::geometry_api::LinearExtrusionSpec>
    FindWallGeometry(const bim::model::ElementId& wall_id) const noexcept;

    // Committed value copies of every Level / StraightWall, in ElementId
    // ascending order (the canonical byte order of ElementId). The order is a
    // semantic guarantee of the contract, never an artifact of internal
    // storage. An empty document yields an empty vector. Building the copy
    // can run out of memory, so these two reads are deliberately not noexcept;
    // they never expose or modify internal state.
    [[nodiscard]] std::vector<bim::model::Level> ListLevels() const;
    [[nodiscard]] std::vector<bim::model::StraightWall> ListStraightWalls() const;

    // Durably saves the document to `path`. One atomic step persists the
    // complete authoritative Level / StraightWall state and every pending
    // journal record of the mutations made since the last successful save (or
    // since the document was opened); previously saved journal history is kept.
    // A missing file is created. An unchanged document saves its state again and
    // adds no journal record, because there is nothing pending.
    //
    // On success the pending journal is cleared. On any failure the durable file
    // keeps its previous content, the in-memory document is untouched and its
    // pending journal is still pending. Saving never mutates the model and never
    // creates a journal record of its own. Fails with PersistenceFailure, or
    // InternalFailure for a contained allocation or internal failure.
    [[nodiscard]] DocumentLifecycleResult Save(const std::filesystem::path& path) noexcept;

    // Replaces `out_document` with the document stored at `path`. The stored
    // authoritative state is loaded and validated, and a complete document is
    // built from it on the side: Levels first, then StraightWalls, each in
    // ElementId order, with fresh private runtime identities, rebuilt
    // Level -> StraightWall dependencies and regenerated derived geometry.
    // Runtime identities are not persisted and are not expected to match those
    // of the document that was saved. Opening creates no journal record and
    // leaves the pending journal empty. `out_document` is changed only when
    // every step succeeded; on any failure it is left exactly as it was.
    // Fails with PersistenceFailure (missing, unreadable or invalid file, or
    // content the document rules reject), RuntimeCapacityExhausted,
    // DerivedGeometryInvalid, RecomputeFailed or InternalFailure.
    [[nodiscard]] static DocumentLifecycleResult Open(const std::filesystem::path& path,
                                                      Document& out_document) noexcept;

private:
    std::unique_ptr<detail::State> state_;
};

} // namespace bim::document
