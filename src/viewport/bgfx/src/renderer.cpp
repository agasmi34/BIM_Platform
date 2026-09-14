#include "bim/viewport_bgfx/renderer.hpp"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

// AA RD1.8-01 (CWD-independent runtime shader asset resolution): the
// running executable's directory is derived privately here, in the bgfx
// adapter implementation only, via the Win32 process-module path on the
// authoritative Windows platform - no Qt dependency, no public/API
// contract change, no environment-variable or hard-coded path. Lean
// include (no min/max macros, no GDI/sockets); nothing from it escapes
// this translation unit (renderer.hpp stays free of Windows types - the
// architecture checker's R8 neutral-contract rule still holds).
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "renderer_impl.hpp"

// renderer.cpp - UNVERIFIED. Authored by Claude, the Implementation
// Engineer, against the bgfx/bx C++ API as documented, but never compiled
// or run: this session has no Windows execution channel (no shell on the
// target machine, no build/CTest/bgfx runtime). The Operator must build,
// run, and fix any API-surface mismatch (bgfx header names/signatures do
// drift across versions; vcpkg pins bgfx 1.129.8940#1 per the Architecture
// Gate, but this file was written from documentation/memory of that API
// shape, not from the actual installed headers) before this file can be
// treated as verified. See docs/evidence/P0-T003/CLAUDE_HANDOVER.md.

