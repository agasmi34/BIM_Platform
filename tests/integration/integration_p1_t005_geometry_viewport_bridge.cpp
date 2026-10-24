// P1-T005 Geometry Tessellation & Viewport Bridge - end-to-end bridge proof
// (Architecture Gate BIM-AG-P1-T005; Implementation Brief P1-T005).
//
// A real Level and StraightWall are authored through the public Document API; the
// committed derived LinearExtrusionSpec is read back with Document::FindWallGeometry;
// bim::presentation turns it into a RenderMeshData; and that same mesh goes through the
// existing headless bim::viewport_bgfx::Renderer CreateMesh / DestroyMesh lifecycle.
// A later committed edit (wall or Level) produces a new derived specification and with
// it the expected, deterministic replacement mesh.
//
// This target links document + presentation + viewport + viewport_bgfx for TEST
// COMPOSITION ONLY; the production dependency direction (presentation depends on
// geometry_api and viewport alone) is unchanged and is enforced by rule R23.
#include "bim/document/document.hpp"
#include "bim/geometry_api/geometry.hpp"
#include "bim/model/element_id.hpp"
#include "bim/model/level.hpp"
#include "bim/model/straight_wall.hpp"
#include "bim/presentation/wall_mesh.hpp"
#include "bim/viewport/error.hpp"
#include "bim/viewport/mesh.hpp"
#include "bim/viewport_bgfx/renderer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace {

using bim::document::Document;
using bim::document::DocumentResult;
using bim::geometry_api::GeometryTolerance;
using bim::geometry_api::LinearExtrusionSpec;
using bim::model::ElementId;
using bim::model::Level;
using bim::model::Point2D;
using bim::model::StraightWall;
using bim::presentation::BuildWallMesh;
using bim::presentation::WallMeshErrorCode;
using bim::presentation::WallMeshResult;
using bim::viewport::RenderMeshData;
using bim::viewport::ViewportErrorCode;
using bim::viewport_bgfx::Renderer;
using bim::viewport_bgfx::RendererCreateInfo;
using bim::viewport_bgfx::RenderMeshHandle;

constexpr GeometryTolerance kTolerance{1.0e-9, 1.0e-9};
constexpr float kEps = 1.0e-5f;

ElementId Id(std::uint8_t first) {
    ElementId id;
    id.bytes[0] = first;
    id.bytes[15] = 0x01;
    return id;
}

StraightWall MakeWall(const ElementId& id, const ElementId& level, Point2D start, Point2D end,
                      double thickness, double height, double base_offset = 0.0) {
    return StraightWall{.id = id,
                        .level_id = level,
                        .start = start,
                        .end = end,
                        .thickness = thickness,
                        .height = height,
                        .base_offset = base_offset};
}

void RequireOk(const DocumentResult result) {
    INFO(bim::document::ToString(result.code));
    REQUIRE(result.ok());
}

RendererCreateInfo HeadlessInfo() {
    RendererCreateInfo info;
    info.headless = true;
    info.widthPx = 640;
    info.heightPx = 480;
    return info;
}

// The committed derived specification of a wall, turned into a mesh.
WallMeshResult MeshOf(const Document& document, const ElementId& wall) {
    const std::optional<LinearExtrusionSpec> spec = document.FindWallGeometry(wall);
    REQUIRE(spec.has_value());
    return BuildWallMesh(*spec, kTolerance);
}

void RequireSameBits(const RenderMeshData& a, const RenderMeshData& b) {
    REQUIRE(a.topology == b.topology);
    REQUIRE(a.indices == b.indices);
    REQUIRE(a.positions.size() == b.positions.size());
    REQUIRE(a.normals.size() == b.normals.size());
    for (std::size_t i = 0; i < a.positions.size(); ++i) {
        REQUIRE(std::bit_cast<std::uint32_t>(a.positions[i].x) ==
                std::bit_cast<std::uint32_t>(b.positions[i].x));
        REQUIRE(std::bit_cast<std::uint32_t>(a.positions[i].y) ==
                std::bit_cast<std::uint32_t>(b.positions[i].y));
        REQUIRE(std::bit_cast<std::uint32_t>(a.positions[i].z) ==
                std::bit_cast<std::uint32_t>(b.positions[i].z));
        REQUIRE(std::bit_cast<std::uint32_t>(a.normals[i].x) ==
                std::bit_cast<std::uint32_t>(b.normals[i].x));
        REQUIRE(std::bit_cast<std::uint32_t>(a.normals[i].y) ==
                std::bit_cast<std::uint32_t>(b.normals[i].y));
        REQUIRE(std::bit_cast<std::uint32_t>(a.normals[i].z) ==
                std::bit_cast<std::uint32_t>(b.normals[i].z));
    }
}

