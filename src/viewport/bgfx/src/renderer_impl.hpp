#pragma once

#include <cstdint>
#include <vector>

#include <bgfx/bgfx.h>

#include "bim/viewport_bgfx/renderer.hpp"

// renderer_impl.hpp - PRIVATE. Not part of the public bim::viewport_bgfx
// footprint (not installed/exported, not reachable via renderer.hpp).
// Included only by renderer.cpp. This is the one file in the whole
// P0-T003 footprint where <bgfx/bgfx.h> and bx types are allowed to appear
// alongside actual GPU resource storage.

namespace bim::viewport_bgfx {

struct MeshSlot {
    bgfx::VertexBufferHandle vertex_buffer = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle index_buffer = BGFX_INVALID_HANDLE;
    bool occupied = false;

    // Bumped by Renderer::Impl::AllocateSlot() every time this slot index
    // is handed out - including slot reuse after a same-epoch
    // DestroyMesh(). Persists across a slot's occupied->free->occupied
    // cycle (only Shutdown() resets it, by clearing `slots` outright), so a
    // RenderMeshHandle captured for one occupant of this slot can never be
    // mistaken for a later occupant of the same slot within the same epoch
    // (AA Source Review Round 1 finding M03; Brief section 9 "index +
    // generation/epoch protected").
    std::uint32_t generation = 0;
};

struct Renderer::Impl {
    bool initialized = false;
    bool headless = false;
    std::uint32_t width_px = 0;
    std::uint32_t height_px = 0;

    // Incremented on every full Shutdown()->Initialize() cycle. Every
    // RenderMeshHandle this Impl hands out is tagged with the epoch that
    // was current at CreateMesh() time (Brief section 4 epoch rule).
    std::uint32_t epoch = 0;

    bgfx::ProgramHandle shader_program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_light_dir = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_base_color = BGFX_INVALID_HANDLE;
    bgfx::VertexLayout vertex_layout;

    // Slots are reused across CreateMesh/DestroyMesh calls within a single
    // epoch (by index, via free_slots), but never across an epoch boundary
    // - Shutdown() clears both slots and free_slots outright, so a stale
    // -epoch handle's slot index no longer corresponds to anything. Within
    // an epoch, same-slot reuse is disambiguated by MeshSlot::generation
    // (see AllocateSlot()), not just by occupied/index alone.
    std::vector<MeshSlot> slots;
    std::vector<std::uint32_t> free_slots;

    // bgfx view id reserved for this Renderer's single viewport. P0-T003
    // is a single-viewport spike (AA Source Review Round 1 MINOR fix: the
    // prior comment cited "Brief section 13," which is actually "Shaders"
    // - no Brief section explicitly states a viewport-count constraint;
    // this is this authoring pass's own design choice, not a Brief
    // mandate), so one fixed view id (0) is sufficient; a future
    // multi-viewport consumer would need to allocate distinct view ids per
    // Renderer instance.
    static constexpr bgfx::ViewId kViewId = 0;

    // Returns a slot index (new or reused-from-free_slots) with
    // slots[index].generation already incremented to the new occupant's
    // generation - the caller reads it back from slots[index].generation
    // when constructing the RenderMeshHandle to return.
    [[nodiscard]] std::uint32_t AllocateSlot();
    // noexcept (AA Source Review Round 2 MINOR fix): every caller
    // (DestroyMesh, CreateMesh's failure path) is itself declared noexcept
    // in renderer.hpp - see renderer.cpp's definition for how an internal
    // allocation failure is absorbed rather than propagated.
    void ReleaseSlot(std::uint32_t slot_index) noexcept;
};

} // namespace bim::viewport_bgfx
