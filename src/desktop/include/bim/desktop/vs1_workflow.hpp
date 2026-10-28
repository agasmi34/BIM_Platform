#pragma once

#include "bim/commands/commands.hpp"
#include "bim/document/document.hpp"
#include "bim/geometry_api/geometry.hpp"
#include "bim/model/element_id.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"
#include "bim/viewport/mesh.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

// P1-T006 Desktop VS1 End-to-End Acceptance - the Qt-free desktop workflow
// (Architecture Gate BIM-AG-P1-T006 sections 1-3, 9, 10; Implementation Brief
// P1-T006-IB sections 3-5).
//
// Vs1Workflow owns the one live bim::document::Document of the desktop and is
// the only thing the Qt layer talks to for BIM state:
//
//     UI -> Vs1Workflow -> bim::commands -> bim::document -> committed state
//        -> derived LinearExtrusionSpec -> bim::presentation::BuildWallMesh
//        -> RenderMeshData
//
// Every BIM mutation goes through bim::commands (CreateLevel, CreateStraightWall,
// ChangeLevelElevation, ChangeStraightWallGeometry, DeleteElement); a Document
// mutation method is never called from here. Reads of committed model values
// go through bim::query. Document::FindWallGeometry is used only to read the
// committed derived geometry handed to the presentation bridge, and
// Document::Save / Document::Open are the lifecycle operations. Nothing here
// tessellates, calls the CAD kernel, persists a render mesh or touches a
// renderer, Qt or a GPU type; this target links only bim::commands, bim::query,
// bim::document and bim::presentation (rule R26).
//
// Durable identity for a user-created element is generated here, before any
// mutation, with the production bim::model::GenerateElementId; a generation
// failure fails closed with no change to the document. No call throws.

namespace bim::desktop {

// The explicit tolerance the workflow hands to the presentation bridge. The
// derived wall frames are exactly orthonormal, so this only needs to be small
// and finite.
inline constexpr bim::geometry_api::GeometryTolerance kVs1GeometryTolerance{1.0e-6, 1.0e-6};

// Source of new durable identities. The default is the production generator;
// the seam exists so a test can prove that a generation failure is fail-closed.
using ElementIdGenerator = std::optional<bim::model::ElementId> (*)() noexcept;

enum class WorkflowCode : std::uint8_t {
    Ok,
    IdentityGenerationFailed, // no ElementId could be generated; nothing was changed
    CommandRejected,          // a bim::commands call refused; see WorkflowResult::command
    SaveFailed,               // Document::Save failed; see WorkflowResult::lifecycle
    OpenFailed,               // Document::Open failed; the live document is unchanged
    RenderRebuildFailed,      // a committed wall could not be turned into a render mesh
    InternalFailure,          // an allocation or internal failure was contained
};

// Result of a workflow operation. `command` is meaningful only for
// CommandRejected, `lifecycle` only for SaveFailed / OpenFailed; otherwise
// they hold their default (Ok) values.
struct WorkflowResult {
    WorkflowCode code = WorkflowCode::Ok;
    bim::commands::CommandResult command;
    bim::document::DocumentLifecycleResult lifecycle;

    [[nodiscard]] constexpr bool ok() const noexcept { return code == WorkflowCode::Ok; }
};

// Result of creating an element. `id` is the new durable identity, valid only
// when `result.ok()`.
struct CreatedElement {
    WorkflowResult result;
    bim::model::ElementId id;
};

// Result of rebuilding the render scene. `meshes` is the COMPLETE scene - one
// mesh per committed StraightWall in ElementId order - and is non-empty or
// empty only when the model genuinely has no walls. On any failure `result` is
// not ok and `meshes` is empty: a partially rebuilt scene is never returned.
struct SceneMeshes {
    WorkflowResult result;
    std::vector<bim::viewport::RenderMeshData> meshes;
};

class Vs1Workflow {
public:
    Vs1Workflow() noexcept;
    explicit Vs1Workflow(ElementIdGenerator generator) noexcept;
    ~Vs1Workflow();

    Vs1Workflow(const Vs1Workflow&) = delete;
    Vs1Workflow& operator=(const Vs1Workflow&) = delete;
    Vs1Workflow(Vs1Workflow&&) noexcept;
    Vs1Workflow& operator=(Vs1Workflow&&) noexcept;

    // --- mutations (each is one bim::commands call; atomic) -----------------
    [[nodiscard]] CreatedElement CreateLevel(double elevation) noexcept;

    [[nodiscard]] CreatedElement CreateStraightWall(const bim::model::ElementId& level_id,
                                                    const bim::model::Point2D& start,
                                                    const bim::model::Point2D& end,
                                                    double thickness, double height,
                                                    double base_offset) noexcept;

    [[nodiscard]] WorkflowResult ChangeLevelElevation(const bim::model::ElementId& level_id,
                                                      double elevation) noexcept;

    // Replaces start, end, thickness, base_offset and height; the wall keeps its
    // id and its hosting Level.
    [[nodiscard]] WorkflowResult ChangeWallGeometry(const bim::model::ElementId& wall_id,
                                                    const bim::model::Point2D& start,
                                                    const bim::model::Point2D& end,
                                                    double thickness, double base_offset,
                                                    double height) noexcept;

    // Removes a StraightWall, or a Level that hosts no wall.
    [[nodiscard]] WorkflowResult RemoveElement(const bim::model::ElementId& id) noexcept;

    // --- document lifecycle ------------------------------------------------------
    [[nodiscard]] WorkflowResult Save(const std::filesystem::path& path) noexcept;

    // Opens into a side document and replaces the live document only when every
    // step succeeded; on failure the live document is exactly as it was.
    [[nodiscard]] WorkflowResult Open(const std::filesystem::path& path) noexcept;

    // --- committed reads (bim::query) --------------------------------------------
    [[nodiscard]] std::vector<bim::model::Level> ListLevels() const noexcept;
    [[nodiscard]] std::vector<bim::model::StraightWall> ListWalls() const noexcept;
    [[nodiscard]] std::optional<bim::model::Level>
    FindLevel(const bim::model::ElementId& id) const noexcept;
    [[nodiscard]] std::optional<bim::model::StraightWall>
    FindWall(const bim::model::ElementId& id) const noexcept;

    // --- render scene ---------------------------------------------------------------
    // Rebuilds the complete wall mesh set from committed derived geometry:
    // bim::query::ListStraightWalls -> Document::FindWallGeometry ->
    // bim::presentation::BuildWallMesh, with kVs1GeometryTolerance.
    [[nodiscard]] SceneMeshes RebuildWallMeshes() const noexcept;

private:
    bim::document::Document document_;
    ElementIdGenerator generator_;
};

} // namespace bim::desktop