// A wall along +X from x0 to x1 on the plan line y = 0, thickness t, from z0 to z1: the
// document derives U=+X, V=+Y, W=+Z with the profile origin at y = -t/2. The mesh of such
// a wall is therefore an axis-aligned box [x0,x1] x [-t/2,t/2] x [z0,z1]; the eight
// corners, in the frozen face order, are checked against it.
void RequireBoxMesh(const RenderMeshData& mesh, float x0, float x1, float half_t, float z0,
                    float z1) {
    REQUIRE(mesh.positions.size() == 24);
    REQUIRE(mesh.normals.size() == 24);
    REQUIRE(mesh.indices.size() == 36);
    struct P {
        float x, y, z;
    };
    const P p0{x0, -half_t, z0}, p1{x1, -half_t, z0}, p2{x1, half_t, z0}, p3{x0, half_t, z0};
    const P q0{x0, -half_t, z1}, q1{x1, -half_t, z1}, q2{x1, half_t, z1}, q3{x0, half_t, z1};
    const std::array<std::array<P, 4>, 6> faces{{
        {p0, p3, p2, p1}, // Bottom
        {q0, q1, q2, q3}, // Top
        {p0, q0, q3, p3}, // Start
        {p1, p2, q2, q1}, // End
        {p0, p1, q1, q0}, // SideA
        {p3, q3, q2, p2}, // SideB
    }};
    for (std::size_t f = 0; f < 6; ++f) {
        for (std::size_t k = 0; k < 4; ++k) {
            INFO("face " << f << " vertex " << k);
            REQUIRE(std::fabs(mesh.positions[f * 4 + k].x - faces[f][k].x) < kEps);
            REQUIRE(std::fabs(mesh.positions[f * 4 + k].y - faces[f][k].y) < kEps);
            REQUIRE(std::fabs(mesh.positions[f * 4 + k].z - faces[f][k].z) < kEps);
        }
    }
}

} // namespace

