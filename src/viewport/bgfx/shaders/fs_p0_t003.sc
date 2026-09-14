$input v_normal

// fs_p0_t003.sc - P0-T003 spike fragment shader. UNVERIFIED - authored
// against bgfx's shaderc/BGFX_SHADER conventions but never compiled in
// this session (no execution channel). Simple N.L (Lambertian)
// single-directional-light shading against u_baseColor - deliberately the
// minimum needed to visually distinguish faces in the spike scenes (Brief
// section 13); not a physically based shading model.

#include <bgfx_shader.sh>

uniform vec4 u_lightDir;
uniform vec4 u_baseColor;

void main() {
    vec3 n = normalize(v_normal);
    vec3 l = normalize(u_lightDir.xyz);
    float ndotl = max(dot(n, l), 0.0);
    float ambient = 0.25;
    float intensity = min(ambient + ndotl * (1.0 - ambient), 1.0);
    gl_FragColor = vec4(u_baseColor.rgb * intensity, u_baseColor.a);
}
