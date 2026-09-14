#include <catch2/catch_test_macros.hpp>

#include "bim/viewport/mesh.hpp"

using bim::viewport::MeshTopology;
using bim::viewport::RenderMeshData;
using bim::viewport::ValidateMesh;
using bim::viewport::ViewportErrorCode;

namespace {

RenderMeshData MakeValidTriangle() {
    RenderMeshData mesh;
    mesh.topology = MeshTopology::TriangleList;
    mesh.positions = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    mesh.indices = {0, 1, 2};
    return mesh;
}

} // namespace

TEST_CASE("ValidateMesh accepts a well-formed triangle without normals", "[viewport][mesh]") {
    const RenderMeshData mesh = MakeValidTriangle();
    const auto status = ValidateMesh(mesh);
    REQUIRE(status.IsOk());
}

TEST_CASE("ValidateMesh accepts a well-formed triangle with matching normals", "[viewport][mesh]") {
    RenderMeshData mesh = MakeValidTriangle();
    mesh.normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}};
    const auto status = ValidateMesh(mesh);
    REQUIRE(status.IsOk());
}

TEST_CASE("ValidateMesh rejects an empty mesh", "[viewport][mesh]") {
    const RenderMeshData mesh;
    const auto status = ValidateMesh(mesh);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidMesh);
}

TEST_CASE("ValidateMesh rejects an index count that is not a multiple of three",
          "[viewport][mesh]") {
    RenderMeshData mesh = MakeValidTriangle();
    mesh.indices = {0, 1, 2, 0}; // 4 indices: not a multiple of 3
    const auto status = ValidateMesh(mesh);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidMesh);
}

TEST_CASE("ValidateMesh rejects a zero index count", "[viewport][mesh]") {
    RenderMeshData mesh = MakeValidTriangle();
    mesh.indices.clear();
    const auto status = ValidateMesh(mesh);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidMesh);
}

TEST_CASE("ValidateMesh rejects an out-of-range index", "[viewport][mesh]") {
    RenderMeshData mesh = MakeValidTriangle();
    mesh.indices = {0, 1, 3}; // only 3 positions exist (indices 0..2)
    const auto status = ValidateMesh(mesh);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidMesh);
}

TEST_CASE("ValidateMesh rejects a normal count that does not match the vertex count",
          "[viewport][mesh]") {
    RenderMeshData mesh = MakeValidTriangle();
    mesh.normals = {{0, 0, 1}}; // 1 normal for 3 positions
    const auto status = ValidateMesh(mesh);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::InvalidMesh);
}

TEST_CASE("ValidateMesh accepts a mesh with multiple triangles sharing vertices",
          "[viewport][mesh]") {
    RenderMeshData mesh;
    mesh.topology = MeshTopology::TriangleList;
    mesh.positions = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
    };
    mesh.indices = {0, 1, 2, 0, 2, 3};
    const auto status = ValidateMesh(mesh);
    REQUIRE(status.IsOk());
}

