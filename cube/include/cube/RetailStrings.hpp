#pragma once

// Retail .rdata wide string literals referenced as &DAT_* in classes/cube__World.cpp.
// Resolved via race/equip IDs and Cube World alpha datamine references.

namespace cube::retail_strings {

inline constexpr const char* kImp = "Imp";              // DAT_0071aa44 @ race 0x55
inline constexpr const char* kFly = "Fly";              // DAT_0071aacc @ race 0x3c
inline constexpr const char* kOwl = "Owl";              // DAT_0071ad10 @ race 0x5c
inline constexpr const char* kBat = "Bat";              // DAT_0071adfc @ race 0x3b
inline constexpr const char* kAim = "Aim";              // DAT_0071af1c @ race 0x8d
inline constexpr const char* kEgg = "Egg";              // DAT_0071b0c0 @ race 0x18
inline constexpr const char* kCat = "Cat";              // DAT_0071b118 @ race 0x1e
inline constexpr const char* kPig = "Pig";              // DAT_0071b154 @ race 0x21
inline constexpr const char* kCow = "Cow";              // DAT_0071b174 @ race 0x64
inline constexpr const char* kHelm = "Helm";            // DAT_00700a30 @ equip 0x13/0
inline constexpr const char* kAxe = "Axe";              // DAT_0071b3d0 @ equip 3/1
inline constexpr const char* kBow = "Bow";              // DAT_0071b3d8 @ equip 3/6
inline constexpr const char* kLog = "Log";              // DAT_0071bad8 @ material 0xb/1
inline constexpr const char* kOrb = "Orb";              // DAT_0071bb5c @ material 0xb/0xd
inline constexpr const char* kPet = "Pet";              // DAT_0071bcf8 @ misc 0x15/3
inline constexpr const char* kVase = "Vase";            // DAT_0071be5c @ static 0x10/0

} // namespace cube::retail_strings
