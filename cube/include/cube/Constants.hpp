#pragma once

namespace cube {

// Retail constants from GlobalFunctions.cpp / decompiled offsets.
inline constexpr float kWalkSpeed = 10.f;
inline constexpr float kRunSpeed = 20.f;
inline constexpr float kJumpImpulse = 4.f;
inline constexpr float kGravity = 18.f;
inline constexpr float kCameraSmooth = 0.5f;       // DAT_00745da0
inline constexpr float kMouseOrbitScale = 0.004f;  // DAT_006fcd9c
inline constexpr float kCameraDistInit = 45.f;
inline constexpr float kCameraDistMin = 12.f;
inline constexpr float kCameraDistMax = 180.f;     // DAT_006fce5c
inline constexpr float kLookHeight = 6.f;
inline constexpr float kGroundZ = 0.f;
inline constexpr float kWorldHalf = 100.f;
inline constexpr float kLookLagMax = 6.f;
inline constexpr float kWalkCycleLen = 3.f;
inline constexpr float kPositionScale = 0.0001f;   // DAT_006fcd94

inline constexpr int kFieldSize = 65536; // Zone field array
inline constexpr int kMaxKeySlots = 256;

} // namespace cube
