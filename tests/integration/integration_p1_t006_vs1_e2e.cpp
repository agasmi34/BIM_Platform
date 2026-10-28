// P1-T006 Desktop VS1 End-to-End Acceptance - the Phase-1 vertical slice, proven
// through real production boundaries (Architecture Gate BIM-AG-P1-T006 sections
// 1-10; Implementation Brief P1-T006-IB section 8).
//
//   bim::commands -> bim::document (Level -> StraightWall dependency, derived
//   LinearExtrusionSpec) -> bim::query (committed reads)
//   -> bim::geometry_api::MakeLinearExtrusion / ObserveFaces (real kernel)
//   -> bim::model::ResolveWallFaceReference (the six production semantic faces)
//   -> Document::Save / Document::Open (schema v2) -> regenerated geometry
//   -> bim::presentation::BuildWallMesh -> ValidateMesh
//   -> the headless bim::viewport_bgfx::Renderer CreateMesh / DestroyMesh path.
//
// The first test case follows the 25 acceptance steps in order. The remaining
// test cases drive the same slice through the Qt-free bim::desktop_vs1_workflow
// the desktop executable uses, and prove its fail-closed behaviour.
//
// This target links the production modules for TEST COMPOSITION ONLY; the
// production dependency directions are enforced by rules R23-R27.
#include "bim/commands/commands.hpp"
#include "bim/desktop/vs1_workflow.hpp"
#include "bim/document/document.hpp"
#include "bim/geometry_api/face_observation.hpp"
#include "bim/geometry_api/geometry.hpp"
#include "bim/model/element_id.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"
#include "bim/model/validation.hpp"
#include "bim/model/wall_face_reference.hpp"
#include "bim/model/wall_face_reference_resolver.hpp"
#include "bim/presentation/wall_mesh.hpp"
#include "bim/query/query.hpp"
#include "bim/viewport/error.hpp"
#include "bim/viewport/mesh.hpp"
#include "bim/viewport_bgfx/renderer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <optional>
#include <sstream>
#include <system_error>
#include <vector>

namespace {

using bim::document::Document;
using bim::geometry_api::FaceObservationResult;
using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::geometry_api::SolidResult;
using bim::geometry_api::SurfaceKind;
using bim::model::ElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;
using bim::model::WallFaceFrame;
using bim::model::WallFaceReference;
using bim::model::WallFaceResolutionResult;
using bim::model::WallFaceResolutionStatus;
using bim::model::WallFaceRole;
using bim::presentation::BuildWallMesh;
using bim::viewport::RenderMeshData;
using bim::viewport::ViewportErrorCode;
using bim::viewport_bgfx::Renderer;
using bim::viewport_bgfx::RendererCreateInfo;
using bim::viewport_bgfx::RenderMeshHandle;

// Kernel build tolerance and the (looser) resolution tolerance: far tighter than
// any real separation between two named planes of this slice (the thinnest
// dimension is 0.2), loose enough that kernel round-off can never fail a match.
constexpr GeometryTolerance kBuildTolerance{1.0e-6, 1.0e-8};
constexpr GeometryTolerance kResolveTolerance{1.0e-4, 1.0e-6};
constexpr double kEps = 1.0e-9;

constexpr std::array<WallFaceRole, 6> kAllRoles{WallFaceRole::Bottom, WallFaceRole::Top,
                                                WallFaceRole::Start,  WallFaceRole::End,
                                                WallFaceRole::SideA,  WallFaceRole::SideB};

class TempDatabasePath {
public:
    TempDatabasePath() : path_(MakeUniquePath()) { RemoveFiles(); }
    ~TempDatabasePath() { RemoveFiles(); }

    TempDatabasePath(const TempDatabasePath&) = delete;
    TempDatabasePath& operator=(const TempDatabasePath&) = delete;
    TempDatabasePath(TempDatabasePath&&) = delete;
    TempDatabasePath& operator=(TempDatabasePath&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }

private:
    static std::filesystem::path MakeUniquePath() {
        static std::atomic<std::uint64_t> counter{0};
        const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
        std::ostringstream name;
        name << "bim_p1_t006_vs1_e2e_" << ticks << "_"
             << counter.fetch_add(1, std::memory_order_relaxed) << ".db";
        return std::filesystem::temp_directory_path() / name.str();
    }
    void RemoveFiles() const {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
        ec.clear();
        std::filesystem::remove(std::filesystem::path(path_).concat("-wal"), ec);
        ec.clear();
        std::filesystem::remove(std::filesystem::path(path_).concat("-shm"), ec);
    }

