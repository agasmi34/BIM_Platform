#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "bim/viewport/camera.hpp"
#include "bim/viewport/error.hpp"
#include "bim/viewport/mesh.hpp"

// bim/viewport_bgfx/renderer.hpp - the bgfx-backed renderer adapter
// (Implementation Brief BIM-TASK-P0-T003-CLAUDE v1.0, sections 8-9, 11).
//
// This is the ONLY public header bim::viewport_bgfx exposes. It owns every
// first-party bgfx include (Brief section 9: "bim::viewport_bgfx owns all
// bgfx includes"); no bgfx type appears in this header's signature - the
// backend state lives entirely behind Impl (pimpl), defined only in
// renderer_impl.hpp / renderer.cpp. D3D11 is the sole authoritative
// backend selected by bgfx::init for this target (Phase A fact, restated
// Brief section 2/9); this header itself is still backend-agnostic at the
// C++ type level - no D3D/Windows native type appears here either.
//
// Threading (Brief section 9): all methods on Renderer must be called from
// the same thread that constructed it (bgfx's single "render thread"
// contract for this spike; bgfx's own internal submit-thread/render-thread
// split, if any, is entirely an implementation detail hidden behind Impl).
// Renderer itself performs no internal synchronization.

namespace bim::viewport_bgfx {

// Non-owning handle to a GPU-resident mesh, returned by Renderer::CreateMesh
// (AA Source Review Round 2 finding: the prior comment called this an
// "Owning handle," which is incorrect and could mislead a caller into
// thinking a handle's destructor/copy releases the underlying GPU resource
// the way an RAII owner would. It does not: RenderMeshHandle is a plain,
// trivially-copyable (epoch, slot, generation) capability/index into the
// Renderer's own slot table. The Renderer instance that produced a handle
// is the sole owner of the underlying vertex/index buffers; copying or
// letting a handle go out of scope has no effect on that resource one way
// or the other. The resource is released only by an explicit
// Renderer::DestroyMesh(handle) call - a caller that drops every copy of a
// handle without calling DestroyMesh() has leaked the GPU resource until
// the owning Renderer's next Shutdown(), not freed it.
// Handles are epoch-tagged AND per-slot generation-tagged (Brief section 9:
// "index + generation/epoch protected"):
//   - an ordinary Resize()/reset does NOT invalidate outstanding handles;
//   - a full Shutdown()+Initialize() cycle increments the renderer's
//     internal epoch and invalidates every handle from the prior epoch
//     (Brief section 4: "renderer epoch" resource-lifetime rule);
//   - WITHIN a single epoch, DestroyMesh() releases a slot back to the free
//     list and a later CreateMesh() may reuse that exact slot index for an
//     unrelated mesh. The epoch alone cannot distinguish the old occupant's
//     handle from the new one, since both carry the same epoch and slot -
//     so each slot additionally carries a generation counter, incremented
//     every time the slot is (re)allocated, and every handle carries the
//     generation that was current at its own CreateMesh() time.
// A handle is therefore only accepted if its (epoch, slot, generation)
// triple exactly matches the slot's current occupant. A stale-epoch handle,
// an already-destroyed handle, or a same-epoch handle whose slot was
// reused for a different mesh in the interim, all fail this check the same
// way and never alias another mesh's GPU resource - Renderer detects it and
// returns ResourceNotFound (AA Source Review Round 1 finding M03).
class RenderMeshHandle {
public:
    RenderMeshHandle() noexcept = default;

    [[nodiscard]] bool IsValid() const noexcept { return valid_; }

private:
    friend class Renderer;
    RenderMeshHandle(std::uint32_t epoch, std::uint32_t slot, std::uint32_t generation) noexcept
        : epoch_(epoch), slot_(slot), generation_(generation), valid_(true) {}

    std::uint32_t epoch_ = 0;
    std::uint32_t slot_ = 0;
    std::uint32_t generation_ = 0;
    bool valid_ = false;
};

// Project-owned, neutral backend-identity snapshot (Brief section 12: the
// renderer adapter "may expose project-owned operations equivalent to: ...
// BackendInfo()"). Added to satisfy the P0-T003 evidence mode's required
// "selected renderer backend; GPU adapter/backend information available
// from bgfx" JSON fields (Brief section 22) without letting a bgfx type
// leak from this header (Brief section 9).
struct RendererBackendInfo {
    // The backend bgfx::getRendererType() actually resolved to after
    // Initialize() (e.g. "Direct3D11", "Noop") - always the resolved
    // backend, never merely the one requested (Brief section 9:
    // "Auto-selecting a different backend and calling that PASS is
    // forbidden" applies equally to what evidence records).
    std::string backendName;
    std::uint32_t vendorId = 0;
    std::uint32_t deviceId = 0;
    bool homogeneousDepth = false;
};

struct RendererCreateInfo {
    // Native window handle (HWND on Windows) to render into. Required
    // unless headless == true. Owned by the caller (src/desktop); Renderer
    // never creates or destroys the native window itself.
    void* nativeWindowHandle = nullptr;

    std::uint32_t widthPx = 0;
    std::uint32_t heightPx = 0;

