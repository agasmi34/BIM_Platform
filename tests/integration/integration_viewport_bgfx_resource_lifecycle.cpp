#include <catch2/catch_test_macros.hpp>

#include "bim/viewport/camera.hpp"
#include "bim/viewport/mesh.hpp"
#include "bim/viewport_bgfx/renderer.hpp"

// integration_viewport_bgfx_resource_lifecycle.cpp - exercises the
// RenderMeshHandle epoch contract against the headless/noop backend
// (Implementation Brief section 4: "ordinary resize/reset does not
// invalidate handles; only a full renderer shutdown/re-initialization
// increments the renderer epoch and invalidates all handles from the
// prior epoch"). UNVERIFIED - see
// docs/evidence/P0-T003/CLAUDE_HANDOVER.md.

using bim::viewport::Camera;
using bim::viewport::MeshTopology;
using bim::viewport::RenderMeshData;
using bim::viewport::ViewportErrorCode;
using bim::viewport_bgfx::Renderer;
using bim::viewport_bgfx::RendererCreateInfo;
using bim::viewport_bgfx::RenderMeshHandle;

namespace {

RenderMeshData MakeTriangle() {
    RenderMeshData mesh;
    mesh.topology = MeshTopology::TriangleList;
    mesh.positions = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    mesh.indices = {0, 1, 2};
    return mesh;
}

RendererCreateInfo MakeHeadlessInfo() {
    RendererCreateInfo info;
    info.headless = true;
    info.widthPx = 640;
    info.heightPx = 480;
    return info;
}

} // namespace

