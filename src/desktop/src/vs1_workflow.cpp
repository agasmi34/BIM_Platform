#include "bim/desktop/vs1_workflow.hpp"

#include "bim/presentation/wall_mesh.hpp"
#include "bim/query/query.hpp"

#include <utility>

// P1-T006 desktop VS1 workflow (see vs1_workflow.hpp for the boundary rules).
// Mutations are bim::commands calls only; reads are bim::query calls; the one
// Document read that is not a query is FindWallGeometry, used to fetch the
// committed derived geometry for the presentation bridge.

namespace bim::desktop {

namespace {

[[nodiscard]] WorkflowResult Rejected(const bim::commands::CommandResult& command) noexcept {
    WorkflowResult result;
    result.code = WorkflowCode::CommandRejected;
    result.command = command;
    return result;
}

[[nodiscard]] WorkflowResult Failure(WorkflowCode code) noexcept {
    WorkflowResult result;
    result.code = code;
    return result;
}

[[nodiscard]] WorkflowResult FromCommand(const bim::commands::CommandResult& command) noexcept {
    return command.ok() ? WorkflowResult{} : Rejected(command);
}

} // namespace

Vs1Workflow::Vs1Workflow() noexcept : generator_(&bim::model::GenerateElementId) {}

Vs1Workflow::Vs1Workflow(ElementIdGenerator generator) noexcept
    : generator_(generator != nullptr ? generator : &bim::model::GenerateElementId) {}

Vs1Workflow::~Vs1Workflow() = default;
Vs1Workflow::Vs1Workflow(Vs1Workflow&&) noexcept = default;
Vs1Workflow& Vs1Workflow::operator=(Vs1Workflow&&) noexcept = default;

CreatedElement Vs1Workflow::CreateLevel(double elevation) noexcept {
    CreatedElement created;
    // Identity first: a generation failure must leave the document untouched.
    const std::optional<bim::model::ElementId> id = generator_();
    if (!id.has_value() || !bim::model::IsValid(*id)) {
        created.result = Failure(WorkflowCode::IdentityGenerationFailed);
        return created;
    }
    bim::model::Level level;
    level.id = *id;
    level.elevation = elevation;
    created.result = FromCommand(bim::commands::CreateLevel(document_, level));
    if (created.result.ok()) {
        created.id = *id;
    }
    return created;
}

CreatedElement Vs1Workflow::CreateStraightWall(const bim::model::ElementId& level_id,
                                               const bim::model::Point2D& start,
                                               const bim::model::Point2D& end, double thickness,
                                               double height, double base_offset) noexcept {
    CreatedElement created;
    const std::optional<bim::model::ElementId> id = generator_();
    if (!id.has_value() || !bim::model::IsValid(*id)) {
        created.result = Failure(WorkflowCode::IdentityGenerationFailed);
        return created;
    }
    bim::model::StraightWall wall;
    wall.id = *id;
    wall.level_id = level_id;
    wall.start = start;
    wall.end = end;
    wall.thickness = thickness;
    wall.height = height;
    wall.base_offset = base_offset;
    created.result = FromCommand(bim::commands::CreateStraightWall(document_, wall));
    if (created.result.ok()) {
        created.id = *id;
    }
    return created;
}

WorkflowResult Vs1Workflow::ChangeLevelElevation(const bim::model::ElementId& level_id,
                                                 double elevation) noexcept {
    return FromCommand(bim::commands::ChangeLevelElevation(document_, level_id, elevation));
}

WorkflowResult Vs1Workflow::ChangeWallGeometry(const bim::model::ElementId& wall_id,
                                               const bim::model::Point2D& start,
                                               const bim::model::Point2D& end, double thickness,
                                               double base_offset, double height) noexcept {
    return FromCommand(bim::commands::ChangeStraightWallGeometry(document_, wall_id, start, end,
                                                                 thickness, base_offset, height));
}

WorkflowResult Vs1Workflow::RemoveElement(const bim::model::ElementId& id) noexcept {
    return FromCommand(bim::commands::DeleteElement(document_, id));
}

WorkflowResult Vs1Workflow::Save(const std::filesystem::path& path) noexcept {
    const bim::document::DocumentLifecycleResult saved = document_.Save(path);
    if (saved.ok()) {
        return WorkflowResult{};
    }
    WorkflowResult result;
    result.code = WorkflowCode::SaveFailed;
    result.lifecycle = saved;
    return result;
}

WorkflowResult Vs1Workflow::Open(const std::filesystem::path& path) noexcept {
    try {
        bim::document::Document side;
        const bim::document::DocumentLifecycleResult opened =
            bim::document::Document::Open(path, side);
        if (!opened.ok()) {
            WorkflowResult result;
            result.code = WorkflowCode::OpenFailed;
            result.lifecycle = opened;
            return result;
        }
        // Every step succeeded: only now does the live document change.
        document_ = std::move(side);
        return WorkflowResult{};
    } catch (...) {
        return Failure(WorkflowCode::InternalFailure);
    }
}

std::vector<bim::model::Level> Vs1Workflow::ListLevels() const noexcept {
    try {
        return bim::query::ListLevels(document_);
    } catch (...) {
        return {};
    }
}

std::vector<bim::model::StraightWall> Vs1Workflow::ListWalls() const noexcept {
    try {
        return bim::query::ListStraightWalls(document_);
    } catch (...) {
        return {};
    }
}

std::optional<bim::model::Level>
Vs1Workflow::FindLevel(const bim::model::ElementId& id) const noexcept {
    return bim::query::FindLevel(document_, id);
}

std::optional<bim::model::StraightWall>
Vs1Workflow::FindWall(const bim::model::ElementId& id) const noexcept {
    return bim::query::FindStraightWall(document_, id);
}

SceneMeshes Vs1Workflow::RebuildWallMeshes() const noexcept {
    SceneMeshes scene;
    try {
        const std::vector<bim::model::StraightWall> walls =
            bim::query::ListStraightWalls(document_);
        std::vector<bim::viewport::RenderMeshData> meshes;
        meshes.reserve(walls.size());
        for (const bim::model::StraightWall& wall : walls) {
            const auto spec = document_.FindWallGeometry(wall.id);
            if (!spec.has_value()) {
                scene.result = Failure(WorkflowCode::RenderRebuildFailed);
                return scene;
            }
            bim::presentation::WallMeshResult built =
                bim::presentation::BuildWallMesh(*spec, kVs1GeometryTolerance);
            if (!built.ok()) {
                scene.result = Failure(WorkflowCode::RenderRebuildFailed);
                return scene;
            }
            meshes.push_back(std::move(built.mesh));
        }
        // Complete scene only: publish it in one move.
        scene.meshes = std::move(meshes);
        return scene;
    } catch (...) {
        scene.meshes.clear();
        scene.result = Failure(WorkflowCode::InternalFailure);
        return scene;
    }
}

} // namespace bim::desktop