namespace bim::viewport_bgfx {

namespace {

struct PosNormalVertex {
    float x, y, z;
    float nx, ny, nz;
};

// AA RD1.8-01 (CWD_DEPENDENT_RUNTIME_ASSET_DEFECT - PROVEN): returns the
// directory containing the running executable, resolved from the Win32
// process-module path (GetModuleFileNameW(nullptr, ...) - the module that
// started this process, never the caller's current working directory).
// Returns an empty path if it cannot be resolved, which LoadCompiledShader()
// treats as fail-closed. Wide-character API so a Unicode install path is
// preserved exactly; the buffer is grown until the full path fits (the API
// truncates and returns nSize when it does not), capped at the Windows
// long-path limit. Private to this translation unit; no Qt, no environment
// variable, no hard-coded repository/build path, no fallback to the CWD.
// On non-Windows platforms this spike has no authoritative backend (D3D11
// only, Phase A fact), so the executable directory is deliberately NOT
// guessed there - an empty path is returned and shader loading fails
// closed rather than silently depending on the CWD again.
std::filesystem::path RunningExecutableDirectory() {
#if defined(_WIN32)
    constexpr std::size_t kMaxWindowsPathChars = 32768;
    std::wstring buffer(260, L'\0');
    for (;;) {
        const DWORD length =
            ::GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) {
            return {};
        }
        if (static_cast<std::size_t>(length) < buffer.size()) {
            buffer.resize(static_cast<std::size_t>(length));
            break;
        }
        if (buffer.size() >= kMaxWindowsPathChars) {
            return {};
        }
        buffer.resize(buffer.size() * 2);
    }
    const std::filesystem::path executable_path(buffer);
    if (!executable_path.has_parent_path()) {
        return {};
    }
    return executable_path.parent_path();
#else
    return {};
#endif
}

// Loads a shaderc-compiled binary blob for the currently active bgfx
// renderer type. shaderc (invoked at build time by
// src/viewport/bgfx/CMakeLists.txt via a find_program(shaderc)-driven
// custom command) writes one compiled variant per backend into
// <binary_dir>/shaders/<backend>/<name>.bin next to the executable
// (installed/copied there by the same CMake target - see
// src/desktop/CMakeLists.txt POST_BUILD step). D3D11 is the only backend
// this spike ships (Phase A fact), so only the dx11 subdirectory is
// populated; headless/noop mode never calls this function.
//
// AA RD1.8-01 (CWD-independent runtime shader asset resolution): this
// function previously built the path as the RELATIVE string
// "shaders/<backend>/<name>.bin", i.e. relative to the process current
// working directory - Operator evidence proved a repo-root CWD found no
// shaders (renderer_initialize=false, backend=Uninitialized) while a
// build-root or exe-directory CWD initialized Direct3D 11 correctly. The
// deployed layout is, and stays, <executable-dir>\shaders\dx11\<name>.bin
// (the existing POST_BUILD step creates it - no deployment redesign), so
// the path is now anchored to the running executable's directory (above),
// making it independent of whatever CWD the caller happens to have. Shader
// names and the backend subdirectory are unchanged; RendererCreateInfo is
// unchanged (no path is carried through the public contract); fail-closed
// if the executable directory or the file cannot be resolved/opened.
// AA RD1.8 Amendment A2 (bugprone-exception-escape - REAL SOURCE FINDING):
// clang-tidy 19.1.5 (Targeted Post-Correction Verification v3) proved that
// RD1.8-01's executable-relative path resolution - RunningExecutableDirectory()
// above, the std::filesystem::path construction/concatenation below, and the
// std::wstring/std::string allocation both involve - can throw
// (std::bad_alloc, std::filesystem::filesystem_error), and this function is
// called from Renderer::Initialize() below, which is declared `noexcept`.
// This function is therefore now itself declared `noexcept`, and its entire
// body is the private exception boundary: every potentially-throwing
// operation introduced by RD1.8-01 (the RunningExecutableDirectory() call,
// path construction/concatenation, string allocation, the final shader path)
// executes inside the try block, so no exception can propagate out of this
// function - it is caught here and converted to this function's own
// pre-existing fail-closed sentinel, BGFX_INVALID_HANDLE, which
// Renderer::Initialize() already treats as an ordinary shader-load failure
// via its `!bgfx::isValid(vs) || !bgfx::isValid(fs)` check - no new failure
// vocabulary, no weakened fail-closed behavior. This is not a `noexcept`
// label added without a guarantee behind it (which would just replace this
// clang-tidy finding with a possible std::terminate()): every throwing path
// RD1.8-01 introduced is genuinely caught right here. Nothing past the
// file-open call below can throw - std::fseek/std::ftell/std::fread/
// bgfx::alloc/bgfx::createShader are all C-style APIs - so wrapping the
// whole body introduces no resource-leak risk beyond what already existed.
bgfx::ShaderHandle LoadCompiledShader(const char* backend_dir, const char* name) noexcept {
    try {
        const std::filesystem::path executable_dir = RunningExecutableDirectory();
        if (executable_dir.empty()) {
            return BGFX_INVALID_HANDLE;
        }
        const std::filesystem::path shader_path =
            executable_dir / "shaders" / backend_dir / (std::string(name) + ".bin");

        // AA Targeted Full Project Build v5 correction (RD1.4-01) - BUILD
        // BLOCKER: the authoritative MSVC /WX build failed here with C2220
        // (error, treated from warning) because plain std::fopen triggers
        // MSVC's C4996 "this function or variable may be unsafe - consider
        // using fopen_s instead" deprecation warning. Per AA's directive this is
        // fixed at the actual call site (not by suppressing C4996, defining
        // _CRT_SECURE_NO_WARNINGS, or removing /WX/relaxing the warning policy):
        // MSVC uses the vendor-specific secure open, every other toolchain keeps
        // the existing std::fopen behavior unchanged. Both branches feed the
        // same `file == nullptr` failure check immediately below - the secure
        // open reports failure by returning a non-zero errno_t, in which case
        // `file` is left explicitly null here so that existing check still
        // catches it exactly like a failed std::fopen would. RD1.8-01: the MSVC
        // branch is now the wide-character _wfopen_s on the native wchar_t
        // std::filesystem::path, so a Unicode executable path is opened
        // correctly (a narrow conversion could mangle it).
#if defined(_MSC_VER)
        std::FILE* file = nullptr;
        if (_wfopen_s(&file, shader_path.c_str(), L"rb") != 0) {
            file = nullptr;
        }
#else
        std::FILE* file = std::fopen(shader_path.string().c_str(), "rb");
#endif
        if (file == nullptr) {
            return BGFX_INVALID_HANDLE;
        }

        std::fseek(file, 0, SEEK_END);
        const long size = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        if (size <= 0) {
            std::fclose(file);
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory* mem = bgfx::alloc(static_cast<std::uint32_t>(size) + 1);
        const std::size_t read = std::fread(mem->data, 1, static_cast<std::size_t>(size), file);
        std::fclose(file);
        if (read != static_cast<std::size_t>(size)) {
            return BGFX_INVALID_HANDLE;
        }
        mem->data[size] = '\0';

        return bgfx::createShader(mem);
    } catch (...) {
        // AA RD1.8 Amendment A2: any exception from the executable-relative
        // path resolution above (RunningExecutableDirectory(), path/string
        // construction) is caught here and converted to this function's
        // existing fail-closed sentinel - Renderer::Initialize() already
        // treats BGFX_INVALID_HANDLE as an ordinary shader-load failure, so
        // no new failure vocabulary and no weakened fail-closed behavior is
        // introduced by this boundary.
        return BGFX_INVALID_HANDLE;
    }
}

const char* BackendDirName(bgfx::RendererType::Enum type) {
    switch (type) {
        case bgfx::RendererType::Direct3D11:
            return "dx11";
        case bgfx::RendererType::Noop:
            return "noop";
        default:
            return "unknown";
    }
}

} // namespace

std::uint32_t Renderer::Impl::AllocateSlot() {
    std::uint32_t slot_index;
    if (!free_slots.empty()) {
        slot_index = free_slots.back();
        free_slots.pop_back();
    } else {
        slots.emplace_back();
        slot_index = static_cast<std::uint32_t>(slots.size() - 1);
    }
    // Every (re)allocation of a slot index gets a new generation, whether
    // this is the slot's first-ever occupant or a reuse after a same-epoch
    // DestroyMesh() freed it - this is what lets DestroyMesh()/RenderFrame()
    // tell an old occupant's handle apart from a new one at the same slot
    // index (AA Source Review Round 1 finding M03).
    ++slots[slot_index].generation;
    return slot_index;
}

void Renderer::Impl::ReleaseSlot(std::uint32_t slot_index) noexcept {
    // AA Source Review Round 2 MINOR fix: called from DestroyMesh() and
    // CreateMesh()'s failure path, both declared noexcept in renderer.hpp -
    // but free_slots.push_back() can itself throw std::bad_alloc/
    // std::length_error under real memory pressure, which this function's
    // own (previously implicit, now explicit) noexcept would have turned
    // into a std::terminate() crash. If the push_back throws, the slot
    // index is simply not returned to the reuse free-list - a lost
    // future-reuse opportunity, never a crash and never a leaked GPU
    // resource (the caller has already released the actual bgfx
    // vertex/index buffers before calling this; only the bookkeeping that
    // lets a later CreateMesh() reuse this exact slot index is affected).
    //
    // RD1.6-06 (bugprone-empty-catch): these two handlers are the only empty
    // catch blocks in this file (CreateMesh()'s bad_alloc/length_error
    // handlers already return Result<RenderMeshHandle>::Fail(
    // ResourceCreationFailed) and are unchanged). This function is `void
    // ... noexcept` with no failure vocabulary to return into, so the
    // established CreateMesh failure path does not apply here; the CURRENT
    // semantics - deliberately abandon the free-list bookkeeping for this
    // slot index and return normally, never a crash, never a leaked bgfx
    // resource - are preserved exactly, made explicit by an explicit
    // `return;` in each handler (the handling decision stated in code, not
    // a lint suppression and not a dummy expression statement). No new
    // error code, no new state, no widening of the caught exception set.
    try {
        free_slots.push_back(slot_index);
    } catch (const std::bad_alloc&) {
        return; // slot index deliberately not returned to the free-list
    } catch (const std::length_error&) {
        return; // slot index deliberately not returned to the free-list
    }
}

// AA Source Review Round 3 finding MINOR: not noexcept - see renderer.hpp's
// declaration comment. std::make_unique<Impl>() can throw std::bad_alloc.
Renderer::Renderer() : impl_(std::make_unique<Impl>()) {}

Renderer::~Renderer() {
    if (impl_ && impl_->initialized) {
        // Best-effort: a caller that lets a live Renderer go out of scope
        // without an explicit Shutdown() still gets bgfx resources
        // released, but this path cannot surface a Status - callers should
        // prefer an explicit Shutdown() to observe any failure.
        static_cast<void>(Shutdown());
    }
}

Renderer::Renderer(Renderer&&) noexcept = default;
Renderer& Renderer::operator=(Renderer&&) noexcept = default;

bool Renderer::IsInitialized() const noexcept {
    return impl_ && impl_->initialized;
}

// AA Source Review Round 3 finding MINOR: not noexcept - see renderer.hpp's
// declaration comment. The std::string assignments below can throw
// std::bad_alloc.
RendererBackendInfo Renderer::BackendInfo() const {
    RendererBackendInfo info;
    if (!impl_ || !impl_->initialized) {
        info.backendName = "Uninitialized";
        return info;
    }
    const bgfx::RendererType::Enum resolved = bgfx::getRendererType();
    const char* name = bgfx::getRendererName(resolved);
    info.backendName = (name != nullptr) ? name : "Unknown";
    const bgfx::Caps* caps = bgfx::getCaps();
    if (caps != nullptr) {
        info.vendorId = caps->vendorId;
        info.deviceId = caps->deviceId;
        info.homogeneousDepth = caps->homogeneousDepth;
    }
    return info;
}

bim::viewport::Status Renderer::Initialize(const RendererCreateInfo& info) noexcept {
    if (impl_->initialized) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidState);
    }
    if (!info.headless && info.nativeWindowHandle == nullptr) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidInput);
    }
    if (info.widthPx == 0 || info.heightPx == 0) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidInput);
    }

    bgfx::Init init;
    init.type = info.headless ? bgfx::RendererType::Noop : bgfx::RendererType::Direct3D11;
    init.resolution.width = info.widthPx;
    init.resolution.height = info.heightPx;
    init.resolution.reset = BGFX_RESET_VSYNC;

    if (!info.headless) {
        bgfx::PlatformData platform_data{};
        platform_data.nwh = info.nativeWindowHandle;
        init.platformData = platform_data;
    }

    if (!bgfx::init(init)) {
        return bim::viewport::Status::Fail(
            bim::viewport::ViewportErrorCode::RendererInitializationFailed);
    }

    const bgfx::RendererType::Enum resolved = bgfx::getRendererType();
    const bool backend_ok = info.headless ? (resolved == bgfx::RendererType::Noop)
                                          : (resolved == bgfx::RendererType::Direct3D11);
    if (!backend_ok) {
        bgfx::shutdown();
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::UnsupportedBackend);
    }

    impl_->vertex_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .end();

    if (!info.headless) {
        const char* backend_dir = BackendDirName(resolved);
        const bgfx::ShaderHandle vs = LoadCompiledShader(backend_dir, "vs_p0_t003");
        const bgfx::ShaderHandle fs = LoadCompiledShader(backend_dir, "fs_p0_t003");
        if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
            bgfx::shutdown();
            return bim::viewport::Status::Fail(
                bim::viewport::ViewportErrorCode::RendererInitializationFailed);
        }
        impl_->shader_program = bgfx::createProgram(vs, fs, /*destroyShaders=*/true);
        if (!bgfx::isValid(impl_->shader_program)) {
            bgfx::shutdown();
            return bim::viewport::Status::Fail(
                bim::viewport::ViewportErrorCode::RendererInitializationFailed);
        }
        impl_->u_light_dir = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);
        impl_->u_base_color = bgfx::createUniform("u_baseColor", bgfx::UniformType::Vec4);
    }

    impl_->headless = info.headless;
    impl_->width_px = info.widthPx;
    impl_->height_px = info.heightPx;
    impl_->initialized = true;
    return bim::viewport::Status::Ok();
}