// AA Source Review Round 2 finding M09 regression coverage.
//
// spike_scene.cpp's MakeV04RepresentativeMediumMesh() builds a subdivided
// XY-plane grid with every vertex authored with a {0,0,1} normal, then
// triangulates each quad. Round 1 shipped that triangulation wound CW as
// seen from +Z - the opposite of its own authored normals and of
// AppendBox's documented "CCW when viewed from outside" convention (see
// spike_scene.cpp's AppendBox comment) - and was fixed this round.
//
// This test file deliberately links only bim::viewport (see this file's
// build target, tests/unit/CMakeLists.txt: no bgfx/Qt/D3D dependency is
// exercised here), so rather than widen that target's scope to also link
// spike_scene.cpp, this test mirrors V04's exact quad-triangulation
// algorithm generically (same corner-index derivation, same fixed index
// ordering) and independently verifies every triangle produced is CCW as
// seen from +Z, matching an outward-facing +Z normal. It duplicates rather
// than calls the production triangulation, so it cannot catch a future
// edit to spike_scene.cpp diverging from this pattern by itself - the
// intent is to pin the winding convention itself, not spike_scene.cpp's
// literal source.
namespace {

// 2D cross product of (b - a) x (c - a); positive means a->b->c turns
// counter-clockwise in the standard (right-handed, +Z-out-of-the-page) XY
// convention - i.e. CCW as seen from +Z looking down the -Z axis.
float Cross2D(const bim::viewport::Position3f& a, const bim::viewport::Position3f& b,
              const bim::viewport::Position3f& c) {
    const float abx = b.x - a.x;
    const float aby = b.y - a.y;
    const float acx = c.x - a.x;
    const float acy = c.y - a.y;
    return abx * acy - aby * acx;
}

// Mirrors spike_scene.cpp's MakeV04RepresentativeMediumMesh() triangulation
// exactly (post-M09-fix index ordering), at a small subdivision count for
// test speed. Every vertex gets a {0,0,1} normal, matching V04.
RenderMeshData MakeSubdividedGridMirroringV04(std::uint32_t subdivisions) {
    RenderMeshData mesh;
    mesh.topology = MeshTopology::TriangleList;

    const std::uint32_t verts_per_side = subdivisions + 1;
    constexpr float kHalfSize = 6.0f;
    const float step = (2.0f * kHalfSize) / static_cast<float>(subdivisions);

    for (std::uint32_t row = 0; row < verts_per_side; ++row) {
        for (std::uint32_t col = 0; col < verts_per_side; ++col) {
            const float x = -kHalfSize + static_cast<float>(col) * step;
            const float y = -kHalfSize + static_cast<float>(row) * step;
            mesh.positions.push_back({x, y, 0.0f});
            mesh.normals.push_back({0.0f, 0.0f, 1.0f});
        }
    }

    for (std::uint32_t row = 0; row < subdivisions; ++row) {
        for (std::uint32_t col = 0; col < subdivisions; ++col) {
            const std::uint32_t i0 = row * verts_per_side + col;
            const std::uint32_t i1 = i0 + 1;
            const std::uint32_t i2 = i0 + verts_per_side;
            const std::uint32_t i3 = i2 + 1;
            mesh.indices.push_back(i0);
            mesh.indices.push_back(i1);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i1);
            mesh.indices.push_back(i3);
            mesh.indices.push_back(i2);
        }
    }

    return mesh;
}

} // namespace

TEST_CASE("V04-style subdivided-grid triangulation is CCW from +Z, agreeing with its +Z normals",
          "[viewport][mesh][M09]") {
    const RenderMeshData mesh = MakeSubdividedGridMirroringV04(/*subdivisions=*/4);

    // Sanity: this mirrors ValidateMesh's own well-formedness expectations
    // too, so a future edit that breaks the topology fails loudly here
    // rather than only in a rendering-side test.
    REQUIRE(ValidateMesh(mesh).IsOk());

    REQUIRE(mesh.indices.size() % 3 == 0);
    const std::size_t triangle_count = mesh.indices.size() / 3;
    REQUIRE(triangle_count > 0);

    for (std::size_t t = 0; t < triangle_count; ++t) {
        const std::uint32_t ia = mesh.indices[t * 3 + 0];
        const std::uint32_t ib = mesh.indices[t * 3 + 1];
        const std::uint32_t ic = mesh.indices[t * 3 + 2];

        const bim::viewport::Position3f& a = mesh.positions[ia];
        const bim::viewport::Position3f& b = mesh.positions[ib];
        const bim::viewport::Position3f& c = mesh.positions[ic];

        // CCW as seen from +Z means a strictly positive 2D cross product.
        // Every vertex here is authored with a {0,0,1} normal, so this is
        // exactly the "winding agrees with the authored outward normal"
        // check M09 requires. (Not using CAPTURE(t, ia, ib, ic) here: this
        // translation unit only has <catch2/catch_test_macros.hpp> included
        // above, matching every other TEST_CASE already in this file, and
        // this correction round does not add a second Catch2 header on
        // unverified faith that CAPTURE is pulled in transitively - REQUIRE
        // alone already reports the failing expression and its operands.)
        REQUIRE(Cross2D(a, b, c) > 0.0f);
    }
}