    std::filesystem::path path_;
};

[[nodiscard]] ElementId NewId() {
    const std::optional<ElementId> id = bim::model::GenerateElementId();
    REQUIRE(id.has_value());
    return *id;
}

void RequireOk(const bim::commands::CommandResult result) {
    INFO("command code " << static_cast<int>(result.code));
    REQUIRE(result.ok());
}

[[nodiscard]] std::uint64_t Bits(double v) {
    return std::bit_cast<std::uint64_t>(v);
}
[[nodiscard]] std::uint32_t Bits(float v) {
    return std::bit_cast<std::uint32_t>(v);
}

void RequireSameSpec(const LinearExtrusionSpec& a, const LinearExtrusionSpec& b) {
    REQUIRE(Bits(a.profile.origin.x) == Bits(b.profile.origin.x));
    REQUIRE(Bits(a.profile.origin.y) == Bits(b.profile.origin.y));
    REQUIRE(Bits(a.profile.origin.z) == Bits(b.profile.origin.z));
    REQUIRE(Bits(a.profile.u_axis.x) == Bits(b.profile.u_axis.x));
    REQUIRE(Bits(a.profile.u_axis.y) == Bits(b.profile.u_axis.y));
    REQUIRE(Bits(a.profile.u_axis.z) == Bits(b.profile.u_axis.z));
    REQUIRE(Bits(a.profile.v_axis.x) == Bits(b.profile.v_axis.x));
    REQUIRE(Bits(a.profile.v_axis.y) == Bits(b.profile.v_axis.y));
    REQUIRE(Bits(a.profile.v_axis.z) == Bits(b.profile.v_axis.z));
    REQUIRE(Bits(a.profile.size_u) == Bits(b.profile.size_u));
    REQUIRE(Bits(a.profile.size_v) == Bits(b.profile.size_v));
    REQUIRE(Bits(a.direction.x) == Bits(b.direction.x));
    REQUIRE(Bits(a.direction.y) == Bits(b.direction.y));
    REQUIRE(Bits(a.direction.z) == Bits(b.direction.z));
    REQUIRE(Bits(a.distance) == Bits(b.distance));
}

void RequireSameMesh(const RenderMeshData& a, const RenderMeshData& b) {
    REQUIRE(a.topology == b.topology);
    REQUIRE(a.indices == b.indices);
    REQUIRE(a.positions.size() == b.positions.size());
    REQUIRE(a.normals.size() == b.normals.size());
    for (std::size_t i = 0; i < a.positions.size(); ++i) {
        REQUIRE(Bits(a.positions[i].x) == Bits(b.positions[i].x));
        REQUIRE(Bits(a.positions[i].y) == Bits(b.positions[i].y));
        REQUIRE(Bits(a.positions[i].z) == Bits(b.positions[i].z));
        REQUIRE(Bits(a.normals[i].x) == Bits(b.normals[i].x));
        REQUIRE(Bits(a.normals[i].y) == Bits(b.normals[i].y));
        REQUIRE(Bits(a.normals[i].z) == Bits(b.normals[i].z));
    }
}

// An axis-aligned wall on the plan line y = 0 (walls in this test run along +X
// from x0 to x1, so the document derives U = +X, V = +Y, W = +Z and the profile
// origin sits at y = -thickness / 2).
struct Box {
    double x0 = 0.0;
    double x1 = 0.0;
    double y0 = 0.0;
    double y1 = 0.0;
    double z0 = 0.0;
    double z1 = 0.0;
};

[[nodiscard]] Box BoxOf(double x0, double x1, double thickness, double z0, double z1) {
    return Box{x0, x1, -thickness / 2.0, thickness / 2.0, z0, z1};
}

void RequireSpecIsBox(const LinearExtrusionSpec& spec, const Box& box) {
    REQUIRE(std::fabs(spec.profile.origin.x - box.x0) < kEps);
    REQUIRE(std::fabs(spec.profile.origin.y - box.y0) < kEps);
    REQUIRE(std::fabs(spec.profile.origin.z - box.z0) < kEps);
    REQUIRE(spec.profile.u_axis.x == 1.0);
    REQUIRE(spec.profile.u_axis.y == 0.0);
    REQUIRE(spec.profile.v_axis.x == 0.0);
    REQUIRE(spec.profile.v_axis.y == 1.0);
    REQUIRE(spec.direction.z == 1.0);
    REQUIRE(std::fabs(spec.profile.size_u - (box.x1 - box.x0)) < kEps);
    REQUIRE(std::fabs(spec.profile.size_v - (box.y1 - box.y0)) < kEps);
    REQUIRE(std::fabs(spec.distance - (box.z1 - box.z0)) < kEps);
}

// A real-kernel observation of `spec`.
[[nodiscard]] FaceObservationResult ObserveRealSolid(const LinearExtrusionSpec& spec) {
    const SolidResult solid = bim::geometry_api::MakeLinearExtrusion(spec, kBuildTolerance);
    REQUIRE(solid.ok());
    FaceObservationResult observed = bim::geometry_api::ObserveFaces(solid.solid);
    REQUIRE(observed.ok());
    return observed;
}

// The observed face the resolver picked must lie on the semantic plane of `box`.
void RequireFaceIsSemantic(WallFaceRole role, const bim::geometry_api::FaceObservation& face,
                           const Box& box) {
    REQUIRE(face.surface_kind == SurfaceKind::Planar);
    double axis_component = 0.0; // |observed normal . expected axis|
    double offset = 0.0;         // signed distance of the face point from the expected plane
    switch (role) {
        case WallFaceRole::Bottom:
            axis_component = std::fabs(face.normal.z);
            offset = face.point_on_surface.z - box.z0;
            break;
        case WallFaceRole::Top:
            axis_component = std::fabs(face.normal.z);
            offset = face.point_on_surface.z - box.z1;
            break;
        case WallFaceRole::Start:
            axis_component = std::fabs(face.normal.x);
            offset = face.point_on_surface.x - box.x0;
            break;
        case WallFaceRole::End:
            axis_component = std::fabs(face.normal.x);
            offset = face.point_on_surface.x - box.x1;
            break;
        case WallFaceRole::SideA:
            axis_component = std::fabs(face.normal.y);
            offset = face.point_on_surface.y - box.y0;
            break;
        case WallFaceRole::SideB:
            axis_component = std::fabs(face.normal.y);
            offset = face.point_on_surface.y - box.y1;
            break;
    }
    REQUIRE(std::fabs(axis_component - 1.0) < 1.0e-9);
    REQUIRE(std::fabs(offset) < 1.0e-9);
}

// Resolves all six production references of `wall_id` against a real-kernel
// observation of `spec`, requires every one to resolve to the semantic plane of
// `box`, and requires six distinct faces.
void RequireAllSixResolve(const ElementId& wall_id, const LinearExtrusionSpec& spec,
                          const Box& box) {
    const FaceObservationResult observed = ObserveRealSolid(spec);
    REQUIRE(observed.faces.size() == 6);
    std::array<bool, 6> used{};
    for (const WallFaceRole role : kAllRoles) {
        const WallFaceReference reference{wall_id, role};
        REQUIRE(bim::model::ValidateWallFaceReference(reference).ok());
        const WallFaceResolutionResult result = bim::model::ResolveWallFaceReference(
            reference, WallFaceFrame{wall_id, spec}, observed, kResolveTolerance);
        INFO("role " << static_cast<int>(role));
        REQUIRE(result.status == WallFaceResolutionStatus::Resolved);
        REQUIRE(result.resolved_face_index < observed.faces.size());
        REQUIRE_FALSE(used[result.resolved_face_index]);
        used[result.resolved_face_index] = true;
        RequireFaceIsSemantic(role, observed.faces[result.resolved_face_index], box);
    }
}

[[nodiscard]] RenderMeshData BuildMeshOf(const LinearExtrusionSpec& spec) {
    bim::presentation::WallMeshResult built = BuildWallMesh(spec, kBuildTolerance);
    REQUIRE(built.ok());
    REQUIRE(bim::viewport::ValidateMesh(built.mesh).IsOk());
    return std::move(built.mesh);
}

[[nodiscard]] RendererCreateInfo HeadlessInfo() {
    RendererCreateInfo info;
    info.headless = true;
    info.widthPx = 640;
    info.heightPx = 480;
    return info;
}

// Everything observable about the committed state of a document.
struct Snapshot {
    std::vector<Level> levels;
    std::vector<StraightWall> walls;
    std::vector<std::optional<LinearExtrusionSpec>> geometry;
};

[[nodiscard]] Snapshot Capture(const Document& document) {
    Snapshot s;
    s.levels = bim::query::ListLevels(document);
    s.walls = bim::query::ListStraightWalls(document);
    for (const StraightWall& wall : s.walls) {
        s.geometry.push_back(document.FindWallGeometry(wall.id));
    }
    return s;
}

void RequireSameSnapshot(const Snapshot& a, const Snapshot& b) {
    REQUIRE(a.levels == b.levels);
    REQUIRE(a.walls == b.walls);
    REQUIRE(a.geometry.size() == b.geometry.size());
    for (std::size_t i = 0; i < a.geometry.size(); ++i) {
        REQUIRE(a.geometry[i].has_value());
        REQUIRE(b.geometry[i].has_value());
        RequireSameSpec(*a.geometry[i], *b.geometry[i]);
    }
}

[[nodiscard]] StraightWall MakeWall(const ElementId& id, const ElementId& level, Point2D start,
                                    Point2D end, double thickness, double height,
                                    double base_offset) {
    return StraightWall{.id = id,
                        .level_id = level,
                        .start = start,
                        .end = end,
                        .thickness = thickness,
                        .height = height,
                        .base_offset = base_offset};
}

} // namespace