bim::viewport::Status Renderer::Shutdown() noexcept {
    if (!impl_->initialized) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidState);
    }

    for (MeshSlot& slot : impl_->slots) {
        if (slot.occupied) {
            if (bgfx::isValid(slot.vertex_buffer)) {
                bgfx::destroy(slot.vertex_buffer);
            }
            if (bgfx::isValid(slot.index_buffer)) {
                bgfx::destroy(slot.index_buffer);
            }
        }
    }
    impl_->slots.clear();
    impl_->free_slots.clear();

    if (bgfx::isValid(impl_->shader_program)) {
        bgfx::destroy(impl_->shader_program);
        impl_->shader_program = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(impl_->u_light_dir)) {
        bgfx::destroy(impl_->u_light_dir);
        impl_->u_light_dir = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(impl_->u_base_color)) {
        bgfx::destroy(impl_->u_base_color);
        impl_->u_base_color = BGFX_INVALID_HANDLE;
    }

    bgfx::shutdown();

    // Full teardown: increment the epoch so every handle issued under the
    // prior epoch is now unconditionally stale, even though slot indices
    // start over from zero again on the next Initialize() (Brief section 4
    // epoch rule).
    ++impl_->epoch;
    impl_->initialized = false;
    return bim::viewport::Status::Ok();
}

