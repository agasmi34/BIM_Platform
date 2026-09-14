#pragma once

#include <cmath>
#include <cstdint>

// bim/viewport/math.hpp - the minimal, project-owned, header-only math
// vocabulary shared by the neutral viewport contract (Implementation Brief
// BIM-TASK-P0-T003-CLAUDE v1.0, sections 4-7).
//
// Two distinct precision families are used deliberately, per the Brief's
// precision contract (section 5): "GPU mesh positions/normals: float32.
// CPU camera/ray/math: double precision." Position3f/Normal3f (float32) and
// Vector3 (double) are therefore separate, non-interchangeable types rather
// than one template parameterized on precision - this makes an accidental
// float/double mix-up (e.g. passing a mesh position where a camera-space
// vector is expected) a compile error instead of a silent narrowing
// conversion.
//
// Header-only by design: no math.cpp exists in the authorized P0-T003
// footprint. Forbidden here: Qt, bgfx, D3D, Windows native types, OCCT, BIM
// semantic/model/query types.

namespace bim::viewport {

// ------------------------------------------------------------------------
// GPU mesh precision (float32) - see mesh.hpp for RenderMeshData itself.
// ------------------------------------------------------------------------
struct Position3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Normal3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// ------------------------------------------------------------------------
// CPU camera/ray precision (double) - used by camera.hpp and ray.hpp.
// ------------------------------------------------------------------------
struct Vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    [[nodiscard]] constexpr Vector3 operator+(const Vector3& rhs) const noexcept {
        return Vector3{x + rhs.x, y + rhs.y, z + rhs.z};
    }
    [[nodiscard]] constexpr Vector3 operator-(const Vector3& rhs) const noexcept {
        return Vector3{x - rhs.x, y - rhs.y, z - rhs.z};
    }
    [[nodiscard]] constexpr Vector3 operator*(double s) const noexcept {
        return Vector3{x * s, y * s, z * s};
    }
};

[[nodiscard]] constexpr double Dot(const Vector3& a, const Vector3& b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

[[nodiscard]] constexpr Vector3 Cross(const Vector3& a, const Vector3& b) noexcept {
    return Vector3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

[[nodiscard]] inline double Length(const Vector3& v) noexcept {
    return std::sqrt(Dot(v, v));
}

// Returns {0,0,0} for a (near-)zero-length input rather than dividing by a
// near-zero magnitude; callers that must distinguish "degenerate input" from
// "a genuine zero vector" validate Length(v) themselves first (see camera.cpp
// / ray.cpp, which do exactly this before calling Normalize()).
[[nodiscard]] inline Vector3 Normalize(const Vector3& v) noexcept {
    const double len = Length(v);
    constexpr double kMinLength = 1.0e-12;
    if (len < kMinLength) {
        return Vector3{0.0, 0.0, 0.0};
    }
    return Vector3{v.x / len, v.y / len, v.z / len};
}

[[nodiscard]] inline bool IsFinite(double v) noexcept {
    return std::isfinite(v);
}

[[nodiscard]] inline bool IsFinite(const Vector3& v) noexcept {
    return IsFinite(v.x) && IsFinite(v.y) && IsFinite(v.z);
}

[[nodiscard]] inline bool IsFinite(float v) noexcept {
    return std::isfinite(v);
}

} // namespace bim::viewport
