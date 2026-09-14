$input a_position, a_normal
$output v_normal

// vs_p0_t003.sc - P0-T003 spike vertex shader. UNVERIFIED - authored
// against bgfx's shaderc/BGFX_SHADER conventions but never compiled in
// this session (no execution channel). Transforms position by the
// standard bgfx model/view/projection uniform chain (u_modelViewProj,
// provided automatically by bgfx via bgfx::setTransform / the built-in
// uniform set - see renderer.cpp's RenderFrame, which does not set a
// per-draw model matrix explicitly since P0-T003 draws every mesh at
// identity model transform; bgfx defaults an unset transform to
// identity) and passes the object-space normal through to the fragment
// stage unmodified (Brief section 8/13: the spike's lighting is
// deliberately simple - no per-instance model matrix, no normal-matrix
// correction for non-uniform scale, since spike meshes are never
// non-uniformly scaled).

#include <bgfx_shader.sh>

void main() {
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_normal = a_normal;
}