bim::viewport::Status Renderer::Resize(std::uint32_t widthPx, std::uint32_t heightPx) noexcept {
    if (!impl_->initialized) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidState);
    }
    if (widthPx == 0 || heightPx == 0) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidInput);
    }

    bgfx::reset(widthPx, heightPx, BGFX_RESET_VSYNC);
    impl_->width_px = widthPx;
    impl_->height_px = heightPx;
    // Ordinary reset: epoch is NOT incremented, outstanding handles remain
    // valid (Brief section 4).
    return bim::viewport::Status::Ok();
}

bim::viewport::Result<RenderMeshHandle>
Renderer::CreateMesh(const bim::viewport::RenderMeshData& mesh) noexcept {
    if (!impl_->initialized) {
        return bim::viewport::Result<RenderMeshHandle>::Fail(
            bim::viewport::ViewportErrorCode::InvalidState);
    }

    const bim::viewport::Status validated = bim::viewport::ValidateMesh(mesh);
    if (!validated) {
        return bim::viewport::Result<RenderMeshHandle>::Fail(validated.Code());
    }

    // AA Source Review Round 2 MINOR fix: CreateMesh is declared noexcept
    // (renderer.hpp) - a promise this implementation did not actually keep,
    // since std::vector<PosNormalVertex>::reserve/push_back and
    // Impl::AllocateSlot() (which itself does std::vector::emplace_back) can
    // all genuinely throw std::bad_alloc/std::length_error under real
    // memory pressure (V03's ~1,000-box scene alone uploads ~24,000
    // vertices per CreateMesh call). An uncaught exception crossing a
    // noexcept boundary calls std::terminate() - silently crashing the
    // whole process instead of returning the ResourceCreationFailed this
    // API already has a vocabulary for. The allocating region is wrapped
    // here so the noexcept promise is actually honored: any allocation
    // failure becomes an ordinary Result<T>::Fail(ResourceCreationFailed),
    // consistent with "never a crash, never silent aliasing" (renderer.hpp
    // DestroyMesh doc comment) applying equally to CreateMesh's own
    // allocation path.
    std::uint32_t slot_index = 0;
    try {
        std::vector<PosNormalVertex> vertices;
        vertices.reserve(mesh.positions.size());
        const bool has_normals = !mesh.normals.empty();
        for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
            PosNormalVertex v{};
            v.x = mesh.positions[i].x;
            v.y = mesh.positions[i].y;
            v.z = mesh.positions[i].z;
            if (has_normals) {
                v.nx = mesh.normals[i].x;
                v.ny = mesh.normals[i].y;
                v.nz = mesh.normals[i].z;
            } else {
                v.nx = 0.0f;
                v.ny = 0.0f;
                v.nz = 1.0f;
            }
            vertices.push_back(v);
        }

        const bgfx::Memory* vertex_mem = bgfx::copy(
            vertices.data(), static_cast<std::uint32_t>(vertices.size() * sizeof(PosNormalVertex)));
        const bgfx::Memory* index_mem =
            bgfx::copy(mesh.indices.data(),
                       static_cast<std::uint32_t>(mesh.indices.size() * sizeof(std::uint32_t)));

        slot_index = impl_->AllocateSlot();
        MeshSlot& slot = impl_->slots[slot_index];
        slot.vertex_buffer = bgfx::createVertexBuffer(vertex_mem, impl_->vertex_layout);
        slot.index_buffer = bgfx::createIndexBuffer(index_mem, BGFX_BUFFER_INDEX32);
        slot.occupied = true;
    } catch (const std::bad_alloc&) {
        return bim::viewport::Result<RenderMeshHandle>::Fail(
            bim::viewport::ViewportErrorCode::ResourceCreationFailed);
    } catch (const std::length_error&) {
        return bim::viewport::Result<RenderMeshHandle>::Fail(
            bim::viewport::ViewportErrorCode::ResourceCreationFailed);
    }

    MeshSlot& slot = impl_->slots[slot_index];

    if (!bgfx::isValid(slot.vertex_buffer) || !bgfx::isValid(slot.index_buffer)) {
        if (bgfx::isValid(slot.vertex_buffer)) {
            bgfx::destroy(slot.vertex_buffer);
        }
        if (bgfx::isValid(slot.index_buffer)) {
            bgfx::destroy(slot.index_buffer);
        }
        slot.occupied = false;
        impl_->ReleaseSlot(slot_index);
        return bim::viewport::Result<RenderMeshHandle>::Fail(
            bim::viewport::ViewportErrorCode::ResourceCreationFailed);
    }

    return bim::viewport::Result<RenderMeshHandle>::Ok(
        RenderMeshHandle(impl_->epoch, slot_index, slot.generation));
}