TEST_CASE("VS1 end to end: commands, derived geometry, face references, save/reopen and the "
          "viewport path",
          "[p1_t006][e2e]") {
    Document document;

    // --- 1. create a Level through bim::commands ----------------------------------
    const ElementId level_id = NewId();
    RequireOk(bim::commands::CreateLevel(document, Level{.id = level_id, .elevation = 0.0}));

    // --- 2. create a StraightWall through bim::commands ---------------------------
    const ElementId wall_id = NewId();
    RequireOk(bim::commands::CreateStraightWall(
        document, MakeWall(wall_id, level_id, {0.0, 0.0}, {6.0, 0.0}, 0.2, 3.0, 0.0)));

    // --- 3. durable ElementIds -------------------------------------------------------
    REQUIRE(bim::model::IsValid(level_id));
    REQUIRE(bim::model::IsValid(wall_id));
    REQUIRE(level_id != wall_id);
    REQUIRE(bim::model::ParseElementId(bim::model::ToString(wall_id)) == wall_id);
    REQUIRE(bim::query::FindLevel(document, level_id).has_value());
    REQUIRE(bim::query::FindLevel(document, level_id)->id == level_id);
    REQUIRE(bim::query::FindStraightWall(document, wall_id).has_value());
    REQUIRE(bim::query::FindStraightWall(document, wall_id)->id == wall_id);
    REQUIRE(bim::query::FindStraightWall(document, wall_id)->level_id == level_id);

    // --- 4. Level -> StraightWall dependency behaviour -------------------------------
    {
        const auto refused = bim::commands::DeleteElement(document, level_id);
        REQUIRE(refused.code == bim::commands::CommandResultCode::ElementHasDependents);
        REQUIRE(bim::query::FindLevel(document, level_id).has_value());
        const auto orphan = bim::commands::CreateStraightWall(
            document, MakeWall(NewId(), NewId(), {0.0, 0.0}, {1.0, 0.0}, 0.2, 3.0, 0.0));
        REQUIRE(orphan.code == bim::commands::CommandResultCode::LevelNotFound);
    }

    // --- 5. derived wall geometry ----------------------------------------------------
    const auto spec_initial = document.FindWallGeometry(wall_id);
    REQUIRE(spec_initial.has_value());
    RequireSpecIsBox(*spec_initial, BoxOf(0.0, 6.0, 0.2, 0.0, 3.0));

    // --- 6. all six production WallFaceReference values -------------------------------
    std::array<WallFaceReference, 6> references;
    for (std::size_t i = 0; i < kAllRoles.size(); ++i) {
        references[i] = WallFaceReference{wall_id, kAllRoles[i]};
        REQUIRE(bim::model::ValidateWallFaceReference(references[i]).ok());
    }

    // --- 7-9. real geometry, real neutral observation, resolve all six -------------------
    const SolidResult solid_initial =
        bim::geometry_api::MakeLinearExtrusion(*spec_initial, kBuildTolerance);
    REQUIRE(solid_initial.ok());
    const FaceObservationResult observed_initial =
        bim::geometry_api::ObserveFaces(solid_initial.solid);
    REQUIRE(observed_initial.ok());
    REQUIRE(observed_initial.faces.size() == 6);
    for (const WallFaceReference& reference : references) {
        const WallFaceResolutionResult resolved = bim::model::ResolveWallFaceReference(
            reference, WallFaceFrame{wall_id, *spec_initial}, observed_initial, kResolveTolerance);
        REQUIRE(resolved.status == WallFaceResolutionStatus::Resolved);
        RequireFaceIsSemantic(reference.role, observed_initial.faces[resolved.resolved_face_index],
                              BoxOf(0.0, 6.0, 0.2, 0.0, 3.0));
    }
    // A reference to a different wall is refused as a foreign owner.
    REQUIRE(bim::model::ResolveWallFaceReference(WallFaceReference{NewId(), WallFaceRole::Top},
                                                 WallFaceFrame{wall_id, *spec_initial},
                                                 observed_initial, kResolveTolerance)
                .status == WallFaceResolutionStatus::InvalidOwner);

    // --- 10. change the wall geometry through a command -----------------------------------
    RequireOk(bim::commands::ChangeStraightWallGeometry(document, wall_id, {0.0, 0.0}, {8.0, 0.0},
                                                        0.3, 0.0, 2.5));
    const Box box_changed = BoxOf(0.0, 8.0, 0.3, 0.0, 2.5);

    // --- 11. deterministic recompute -----------------------------------------------------------
    const auto spec_changed = document.FindWallGeometry(wall_id);
    REQUIRE(spec_changed.has_value());
    RequireSpecIsBox(*spec_changed, box_changed);
    {
        // Re-applying the same values changes nothing, bit for bit...
        RequireOk(bim::commands::ChangeStraightWallGeometry(document, wall_id, {0.0, 0.0},
                                                            {8.0, 0.0}, 0.3, 0.0, 2.5));
        RequireSameSpec(*spec_changed, *document.FindWallGeometry(wall_id));
        // ...and an independently built document with the same final values derives
        // the identical specification.
        Document independent;
        RequireOk(bim::commands::CreateLevel(independent, Level{.id = level_id, .elevation = 0.0}));
        RequireOk(bim::commands::CreateStraightWall(
            independent, MakeWall(wall_id, level_id, {0.0, 0.0}, {8.0, 0.0}, 0.3, 2.5, 0.0)));
        RequireSameSpec(*spec_changed, *independent.FindWallGeometry(wall_id));
    }

    // --- 12. re-resolve the same references against the regenerated geometry ------------------
    RequireAllSixResolve(wall_id, *spec_changed, box_changed);
    {
        // Stale observations of the previous geometry are judged against the new
        // frame: every plane that moved is Missing, never silently matched.
        for (const WallFaceRole moved :
             {WallFaceRole::Top, WallFaceRole::End, WallFaceRole::SideA, WallFaceRole::SideB}) {
            REQUIRE(bim::model::ResolveWallFaceReference(WallFaceReference{wall_id, moved},
                                                         WallFaceFrame{wall_id, *spec_changed},
                                                         observed_initial, kResolveTolerance)
                        .status == WallFaceResolutionStatus::Missing);
        }
    }

    // --- 13. change the Level elevation through a command ---------------------------------------
    RequireOk(bim::commands::ChangeLevelElevation(document, level_id, 1.5));

    // --- 14. the wall's derived geometry moved with the Level -----------------------------------
    const auto spec_raised = document.FindWallGeometry(wall_id);
    REQUIRE(spec_raised.has_value());
    const Box box_raised = BoxOf(0.0, 8.0, 0.3, 1.5, 4.0);
    RequireSpecIsBox(*spec_raised, box_raised);
    REQUIRE(Bits(spec_raised->profile.origin.x) == Bits(spec_changed->profile.origin.x));
    REQUIRE(Bits(spec_raised->profile.origin.y) == Bits(spec_changed->profile.origin.y));
    RequireAllSixResolve(wall_id, *spec_raised, box_raised);
    REQUIRE(bim::model::ResolveWallFaceReference(WallFaceReference{wall_id, WallFaceRole::Bottom},
                                                 WallFaceFrame{wall_id, *spec_raised},
                                                 ObserveRealSolid(*spec_changed), kResolveTolerance)
                .status == WallFaceResolutionStatus::Missing);

    // A second wall on the same Level, so that listing order, dependency and the
    // reopened document carry more than one hosted element.
    const ElementId wall2_id = NewId();
    RequireOk(bim::commands::CreateStraightWall(
        document, MakeWall(wall2_id, level_id, {0.0, 4.0}, {8.0, 4.0}, 0.2, 3.0, 0.25)));
    const auto spec_wall2 = document.FindWallGeometry(wall2_id);
    REQUIRE(spec_wall2.has_value());
    RequireSpecIsBox(*spec_wall2, Box{0.0, 8.0, 3.9, 4.1, 1.75, 4.75});

    // --- 15. an invalid mutation leaves no partial committed state
    // --------------------------------
    {
        const Snapshot before = Capture(document);
        const double nan = std::numeric_limits<double>::quiet_NaN();
        const ElementId unknown = NewId();
        REQUIRE_FALSE(bim::commands::ChangeStraightWallGeometry(document, wall_id, {0.0, 0.0},
                                                                {8.0, 0.0}, nan, 0.0, 2.5)
                          .ok());
        REQUIRE_FALSE(bim::commands::ChangeStraightWallGeometry(document, wall_id, {1.0, 1.0},
                                                                {1.0, 1.0}, 0.3, 0.0, 2.5)
                          .ok());
        REQUIRE_FALSE(bim::commands::ChangeStraightWallGeometry(document, wall_id, {0.0, 0.0},
                                                                {8.0, 0.0}, 0.3, 0.0, -1.0)
                          .ok());
        REQUIRE_FALSE(bim::commands::ChangeStraightWallGeometry(document, unknown, {0.0, 0.0},
                                                                {8.0, 0.0}, 0.3, 0.0, 2.5)
                          .ok());
        REQUIRE_FALSE(bim::commands::ChangeLevelElevation(document, level_id, nan).ok());
        REQUIRE_FALSE(
            bim::commands::ChangeLevelElevation(document, wall_id, 2.0).ok()); // kind mismatch
        REQUIRE_FALSE(
            bim::commands::CreateLevel(document, Level{.id = level_id, .elevation = 9.0}).ok());
        REQUIRE_FALSE(
            bim::commands::CreateLevel(document, Level{.id = ElementId{}, .elevation = 0.0}).ok());
        REQUIRE_FALSE(
            bim::commands::CreateStraightWall(
                document, MakeWall(NewId(), level_id, {0.0, 0.0}, {1.0, 0.0}, 0.0, 3.0, 0.0))
                .ok());
        REQUIRE_FALSE(
            bim::commands::CreateStraightWall(
                document, MakeWall(wall_id, level_id, {0.0, 0.0}, {1.0, 0.0}, 0.2, 3.0, 0.0))
                .ok());                                                       // duplicate id
        REQUIRE_FALSE(bim::commands::DeleteElement(document, level_id).ok()); // has dependents
        REQUIRE_FALSE(bim::commands::DeleteElement(document, unknown).ok());
        RequireSameSnapshot(before, Capture(document));
    }

    // --- 16. queries read committed state
    // ---------------------------------------------------------------
    {
        const std::vector<Level> levels = bim::query::ListLevels(document);
        const std::vector<StraightWall> walls = bim::query::ListStraightWalls(document);
        REQUIRE(levels.size() == 1);
        REQUIRE(levels[0] == Level{.id = level_id, .elevation = 1.5});
        REQUIRE(walls.size() == 2);
        REQUIRE(std::is_sorted(
            walls.begin(), walls.end(),
            [](const StraightWall& a, const StraightWall& b) { return a.id < b.id; }));
        const auto found = bim::query::FindStraightWall(document, wall_id);
        REQUIRE(found.has_value());
        REQUIRE(*found == MakeWall(wall_id, level_id, {0.0, 0.0}, {8.0, 0.0}, 0.3, 2.5, 0.0));
        REQUIRE_FALSE(
            bim::query::FindStraightWall(document, level_id).has_value()); // a Level is not a wall
        REQUIRE_FALSE(bim::query::FindLevel(document, wall_id).has_value());
        REQUIRE_FALSE(bim::query::FindLevel(document, NewId()).has_value());
    }

    const Snapshot saved_state = Capture(document);
    const RenderMeshData mesh_wall1_before_save = BuildMeshOf(*spec_raised);
    const RenderMeshData mesh_wall2_before_save = BuildMeshOf(*spec_wall2);

    // --- 17. Save the document (schema v2)
    // --------------------------------------------------------------------
    const TempDatabasePath database;
    REQUIRE(document.Save(database.path()).ok());

    // --- 18. Open into a new Document
    // -------------------------------------------------------------------------------
    Document reopened;
    REQUIRE(Document::Open(database.path(), reopened).ok());

    // --- 19-20. the same durable ElementIds and the same BIM parameters
    // ---------------------------------------------
    {
        const std::vector<Level> levels = bim::query::ListLevels(reopened);
        const std::vector<StraightWall> walls = bim::query::ListStraightWalls(reopened);
        REQUIRE(levels.size() == saved_state.levels.size());
        REQUIRE(walls.size() == saved_state.walls.size());
        for (std::size_t i = 0; i < levels.size(); ++i) {
            REQUIRE(levels[i].id == saved_state.levels[i].id); // durable identity survived
        }
        for (std::size_t i = 0; i < walls.size(); ++i) {
            REQUIRE(walls[i].id == saved_state.walls[i].id);
        }
        REQUIRE(levels == saved_state.levels); // BIM parameters, exactly
        REQUIRE(walls == saved_state.walls);
    }

    // --- 22. derived geometry regenerated (bit-identical to the original)
    // ----------------------------------------------
    {
        RequireSameSnapshot(saved_state, Capture(reopened));
        RequireSameSpec(*spec_raised, *reopened.FindWallGeometry(wall_id));
        RequireSameSpec(*spec_wall2, *reopened.FindWallGeometry(wall2_id));
    }

    // --- 23. semantic face references re-resolve after reopen
    // ------------------------------------------------------------
    {
        const auto spec1 = reopened.FindWallGeometry(wall_id);
        const auto spec2 = reopened.FindWallGeometry(wall2_id);
        REQUIRE(spec1.has_value());
        REQUIRE(spec2.has_value());
        RequireAllSixResolve(wall_id, *spec1, box_raised);
        RequireAllSixResolve(wall2_id, *spec2, Box{0.0, 8.0, 3.9, 4.1, 1.75, 4.75});
    }

    // --- 24. BuildWallMesh from the reopened derived geometry
    // --------------------------------------------------------------
    const RenderMeshData mesh_wall1 = BuildMeshOf(*reopened.FindWallGeometry(wall_id));
    const RenderMeshData mesh_wall2 = BuildMeshOf(*reopened.FindWallGeometry(wall2_id));
    REQUIRE(mesh_wall1.positions.size() == 24);
    REQUIRE(mesh_wall1.indices.size() == 36);
    RequireSameMesh(mesh_wall1, mesh_wall1_before_save); // regeneration is deterministic
    RequireSameMesh(mesh_wall2, mesh_wall2_before_save);

    // --- 25. consume the meshes through the real headless viewport/bgfx path
    // -----------------------------------------------------
    {
        Renderer renderer;
        REQUIRE(renderer.Initialize(HeadlessInfo()).IsOk());
        const auto created1 = renderer.CreateMesh(mesh_wall1);
        const auto created2 = renderer.CreateMesh(mesh_wall2);
        REQUIRE(created1.IsOk());
        REQUIRE(created2.IsOk());
        const RenderMeshHandle handle1 = created1.Value();
        const RenderMeshHandle handle2 = created2.Value();
        REQUIRE(handle1.IsValid());
        REQUIRE(handle2.IsValid());
        REQUIRE(renderer.DestroyMesh(handle1).IsOk());
        REQUIRE(renderer.DestroyMesh(handle2).IsOk());
        const auto stale = renderer.DestroyMesh(handle1);
        REQUIRE_FALSE(stale.IsOk());
        REQUIRE(stale.Code() == ViewportErrorCode::ResourceNotFound);
        REQUIRE(renderer.Shutdown().IsOk());
    }

    // --- 21. dependency behaviour is reconstructed after reopen
    // ------------------------------------------------------------------ A Level change reaches
    // every hosted wall of the REOPENED document, so the Level -> StraightWall dependencies were
    // rebuilt; a Level that still hosts walls is still refused.
    {
        Document second;
        REQUIRE(Document::Open(database.path(), second).ok());
        RequireOk(bim::commands::ChangeLevelElevation(second, level_id, 4.0));
        REQUIRE(second.FindWallGeometry(wall_id)->profile.origin.z == 4.0); // 4.0 + base_offset 0
        REQUIRE(second.FindWallGeometry(wall2_id)->profile.origin.z ==
                4.25); // 4.0 + base_offset 0.25
        REQUIRE(bim::commands::DeleteElement(second, level_id).code ==
                bim::commands::CommandResultCode::ElementHasDependents);
        RequireOk(bim::commands::DeleteElement(second, wall_id));
        REQUIRE(bim::commands::DeleteElement(second, level_id).code ==
                bim::commands::CommandResultCode::ElementHasDependents); // wall2 still hosted
        RequireOk(bim::commands::DeleteElement(second, wall2_id));
        RequireOk(bim::commands::DeleteElement(second, level_id)); // now free
        REQUIRE(bim::query::ListLevels(second).empty());
        REQUIRE(bim::query::ListStraightWalls(second).empty());
        // The original reopened document was not affected.
        RequireSameSnapshot(saved_state, Capture(reopened));
    }
}

