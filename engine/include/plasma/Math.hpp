#pragma once

#include <cmath>
#include <cstring>

namespace plasma {

template <typename T, int N>
struct Vector {
    T data[N]{};

    T& operator[](int i) { return data[i]; }
    const T& operator[](int i) const { return data[i]; }
};

using Vec2 = Vector<float, 2>;
using Vec3 = Vector<float, 3>;
using Vec4 = Vector<float, 4>;

template <typename T>
struct Matrix {
    T m[16]{};

    static Matrix identity() {
        Matrix out{};
        out.m[0] = out.m[5] = out.m[10] = out.m[15] = T(1);
        return out;
    }
};

using Mat4 = Matrix<float>;

inline void setPerspective(Mat4& out, float fovY, float aspect, float zNear, float zFar) {
    const float f = 1.f / std::tan(fovY * 0.5f);
    out = Mat4::identity();
    out.m[0] = f / aspect;
    out.m[5] = f;
    out.m[10] = (zFar + zNear) / (zNear - zFar);
    out.m[11] = -1.f;
    out.m[14] = (2.f * zFar * zNear) / (zNear - zFar);
    out.m[15] = 0.f;
}

inline void setLookAt(Mat4& out, const Vec3& eye, const Vec3& center, const Vec3& up) {
    const float fx = center[0] - eye[0];
    const float fy = center[1] - eye[1];
    const float fz = center[2] - eye[2];
    const float fl = std::sqrt(fx * fx + fy * fy + fz * fz);
    const float ifl = fl > 0.f ? 1.f / fl : 0.f;
    const float sx = fy * up[2] - fz * up[1];
    const float sy = fz * up[0] - fx * up[2];
    const float sz = fx * up[1] - fy * up[0];
    const float sl = std::sqrt(sx * sx + sy * sy + sz * sz);
    const float isl = sl > 0.f ? 1.f / sl : 0.f;
    const float rsx = sx * isl;
    const float rsy = sy * isl;
    const float rsz = sz * isl;
    const float rfx = fx * ifl;
    const float rfy = fy * ifl;
    const float rfz = fz * ifl;
    const float ux = rsy * rfz - rsz * rfy;
    const float uy = rsz * rfx - rsx * rfz;
    const float uz = rsx * rfy - rsy * rfx;

    out = Mat4::identity();
    // Column-major: basis vectors in columns 0..2.
    out.m[0] = rsx;
    out.m[1] = rsy;
    out.m[2] = rsz;
    out.m[4] = ux;
    out.m[5] = uy;
    out.m[6] = uz;
    out.m[8] = -rfx;
    out.m[9] = -rfy;
    out.m[10] = -rfz;
    out.m[12] = -(out.m[0] * eye[0] + out.m[1] * eye[1] + out.m[2] * eye[2]);
    out.m[13] = -(out.m[4] * eye[0] + out.m[5] * eye[1] + out.m[6] * eye[2]);
    out.m[14] = -(out.m[8] * eye[0] + out.m[9] * eye[1] + out.m[10] * eye[2]);
}

inline Mat4 rotationY(float angle) {
    Mat4 m = Mat4::identity();
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    m.m[0] = c;
    m.m[2] = -s;
    m.m[8] = s;
    m.m[10] = c;
    return m;
}

// Rotation around +Z (Cube World ground normal; horizontal plane is XY).
inline Mat4 rotationZ(float angle) {
    Mat4 m = Mat4::identity();
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    m.m[0] = c;
    m.m[1] = s;
    m.m[4] = -s;
    m.m[5] = c;
    return m;
}

inline Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 out{};
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.f;
            for (int k = 0; k < 4; ++k) {
                sum += a.m[k * 4 + row] * b.m[col * 4 + k];
            }
            out.m[col * 4 + row] = sum;
        }
    }
    return out;
}

} // namespace plasma