TEST_CASE("CreateMesh succeeds for a valid mesh and DestroyMesh releases it",
          "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    const auto created = renderer.CreateMesh(MakeTriangle());
    REQUIRE(created.IsOk());
    REQUIRE(created.Value().IsValid());

    REQUIRE(renderer.DestroyMesh(created.Value()).IsOk());

    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE(
    "CreateMesh rejects an invalid mesh with the ValidateMesh diagnosis, not a generic failure",
    "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    RenderMeshData empty_mesh;
    const auto result = renderer.CreateMesh(empty_mesh);
    REQUIRE_FALSE(result.IsOk());
    REQUIRE(result.Code() == ViewportErrorCode::InvalidMesh);

    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("CreateMesh on an uninitialized Renderer is refused with InvalidState",
          "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    const auto result = renderer.CreateMesh(MakeTriangle());
    REQUIRE_FALSE(result.IsOk());
    REQUIRE(result.Code() == ViewportErrorCode::InvalidState);
}

TEST_CASE("Destroying an already-destroyed handle returns ResourceNotFound, never a crash",
          "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    const auto created = renderer.CreateMesh(MakeTriangle());
    REQUIRE(created.IsOk());
    const RenderMeshHandle handle = created.Value();

    REQUIRE(renderer.DestroyMesh(handle).IsOk());
    const auto second_destroy = renderer.DestroyMesh(handle);
    REQUIRE_FALSE(second_destroy.IsOk());
    REQUIRE(second_destroy.Code() == ViewportErrorCode::ResourceNotFound);

    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("An unset (default-constructed) RenderMeshHandle is never valid and always yields "
          "ResourceNotFound",
          "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    const RenderMeshHandle unset_handle;
    REQUIRE_FALSE(unset_handle.IsValid());

    const auto status = renderer.DestroyMesh(unset_handle);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::ResourceNotFound);

    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("Resize does not invalidate outstanding mesh handles",
          "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    const auto created = renderer.CreateMesh(MakeTriangle());
    REQUIRE(created.IsOk());
    const RenderMeshHandle handle = created.Value();

    REQUIRE(renderer.Resize(1920, 1080).IsOk());

    // Still destroyable after an ordinary resize - the handle survived.
    REQUIRE(renderer.DestroyMesh(handle).IsOk());

    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("A full Shutdown()+Initialize() cycle invalidates every handle from the prior epoch",
          "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    const auto created = renderer.CreateMesh(MakeTriangle());
    REQUIRE(created.IsOk());
    const RenderMeshHandle stale_handle = created.Value();

    REQUIRE(renderer.Shutdown().IsOk());
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    // The prior epoch's handle must never alias whatever now occupies slot
    // 0 in the new epoch - DestroyMesh must report ResourceNotFound, not
    // silently succeed against a different mesh.
    const auto status = renderer.DestroyMesh(stale_handle);
    REQUIRE_FALSE(status.IsOk());
    REQUIRE(status.Code() == ViewportErrorCode::ResourceNotFound);

    REQUIRE(renderer.Shutdown().IsOk());
}

TEST_CASE("Slot indices are reused within a single epoch after a destroy",
          "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    const auto first = renderer.CreateMesh(MakeTriangle());
    REQUIRE(first.IsOk());
    REQUIRE(renderer.DestroyMesh(first.Value()).IsOk());

    const auto second = renderer.CreateMesh(MakeTriangle());
    REQUIRE(second.IsOk());
    REQUIRE(renderer.DestroyMesh(second.Value()).IsOk());

    REQUIRE(renderer.Shutdown().IsOk());
}

// Regression test for AA Source Review Round 1 finding M03: a per-slot
// generation counter must stop a stale same-epoch handle from aliasing a
// slot's new occupant. Before this fix, RenderMeshHandle carried only
// (epoch, slot): once a slot was destroyed and immediately reused within
// the same epoch (the LIFO free_slots reuse the test above already
// demonstrates), the destroyed handle's (epoch, slot) pair was
// indistinguishable from the new occupant's - DestroyMesh()/RenderFrame()
// would have treated the stale handle as referring to the NEW mesh's live
// GPU buffers, which is exactly the aliasing bug Brief section 9 forbids
// ("Stale or double-destroy use returns ResourceNotFound and must never
// alias a newer resource occupying the same slot.").
TEST_CASE("A stale handle to a slot reused within the same epoch is rejected, never aliasing the "
          "new occupant",
          "[viewport_bgfx][resource_lifecycle]") {
    Renderer renderer;
    REQUIRE(renderer.Initialize(MakeHeadlessInfo()).IsOk());

    const auto first = renderer.CreateMesh(MakeTriangle());
    REQUIRE(first.IsOk());
    const RenderMeshHandle stale_handle = first.Value();
    REQUIRE(renderer.DestroyMesh(stale_handle).IsOk());

    // Slot indices are reused LIFO (free_slots.back()), so this second
    // CreateMesh is expected to occupy the exact same slot index the
    // just-destroyed first handle referenced, within the SAME epoch (no
    // Shutdown()/Initialize() cycle has happened here) - the scenario the
    // epoch-only check could not previously distinguish.
    const auto second = renderer.CreateMesh(MakeTriangle());
    REQUIRE(second.IsOk());
    const RenderMeshHandle current_handle = second.Value();

    // The stale, prior-generation handle must never be accepted as
    // referring to the slot's new occupant.
    const auto stale_destroy = renderer.DestroyMesh(stale_handle);
    REQUIRE_FALSE(stale_destroy.IsOk());
    REQUIRE(stale_destroy.Code() == ViewportErrorCode::ResourceNotFound);

    // RenderFrame must likewise skip the stale handle rather than
    // submitting the new occupant's buffers under the old handle's
    // identity - the frame as a whole is still fully submitted (bgfx::frame()
    // still runs; this is not a fatal condition), but AA Source Review
    // Round 2 fixed a real contract bug here: RenderFrame's own doc comment
    // (renderer.hpp) has always promised the returned Status reports
    // ResourceNotFound when a handle was rejected, but the implementation
    // previously always returned Ok() regardless. It now honors that
    // contract, so this stale-handle-only call must report the rejection.
    Camera camera;
    REQUIRE(camera.SetLookAt({0, -5, 2}, {0, 0, 0}, {0, 0, 1}).IsOk());
    REQUIRE(camera.SetPerspective(0.9, 0.1, 1000.0).IsOk());
    REQUIRE(camera.SetAspectRatio(1.777).IsOk());
    const RenderMeshHandle stale_handles_array[] = {stale_handle};
    const auto render_status = renderer.RenderFrame(camera, stale_handles_array, 1);
    REQUIRE_FALSE(render_status.IsOk());
    REQUIRE(render_status.Code() == ViewportErrorCode::ResourceNotFound);

    // A frame containing a MIX of one valid and one stale handle must still
    // draw the valid one (not abort the whole frame) while still reporting
    // the rejection in its Status - proof the fix is "record, don't abort."
    const RenderMeshHandle mixed_handles_array[] = {current_handle, stale_handle};
    const auto mixed_render_status = renderer.RenderFrame(camera, mixed_handles_array, 2);
    REQUIRE_FALSE(mixed_render_status.IsOk());
    REQUIRE(mixed_render_status.Code() == ViewportErrorCode::ResourceNotFound);

    // The current (new-generation) handle must remain fully valid and
    // independently destroyable - proof the stale handle did not alias or
    // otherwise disturb it.
    REQUIRE(renderer.DestroyMesh(current_handle).IsOk());

    REQUIRE(renderer.Shutdown().IsOk());
}
