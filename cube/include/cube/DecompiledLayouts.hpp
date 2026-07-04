#pragma once

#include "cube/DecompiledConstants.hpp"
#include "cube/Types.hpp"

#include <cstdint>
#include <cstring>

namespace cube {

// Entity / creature field layout (retail offsets used by behaviors + GameController).
#pragma pack(push, 1)
struct EntityLayout {
    uint8_t pad0[0x10];
    int32_t posXFixed;
    int32_t posXHi;
    int32_t posYFixed;
    int32_t posYHi;
    int32_t posZFixed;
    int32_t posZHi;
    uint8_t pad40[0x40 - 0x28];
    float velX;
    float velY;
    float velZ;
    uint8_t pad4c[0x68 - 0x4c];
    char stateChar;
    uint8_t pad69[0x124 - 0x69];
    uint16_t flags124;
    uint8_t pad126[0x160 - 0x126];
    float lookX;
    float lookY;
    float lookZ;
};

struct ControllerLayout {
    uint8_t flags0[7];
    uint8_t activeFlag;
    uint8_t pad1[5];
    void* owner8;
    uint8_t pad10[4];
    uint8_t keyHeld[256];
    uint8_t keyEdgeDown[2];
    uint8_t keyEdgeUp[2];
    uint8_t pad119[7];
    void* fieldC0;
    void* fieldC4;
    uint8_t padC8[8];
    void* fieldB0;
    uint8_t padB4[0xd0 - 0xb4];
    float mousePrevX;
    float mouseX;
    float mousePrevY;
    float mouseY;
    uint8_t padE0[0xf0 - 0xe0];
    uint32_t fieldF0;
};

struct GameControllerFields {
    ControllerLayout base;
    uint8_t _pad54[0x54];
    int32_t scrollDirection;
    uint8_t _pad58[4];
    int32_t scrollInvert;
    uint8_t _pad60[0x78 - 0x64];
    float cameraDistance;
    float orbitYaw;
    uint8_t _pad84[4];
    float pitch;
    float zoomSpeed;
    uint8_t _pad94[0xbc - 0x94];
    EntityLayout* player;
    uint8_t _padC0[0x1cc - 0xc0];
    float chatScrollA;
    float chatScrollB;
    uint8_t _pad1d4[0x23c - 0x1d4];
    char inWorldUi;
};
#pragma pack(pop)

inline float fixedToWorld(int32_t lo, int32_t hi) {
    const int64_t v = (static_cast<int64_t>(hi) << 32) | static_cast<uint32_t>(lo);
    return static_cast<float>(v) * decomp::kDat006fcd94;
}

inline void worldToFixed(float w, int32_t& lo, int32_t& hi) {
    const double scaled = w / decomp::kDat006fcd94;
    const int64_t v = static_cast<int64_t>(scaled);
    lo = static_cast<int32_t>(v);
    hi = static_cast<int32_t>(v >> 32);
}

inline void syncEntityFromState(EntityLayout& e, const EntityState& s) {
    worldToFixed(s.posX, e.posXFixed, e.posXHi);
    worldToFixed(s.posY, e.posYFixed, e.posYHi);
    worldToFixed(s.posZ, e.posZFixed, e.posZHi);
    e.velX = e.velY = 0.f;
    e.velZ = s.velZ;
    e.stateChar = s.stateChar;
    e.flags124 = static_cast<uint16_t>(s.moveFlags);
    e.lookX = s.lookOffsetX;
    e.lookY = s.lookOffsetY;
    e.lookZ = s.lookOffsetZ;
}

inline void syncStateFromEntity(EntityState& s, const EntityLayout& e) {
    s.posX = fixedToWorld(e.posXFixed, e.posXHi);
    s.posY = fixedToWorld(e.posYFixed, e.posYHi);
    s.posZ = fixedToWorld(e.posZFixed, e.posZHi);
    s.velZ = e.velZ;
    s.stateChar = e.stateChar;
    s.moveFlags = e.flags124;
    s.lookOffsetX = e.lookX;
    s.lookOffsetY = e.lookY;
    s.lookOffsetZ = e.lookZ;
}

} // namespace cube
