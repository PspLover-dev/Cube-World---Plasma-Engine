#pragma once

#include <cstdint>
#include <cstring>

namespace cube::decomp {

// Retail .rdata floats referenced as DAT_* in GlobalFunctions.cpp.
inline constexpr float kDat006fcd94 = 0.0001f;
inline constexpr float kDat00745da0 = 0.5f;
inline constexpr float kDat006fcd9c = 0.004f;
inline constexpr float kDat006fce5c = 180.f;
inline constexpr float kDat00745e10 = 0.001f;
inline constexpr float kDat00745dc0 = 1.f;
inline constexpr float kDat00745e74 = 14.f;
inline constexpr float kDat006fce34 = 14.f;
inline constexpr float kDat006fcda0 = 0.01f;
inline constexpr float kDat006fd338 = 1000.f;

inline constexpr float kDat006fcdcc = 0.96000004f;
inline constexpr float kDat006fd780 = 0.89999998f;
inline constexpr float kDat006fd570 = 0.75f;
inline constexpr float kDat006fd31c = 1.1f;

inline constexpr float kDefaultScaleXY = 0.96000004f;
inline constexpr float kDefaultScaleZ = 2.16000009f;

inline constexpr float kWalkSpeed = 10.f;
inline constexpr float kJumpImpulse = 4.f;

inline float bitsToFloat(uint32_t bits) {
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}

} // namespace cube::decomp