TEST_CASE("VS1 workflow drives the same slice through bim::commands, bim::query and the bridge",
          "[p1_t006][e2e][workflow]") {
    bim::desktop::Vs1Workflow workflow;

    // An empty model is a valid, empty scene.
    {
        const bim::desktop::SceneMeshes empty_scene = workflow.RebuildWallMeshes();
        REQUIRE(empty_scene.result.ok());
        REQUIRE(empty_scene.meshes.empty());
        REQUIRE(workflow.ListLevels().empty());
        REQUIRE(workflow.ListWalls().empty());
    }

    const bim::desktop::CreatedElement level = workflow.CreateLevel(0.0);
    REQUIRE(level.result.ok());
    REQUIRE(bim::model::IsValid(level.id));
    const bim::desktop::CreatedElement wall1 =
        workflow.CreateStraightWall(level.id, {0.0, 0.0}, {6.0, 0.0}, 0.2, 3.0, 0.0);
    REQUIRE(wall1.result.ok());
    const bim::desktop::CreatedElement wall2 =
        workflow.CreateStraightWall(level.id, {0.0, 4.0}, {6.0, 4.0}, 0.2, 3.0, 0.0);
    REQUIRE(wall2.result.ok());
    REQUIRE(wall1.id != wall2.id);
    REQUIRE(workflow.ListLevels().size() == 1);
    REQUIRE(workflow.ListWalls().size() == 2);
    REQUIRE(workflow.FindLevel(level.id).has_value());
    REQUIRE(workflow.FindWall(wall1.id).has_value());

    // The complete scene: one mesh per committed wall, in query (ElementId) order.
    const bim::desktop::SceneMeshes scene = workflow.RebuildWallMeshes();
    REQUIRE(scene.result.ok());
    REQUIRE(scene.meshes.size() == 2);
    for (const RenderMeshData& mesh : scene.meshes) {
        REQUIRE(bim::viewport::ValidateMesh(mesh).IsOk());
        REQUIRE(mesh.positions.size() == 24);
        REQUIRE(mesh.indices.size() == 36);
    }
    // ... and it is exactly the bridge applied to the committed derived geometry.
    {
        Document reference;
        REQUIRE(bim::commands::CreateLevel(reference, *workflow.FindLevel(level.id)).ok());
        for (const StraightWall& wall : workflow.ListWalls()) {
            REQUIRE(bim::commands::CreateStraightWall(reference, wall).ok());
        }
        const std::vector<StraightWall> walls = bim::query::ListStraightWalls(reference);
        for (std::size_t i = 0; i < walls.size(); ++i) {
            RequireSameMesh(scene.meshes[i], BuildMeshOf(*reference.FindWallGeometry(walls[i].id)));
        }
    }

    // A Level edit moves every hosted wall; a wall edit changes only that wall.
    REQUIRE(workflow.ChangeLevelElevation(level.id, 2.0).ok());
    const bim::desktop::SceneMeshes raised = workflow.RebuildWallMeshes();
    REQUIRE(raised.result.ok());
    REQUIRE(raised.meshes.size() == 2);
    for (std::size_t m = 0; m < 2; ++m) {
        for (std::size_t i = 0; i < raised.meshes[m].positions.size(); ++i) {
            REQUIRE(std::fabs(raised.meshes[m].positions[i].z - scene.meshes[m].positions[i].z -
                              2.0f) < 1.0e-5f);
            REQUIRE(Bits(raised.meshes[m].positions[i].x) == Bits(scene.meshes[m].positions[i].x));
            REQUIRE(Bits(raised.meshes[m].positions[i].y) == Bits(scene.meshes[m].positions[i].y));
        }
    }
    REQUIRE(workflow.ChangeWallGeometry(wall1.id, {0.0, 0.0}, {9.0, 0.0}, 0.4, 0.0, 2.5).ok());
    const std::optional<StraightWall> edited = workflow.FindWall(wall1.id);
    REQUIRE(edited.has_value());
    REQUIRE(edited->end.x == 9.0);
    REQUIRE(edited->level_id == level.id); // hosting Level preserved
    const bim::desktop::SceneMeshes edited_scene = workflow.RebuildWallMeshes();
    REQUIRE(edited_scene.result.ok());
    REQUIRE(edited_scene.meshes.size() == 2);

    // Commands are atomic and report their refusal through the workflow result.
    {
        const std::vector<StraightWall> walls_before = workflow.ListWalls();
        const bim::desktop::WorkflowResult nan_edit = workflow.ChangeWallGeometry(
            wall1.id, {0.0, 0.0}, {9.0, 0.0}, std::numeric_limits<double>::quiet_NaN(), 0.0, 2.5);
        REQUIRE_FALSE(nan_edit.ok());
        REQUIRE(nan_edit.code == bim::desktop::WorkflowCode::CommandRejected);
        REQUIRE(nan_edit.command.code == bim::commands::CommandResultCode::InvalidElement);
        REQUIRE(workflow.ListWalls() == walls_before);
        const bim::desktop::WorkflowResult hosted = workflow.RemoveElement(level.id);
        REQUIRE(hosted.code == bim::desktop::WorkflowCode::CommandRejected);
        REQUIRE(hosted.command.code == bim::commands::CommandResultCode::ElementHasDependents);
        const bim::desktop::CreatedElement orphan =
            workflow.CreateStraightWall(NewId(), {0.0, 0.0}, {1.0, 0.0}, 0.2, 3.0, 0.0);
        REQUIRE_FALSE(orphan.result.ok());
        REQUIRE(workflow.ListWalls() == walls_before);
    }

    // Save, then Open into a fresh workflow: same identities, parameters and scene.
    const TempDatabasePath database;
    REQUIRE(workflow.Save(database.path()).ok());
    bim::desktop::Vs1Workflow reopened;
    REQUIRE(reopened.Open(database.path()).ok());
    REQUIRE(reopened.ListLevels() == workflow.ListLevels());
    REQUIRE(reopened.ListWalls() == workflow.ListWalls());
    const bim::desktop::SceneMeshes reopened_scene = reopened.RebuildWallMeshes();
    REQUIRE(reopened_scene.result.ok());
    REQUIRE(reopened_scene.meshes.size() == edited_scene.meshes.size());
    for (std::size_t m = 0; m < reopened_scene.meshes.size(); ++m) {
        RequireSameMesh(reopened_scene.meshes[m], edited_scene.meshes[m]);
    }

    // The reopened scene is consumable by the headless renderer.
    {
        Renderer renderer;
        REQUIRE(renderer.Initialize(HeadlessInfo()).IsOk());
        std::vector<RenderMeshHandle> handles;
        for (const RenderMeshData& mesh : reopened_scene.meshes) {
            const auto created = renderer.CreateMesh(mesh);
            REQUIRE(created.IsOk());
            handles.push_back(created.Value());
        }
        for (const RenderMeshHandle& handle : handles) {
            REQUIRE(renderer.DestroyMesh(handle).IsOk());
        }
        REQUIRE(renderer.Shutdown().IsOk());
    }

    // A failed Open leaves the live workflow exactly as it was.
    {
        const std::vector<Level> levels_before = reopened.ListLevels();
        const std::vector<StraightWall> walls_before = reopened.ListWalls();
        const TempDatabasePath missing; // never created
        const bim::desktop::WorkflowResult failed = reopened.Open(missing.path());
        REQUIRE_FALSE(failed.ok());
        REQUIRE(failed.code == bim::desktop::WorkflowCode::OpenFailed);
        REQUIRE(reopened.ListLevels() == levels_before);
        REQUIRE(reopened.ListWalls() == walls_before);
        const bim::desktop::SceneMeshes still = reopened.RebuildWallMeshes();
        REQUIRE(still.result.ok());
        REQUIRE(still.meshes.size() == walls_before.size());
    }

    // Removing both walls and then the Level empties the model.
    REQUIRE(workflow.RemoveElement(wall1.id).ok());
    REQUIRE(workflow.RemoveElement(wall2.id).ok());
    REQUIRE(workflow.RemoveElement(level.id).ok());
    REQUIRE(workflow.ListLevels().empty());
    REQUIRE(workflow.ListWalls().empty());
    REQUIRE(workflow.RebuildWallMeshes().meshes.empty());
}