bim::viewport::Status Renderer::DestroyMesh(RenderMeshHandle handle) noexcept {
    if (!impl_->initialized) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidState);
    }
    if (!handle.IsValid() || handle.epoch_ != impl_->epoch || handle.slot_ >= impl_->slots.size() ||
        !impl_->slots[handle.slot_].occupied ||
        handle.generation_ != impl_->slots[handle.slot_].generation) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::ResourceNotFound);
    }

    MeshSlot& slot = impl_->slots[handle.slot_];
    if (bgfx::isValid(slot.vertex_buffer)) {
        bgfx::destroy(slot.vertex_buffer);
    }
    if (bgfx::isValid(slot.index_buffer)) {
        bgfx::destroy(slot.index_buffer);
    }
    slot.vertex_buffer = BGFX_INVALID_HANDLE;
    slot.index_buffer = BGFX_INVALID_HANDLE;
    slot.occupied = false;
    impl_->ReleaseSlot(handle.slot_);
    return bim::viewport::Status::Ok();
}

bim::viewport::Status Renderer::RenderFrame(const bim::viewport::Camera& camera,
                                            const RenderMeshHandle* meshes,
                                            std::size_t meshCount) noexcept {
    if (!impl_->initialized) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidState);
    }
    if (!camera.IsConfigured()) {
        return bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::InvalidState);
    }

    bgfx::setViewRect(Impl::kViewId, 0, 0, static_cast<std::uint16_t>(impl_->width_px),
                      static_cast<std::uint16_t>(impl_->height_px));
    bgfx::setViewClear(Impl::kViewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x202020ff, 1.0f, 0);
    bgfx::touch(Impl::kViewId);

    if (!impl_->headless) {
        // bx::mtxLookAt / bx::mtxProj are the only place in the entire
        // P0-T003 footprint a view/projection matrix is constructed (Brief
        // section 5) - Camera itself exposes only semantic parameters.
        const bim::viewport::Vector3 eye = camera.Eye();
        const bim::viewport::Vector3 target = camera.Target();
        const bim::viewport::Vector3 up = camera.WorldUp();
        const bx::Vec3 bx_eye{static_cast<float>(eye.x), static_cast<float>(eye.y),
                              static_cast<float>(eye.z)};
        const bx::Vec3 bx_target{static_cast<float>(target.x), static_cast<float>(target.y),
                                 static_cast<float>(target.z)};
        const bx::Vec3 bx_up{static_cast<float>(up.x), static_cast<float>(up.y),
                             static_cast<float>(up.z)};

        float view[16];
        // Explicitly right-handed, matching the handedness already explicit
        // on the bx::mtxProj call below and the project's locked RH world
        // convention (Brief section 4) - relying on bx::mtxLookAt's default
        // parameter to happen to agree is not acceptable (AA Source Review
        // Round 1 finding M07).
        bx::mtxLookAt(view, bx_eye, bx_target, bx_up, bx::Handedness::Right);

        float proj[16];
        // bx::mtxProj's fovy parameter (this overload) is in degrees, not
        // radians - Camera stores verticalFovRadians (Brief section 5), so
        // it is converted here, at the one place a backend matrix is built.
        //
        // AA Targeted Full Project Build v5 correction (RD1.4-03) - BUILD
        // BLOCKER: the authoritative build failed with C2039 ("'kRadToDeg':
        // is not a member of 'bx'") - the installed bx version (vcpkg-pinned
        // per the Architecture Gate) does not expose a bx::kRadToDeg
        // constant; its actual conversion API is the function
        // bx::toDeg(float radians). Replacing the constant-multiply with
        // that function call preserves the exact same radians->degrees
        // value and the exact same existing FOV/camera/projection semantics
        // (Camera::VerticalFovRadians() is still the sole source value, the
        // result still feeds bx::mtxProj exactly as before) - no hard-coded
        // 180/pi replacement constant and no locally-defined kRadToDeg were
        // introduced, per AA's explicit instruction.
        const float fovy_degrees = bx::toDeg(static_cast<float>(camera.VerticalFovRadians()));
        // Right-handed, homogeneous-depth flag taken from the active
        // backend's caps (D3D11 uses a [0, 1] clip-space depth range,
        // reported as homogeneousDepth == false).
        bx::mtxProj(proj, fovy_degrees, static_cast<float>(camera.AspectRatio()),
                    static_cast<float>(camera.NearPlane()), static_cast<float>(camera.FarPlane()),
                    bgfx::getCaps()->homogeneousDepth, bx::Handedness::Right);

        bgfx::setViewTransform(Impl::kViewId, view, proj);

        const float light_dir[4] = {0.3f, -0.5f, 0.8f, 0.0f};
        const float base_color[4] = {0.75f, 0.75f, 0.8f, 1.0f};
        bgfx::setUniform(impl_->u_light_dir, light_dir);
        bgfx::setUniform(impl_->u_base_color, base_color);
    }

    // Handle validation (epoch + per-slot generation, AA Source Review
    // Round 1 finding M03; Brief section 9 "index + generation/epoch
    // protected") runs unconditionally, including in headless mode, so a
    // stale/aliased handle is rejected the same way regardless of backend.
    // This deliberately makes the check reachable from the headless
    // integration-test suite (integration_viewport_bgfx_resource_lifecycle)
    // - the only regression coverage available without a live D3D11 device
    // in this authoring environment (no Windows execution channel here;
    // see docs/evidence/P0-T003/CLAUDE_HANDOVER.md). Only the actual GPU
    // draw submission below stays live-backend-only.
    //
    // AA Source Review Round 2 MINOR fix (RenderFrame stale-handle status
    // contract): renderer.hpp's own doc comment for RenderFrame has always
    // promised that "any handle failing validation is skipped and recorded
    // as ResourceNotFound in the returned Status" - but this implementation
    // previously just `continue`d silently and always returned Ok() at the
    // end, regardless of how many handles were rejected, contradicting its
    // own documented contract. `any_handle_rejected` now makes the returned
    // Status actually reflect that - the frame is still fully submitted
    // (every other valid handle still draws, bgfx::frame() still runs), so
    // a caller ignoring the Status sees exactly the same rendering
    // behavior as before; only a caller that checks the Status can now
    // actually discover that some of what it asked to draw was silently
    // skipped, which is the whole point of returning a Status here at all.
    bool any_handle_rejected = false;
    for (std::size_t i = 0; i < meshCount; ++i) {
        const RenderMeshHandle& handle = meshes[i];
        if (!handle.IsValid() || handle.epoch_ != impl_->epoch ||
            handle.slot_ >= impl_->slots.size() || !impl_->slots[handle.slot_].occupied ||
            handle.generation_ != impl_->slots[handle.slot_].generation) {
            any_handle_rejected = true;
            continue; // stale/invalid handle: skipped, not fatal to the frame
        }
        if (impl_->headless) {
            continue; // headless: handle validated above; no live GPU submission to make
        }
        const MeshSlot& slot = impl_->slots[handle.slot_];
        bgfx::setVertexBuffer(0, slot.vertex_buffer);
        bgfx::setIndexBuffer(slot.index_buffer);
        bgfx::setState(BGFX_STATE_DEFAULT);
        bgfx::submit(Impl::kViewId, impl_->shader_program);
    }

    bgfx::frame();
    return any_handle_rejected
               ? bim::viewport::Status::Fail(bim::viewport::ViewportErrorCode::ResourceNotFound)
               : bim::viewport::Status::Ok();
}

} // namespace bim::viewport_bgfx