TEST_CASE("a document wall becomes a valid mesh that the headless renderer accepts",
          "[p1_t005][bridge]") {
    Document document;
    const ElementId level = Id(0x10);
    const ElementId wall = Id(0x31);
    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    RequireOk(document.AddStraightWall(
        MakeWall(wall, level, Point2D{0.0, 0.0}, Point2D{6.0, 0.0}, 0.2, 3.0)));

    const WallMeshResult result = MeshOf(document, wall);
    INFO(static_cast<int>(result.code));
    REQUIRE(result.ok());
    REQUIRE(result.code == WallMeshErrorCode::None);
    REQUIRE(result.mesh.topology == bim::viewport::MeshTopology::TriangleList);
    REQUIRE(bim::viewport::ValidateMesh(result.mesh).IsOk());
    RequireBoxMesh(result.mesh, 0.0f, 6.0f, 0.1f, 0.0f, 3.0f);

    // The same derived specification always gives the same mesh, bit for bit.
    RequireSameBits(MeshOf(document, wall).mesh, result.mesh);

    // The very same mesh goes through the existing headless renderer lifecycle.
    Renderer renderer;
    REQUIRE(renderer.Initialize(HeadlessInfo()).IsOk());
    const auto created = renderer.CreateMesh(result.mesh);
    REQUIRE(created.IsOk());
    REQUIRE(created.Value().IsValid());
    REQUIRE(renderer.DestroyMesh(created.Value()).IsOk());
    // A released handle is not a live resource any more (existing renderer contract).
    const auto again = renderer.DestroyMesh(created.Value());
    REQUIRE_FALSE(again.IsOk());
    REQUIRE(again.Code() == ViewportErrorCode::ResourceNotFound);
    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("a committed wall edit produces the expected replacement mesh", "[p1_t005][bridge]") {
    Document document;
    const ElementId level = Id(0x10);
    const ElementId wall = Id(0x31);
    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    RequireOk(document.AddStraightWall(
        MakeWall(wall, level, Point2D{0.0, 0.0}, Point2D{6.0, 0.0}, 0.2, 3.0)));
    const WallMeshResult before = MeshOf(document, wall);
    REQUIRE(before.ok());

    Renderer renderer;
    REQUIRE(renderer.Initialize(HeadlessInfo()).IsOk());
    const auto first_handle = renderer.CreateMesh(before.mesh);
    REQUIRE(first_handle.IsOk());

    // Longer, thicker, taller wall.
    RequireOk(document.UpdateStraightWall(
        MakeWall(wall, level, Point2D{0.0, 0.0}, Point2D{8.0, 0.0}, 0.4, 2.5)));
    const WallMeshResult after = MeshOf(document, wall);
    REQUIRE(after.ok());
    REQUIRE(bim::viewport::ValidateMesh(after.mesh).IsOk());
    RequireBoxMesh(after.mesh, 0.0f, 8.0f, 0.2f, 0.0f, 2.5f);
    RequireSameBits(MeshOf(document, wall).mesh, after.mesh); // deterministic

    // The mesh that was built earlier is a value: the edit did not alter it.
    RequireBoxMesh(before.mesh, 0.0f, 6.0f, 0.1f, 0.0f, 3.0f);
    REQUIRE(before.mesh.positions[3].x != after.mesh.positions[3].x);

    // Replace the renderer resource: new mesh created, old one released.
    const auto second_handle = renderer.CreateMesh(after.mesh);
    REQUIRE(second_handle.IsOk());
    REQUIRE(renderer.DestroyMesh(first_handle.Value()).IsOk());
    REQUIRE(renderer.DestroyMesh(second_handle.Value()).IsOk());
    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("a committed Level edit moves every hosted wall mesh", "[p1_t005][bridge]") {
    Document document;
    const ElementId level = Id(0x10);
    const ElementId wall_a = Id(0x31);
    const ElementId wall_b = Id(0x32);
    RequireOk(document.AddLevel(Level{.id = level, .elevation = 1.0}));
    RequireOk(document.AddStraightWall(
        MakeWall(wall_a, level, Point2D{0.0, 0.0}, Point2D{4.0, 0.0}, 0.2, 3.0, 0.5)));
    RequireOk(document.AddStraightWall(
        MakeWall(wall_b, level, Point2D{10.0, 0.0}, Point2D{12.0, 0.0}, 0.3, 2.0)));

    const WallMeshResult a1 = MeshOf(document, wall_a);
    const WallMeshResult b1 = MeshOf(document, wall_b);
    REQUIRE(a1.ok());
    REQUIRE(b1.ok());
    RequireBoxMesh(a1.mesh, 0.0f, 4.0f, 0.1f, 1.5f, 4.5f); // 1.0 + 0.5 .. + 3.0
    RequireBoxMesh(b1.mesh, 10.0f, 12.0f, 0.15f, 1.0f, 3.0f);

    RequireOk(document.UpdateLevelElevation(level, 2.5));
    const WallMeshResult a2 = MeshOf(document, wall_a);
    const WallMeshResult b2 = MeshOf(document, wall_b);
    REQUIRE(a2.ok());
    REQUIRE(b2.ok());
    RequireBoxMesh(a2.mesh, 0.0f, 4.0f, 0.1f, 3.0f, 6.0f);
    RequireBoxMesh(b2.mesh, 10.0f, 12.0f, 0.15f, 2.5f, 4.5f);

    // Only z moved: plan coordinates and normals are untouched, byte for byte.
    for (std::size_t i = 0; i < 24; ++i) {
        REQUIRE(std::bit_cast<std::uint32_t>(a1.mesh.positions[i].x) ==
                std::bit_cast<std::uint32_t>(a2.mesh.positions[i].x));
        REQUIRE(std::bit_cast<std::uint32_t>(a1.mesh.positions[i].y) ==
                std::bit_cast<std::uint32_t>(a2.mesh.positions[i].y));
        REQUIRE(std::fabs((a2.mesh.positions[i].z - a1.mesh.positions[i].z) - 1.5f) < kEps);
        REQUIRE(std::bit_cast<std::uint32_t>(a1.mesh.normals[i].z) ==
                std::bit_cast<std::uint32_t>(a2.mesh.normals[i].z));
    }

    Renderer renderer;
    REQUIRE(renderer.Initialize(HeadlessInfo()).IsOk());
    const auto handle_a = renderer.CreateMesh(a2.mesh);
    const auto handle_b = renderer.CreateMesh(b2.mesh);
    REQUIRE(handle_a.IsOk());
    REQUIRE(handle_b.IsOk());
    REQUIRE(renderer.DestroyMesh(handle_a.Value()).IsOk());
    REQUIRE(renderer.DestroyMesh(handle_b.Value()).IsOk());
    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("an angled wall bridges with outward counter-clockwise faces", "[p1_t005][bridge]") {
    Document document;
    const ElementId level = Id(0x10);
    const ElementId wall = Id(0x31);
    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    // A 3-4-5 plan wall: U = (0.6, 0.8), V = (-0.8, 0.6).
    RequireOk(document.AddStraightWall(
        MakeWall(wall, level, Point2D{1.0, 1.0}, Point2D{4.0, 5.0}, 0.5, 2.0)));
    const WallMeshResult result = MeshOf(document, wall);
    REQUIRE(result.ok());
    REQUIRE(bim::viewport::ValidateMesh(result.mesh).IsOk());
    const RenderMeshData& mesh = result.mesh;

    // Centroid and per-triangle outwardness.
    float cx = 0.0f, cy = 0.0f, cz = 0.0f;
    for (const auto& p : mesh.positions) {
        cx += p.x / 24.0f;
        cy += p.y / 24.0f;
        cz += p.z / 24.0f;
    }
    for (std::size_t tri = 0; tri < 12; ++tri) {
        const auto& a = mesh.positions[mesh.indices[tri * 3 + 0]];
        const auto& b = mesh.positions[mesh.indices[tri * 3 + 1]];
        const auto& c = mesh.positions[mesh.indices[tri * 3 + 2]];
        const auto& n = mesh.normals[mesh.indices[tri * 3 + 0]];
        const float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
        const float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        const float gx = uy * vz - uz * vy, gy = uz * vx - ux * vz, gz = ux * vy - uy * vx;
        REQUIRE(gx * n.x + gy * n.y + gz * n.z > 0.0f);
        const float mx = (a.x + b.x + c.x) / 3.0f - cx;
        const float my = (a.y + b.y + c.y) / 3.0f - cy;
        const float mz = (a.z + b.z + c.z) / 3.0f - cz;
        REQUIRE(mx * n.x + my * n.y + mz * n.z > 0.0f);
    }
    // Start face normal is -U, End face normal is +U.
    REQUIRE(std::fabs(mesh.normals[8].x - (-0.6f)) < kEps);
    REQUIRE(std::fabs(mesh.normals[8].y - (-0.8f)) < kEps);
    REQUIRE(std::fabs(mesh.normals[12].x - 0.6f) < kEps);
    REQUIRE(std::fabs(mesh.normals[12].y - 0.8f) < kEps);

    Renderer renderer;
    REQUIRE(renderer.Initialize(HeadlessInfo()).IsOk());
    const auto handle = renderer.CreateMesh(mesh);
    REQUIRE(handle.IsOk());
    REQUIRE(renderer.DestroyMesh(handle.Value()).IsOk());
    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("no mesh exists for a wall the document does not hold", "[p1_t005][bridge]") {
    Document document;
    const ElementId level = Id(0x10);
    const ElementId wall = Id(0x31);
    RequireOk(document.AddLevel(Level{.id = level, .elevation = 0.0}));
    RequireOk(document.AddStraightWall(
        MakeWall(wall, level, Point2D{0.0, 0.0}, Point2D{6.0, 0.0}, 0.2, 3.0)));
    REQUIRE(document.FindWallGeometry(wall).has_value());
    RequireOk(document.DeleteElement(wall));
    // The derived specification is gone with the wall, so the bridge has nothing to
    // convert: there is no stale geometry to mesh.
    REQUIRE_FALSE(document.FindWallGeometry(wall).has_value());
    REQUIRE_FALSE(document.FindWallGeometry(Id(0x77)).has_value());
}