TEST_CASE("VS1 workflow fails closed: identity failure and an unrenderable wall",
          "[p1_t006][e2e][workflow]") {
    // Identity generation failure: nothing is created, nothing is mutated.
    {
        bim::desktop::Vs1Workflow failing(
            []() noexcept -> std::optional<ElementId> { return std::nullopt; });
        const bim::desktop::CreatedElement level = failing.CreateLevel(0.0);
        REQUIRE_FALSE(level.result.ok());
        REQUIRE(level.result.code == bim::desktop::WorkflowCode::IdentityGenerationFailed);
        REQUIRE(failing.ListLevels().empty());
        const bim::desktop::CreatedElement wall =
            failing.CreateStraightWall(NewId(), {0.0, 0.0}, {1.0, 0.0}, 0.2, 3.0, 0.0);
        REQUIRE(wall.result.code == bim::desktop::WorkflowCode::IdentityGenerationFailed);
        REQUIRE(failing.ListWalls().empty());
    }
    // An all-zero generated identity is as unusable as no identity.
    {
        bim::desktop::Vs1Workflow zero(
            []() noexcept -> std::optional<ElementId> { return ElementId{}; });
        REQUIRE(zero.CreateLevel(0.0).result.code ==
                bim::desktop::WorkflowCode::IdentityGenerationFailed);
        REQUIRE(zero.ListLevels().empty());
    }

    // A committed wall whose geometry cannot be represented as a float32 render
    // mesh makes the whole rebuild fail: no partially rebuilt scene is returned.
    {
        bim::desktop::Vs1Workflow workflow;
        const auto level = workflow.CreateLevel(0.0);
        REQUIRE(level.result.ok());
        REQUIRE(workflow.CreateStraightWall(level.id, {0.0, 0.0}, {6.0, 0.0}, 0.2, 3.0, 0.0)
                    .result.ok());
        REQUIRE(workflow.RebuildWallMeshes().meshes.size() == 1);
        const auto huge =
            workflow.CreateStraightWall(level.id, {0.0, 8.0}, {1.0e39, 8.0}, 0.2, 3.0, 0.0);
        REQUIRE(huge.result.ok()); // finite, so the document accepts it
        const bim::desktop::SceneMeshes scene = workflow.RebuildWallMeshes();
        REQUIRE_FALSE(scene.result.ok());
        REQUIRE(scene.result.code == bim::desktop::WorkflowCode::RenderRebuildFailed);
        REQUIRE(scene.meshes.empty());
        // The committed model itself is untouched by the failed rebuild.
        REQUIRE(workflow.ListWalls().size() == 2);
    }
}