    // Headless mode (Brief section 12: headless/test split) initializes
    // bgfx against a noop or off-screen backend suitable for CI/CTest
    // execution with no visible window and no live D3D device - used by
    // integration_viewport_bgfx_headless and
    // integration_viewport_bgfx_resource_lifecycle. When true,
    // nativeWindowHandle is ignored (may be nullptr).
    bool headless = false;
};

class Renderer {
public:
    // AA Source Review Round 3 finding MINOR: NOT noexcept - impl_'s
    // std::make_unique<Impl>() is a heap allocation that can throw
    // std::bad_alloc, and this constructor has no internal exception-safety
    // wrapping that would make that guarantee genuine (unlike CreateMesh(),
    // there is no Result<T>/Status vocabulary a constructor can return, and
    // catching bad_alloc internally here would leave impl_ == nullptr while
    // every other method on this class assumes impl_ is always valid once
    // constructed - a broken invariant worse than simply propagating the
    // exception). A previous revision marked this noexcept without
    // justifying it against this actual allocation.
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) noexcept;
    Renderer& operator=(Renderer&&) noexcept;

    // Uninitialized -> live backend. Fails with RendererInitializationFailed
    // if bgfx::init does not report success, or UnsupportedBackend if the
    // resolved renderer type is not the authoritative D3D11 backend (or,
    // in headless mode, the noop backend) - see Brief section 2/9.
    [[nodiscard]] bim::viewport::Status Initialize(const RendererCreateInfo& info) noexcept;

    // Live backend -> torn down. Destroys every outstanding mesh resource
    // and increments the renderer epoch, so any RenderMeshHandle obtained
    // before this call is permanently stale afterward, even if a later
    // Initialize() reuses the same slot indices internally. Calling
    // Shutdown() on an already-shut-down (or never-initialized) Renderer
    // is InvalidState.
    [[nodiscard]] bim::viewport::Status Shutdown() noexcept;

    // Ordinary resize/reset (Brief section 4: "does not invalidate
    // handles"). Requires a live backend (InvalidState otherwise).
    //
    // AA Runbook D Attempt 1 correction (RD1-06): the authoritative
    // clang-format run (repository .clang-format, LLVM 19.1.5) reported
    // this declaration as a formatting violation - the previous single-line
    // form exceeded the column limit. Wrapped here to match this same
    // file's existing style for other over-limit declarations
    // (CreateMesh()/RenderFrame() below): the return type and function name
    // stay on the declaring line, one parameter per continuation line.
    [[nodiscard]] bim::viewport::Status Resize(std::uint32_t widthPx,
                                               std::uint32_t heightPx) noexcept;

    // Validates mesh (bim::viewport::ValidateMesh) and, on success,
    // uploads it to GPU-resident vertex/index buffers, returning a handle
    // tagged with the current renderer epoch. Requires a live backend.
    [[nodiscard]] bim::viewport::Result<RenderMeshHandle>
    CreateMesh(const bim::viewport::RenderMeshData& mesh) noexcept;

    // Releases the GPU resources for a mesh created by this Renderer
    // instance's current epoch. A handle from a prior epoch, an already
    // -destroyed handle, or an IsValid() == false handle all yield
    // ResourceNotFound - never a crash, never silent aliasing.
    [[nodiscard]] bim::viewport::Status DestroyMesh(RenderMeshHandle handle) noexcept;

    // Submits one frame: clears, sets the view/projection matrices derived
    // from camera (bx::mtxLookAt / bx::mtxProj - the only place in the
    // entire P0-T003 footprint a view/projection matrix is constructed;
    // see camera.hpp), draws each handle in meshes[0..meshCount), and
    // calls bgfx::frame(). Requires a live backend and IsConfigured()
    // camera (InvalidState otherwise); any handle failing validation is
    // skipped - not aborting the whole frame, every other valid handle in
    // meshes[] is still submitted and bgfx::frame() still runs - but the
    // Status this call returns reports ResourceNotFound when at least one
    // handle was rejected (AA Source Review Round 2 finding: the previous
    // implementation always returned Ok() here regardless of how many
    // handles were silently skipped, contradicting this exact comment;
    // renderer.cpp now actually honors it).
    [[nodiscard]] bim::viewport::Status RenderFrame(const bim::viewport::Camera& camera,
                                                    const RenderMeshHandle* meshes,
                                                    std::size_t meshCount) noexcept;

    [[nodiscard]] bool IsInitialized() const noexcept;

    // Snapshot of the actually-resolved backend/adapter identity. Valid to
    // call once IsInitialized() - returns a default-constructed (empty
    // "Uninitialized" name) RendererBackendInfo otherwise. Added for the
    // P0-T003 evidence mode (AA Source Review Round 1 finding M05; Brief
    // section 22).
    //
    // AA Source Review Round 3 finding MINOR: NOT noexcept - the
    // std::string assignments building RendererBackendInfo::backendName can
    // throw std::bad_alloc, and RendererBackendInfo has no error-reporting
    // field this method could signal an allocation failure through (unlike
    // CreateMesh()'s Result<T>). A previous revision marked this noexcept
    // without justifying it against that allocation.
    [[nodiscard]] RendererBackendInfo BackendInfo() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bim::viewport_bgfx
