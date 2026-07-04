#include "cube/ActionConfig.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace cube {
namespace {

float floatAt(const uint8_t* cfg, size_t off) {
    float v{};
    std::memcpy(&v, cfg + off, 4);
    return v;
}

void writeFloat(uint8_t* p, float v) {
    std::memcpy(p, &v, sizeof(v));
}

void writeVec3(uint8_t* p, float x, float y, float z) {
    writeFloat(p, x);
    writeFloat(p + 4, y);
    writeFloat(p + 8, z);
}

void writeU16(uint8_t* p, uint16_t v) {
    std::memcpy(p, &v, sizeof(v));
}

void orU16(uint8_t* p, uint16_t bits) {
    uint16_t cur{};
    std::memcpy(&cur, p, 2);
    cur = static_cast<uint16_t>(cur | bits);
    std::memcpy(p, &cur, 2);
}

void applyScaleDivide(uint8_t* cfg, float divisor) {
    for (size_t off : {0x08u, 0x0cu, 0x10u}) {
        writeFloat(cfg + off, floatAt(cfg, off) / divisor);
    }
}

void applyScaleMultiply(uint8_t* cfg, float mul) {
    for (size_t off : {0x08u, 0x0cu, 0x10u}) {
        writeFloat(cfg + off, floatAt(cfg, off) * mul);
    }
}

} // namespace

// Port of FUN_0043f7c0 — action table from GlobalFunctions.cpp.
void configureActionRaw(uint32_t* actionIdOut, uint8_t* cfg, uint32_t* seed) {
    writeFloat(cfg + 0x08, decomp::kDefaultScaleXY);
    writeFloat(cfg + 0x0c, decomp::kDefaultScaleXY);
    writeFloat(cfg + 0x10, decomp::kDefaultScaleZ);

    if (seed == nullptr) {
        const int r0 = std::rand();
        const int r1 = std::rand();
        const int r2 = std::rand();
        writeU16(cfg + 0x02, static_cast<uint16_t>((r1 & 0xff) << 8 | (r2 & 0xff)));
        writeFloat(cfg + 0x04, static_cast<float>(r0 & 0xff));
        writeFloat(cfg + 0x20, static_cast<float>(std::rand() % 100));
        writeFloat(cfg + 0x24, static_cast<float>(std::rand() % 100));
    } else {
        switch (seed[0]) {
        case 0:
            *actionIdOut = ((seed[1] & 1u) + 2u);
            break;
        case 1:
            *actionIdOut = (seed[1] & 1u);
            break;
        case 2:
            *actionIdOut = ((seed[1] & 1u) + 9u);
            break;
        case 3:
            *actionIdOut = ((seed[1] & 1u) + 0xbu);
            break;
        case 4:
            *actionIdOut = ((seed[1] & 1u) + 4u);
            break;
        case 5:
            *actionIdOut = ((seed[1] & 1u) + 7u);
            break;
        case 6:
            *actionIdOut = ((seed[1] & 1u) + 0xfu);
            break;
        case 7:
            *actionIdOut = ((seed[1] & 1u) + 0xdu);
            break;
        default:
            break;
        }
    }

    switch (*actionIdOut) {
    case 0x31: // idle
        writeU16(cfg + 0x14, 0x10);
        writeU16(cfg + 0x18, 0x12);
        writeFloat(cfg + 0x70, 0.f);
        writeFloat(cfg + 0x74, 1.5f);  // 0x3fc00000
        writeFloat(cfg + 0x78, 4.f);   // 0x40800000
        writeFloat(cfg + 0x24, 1.2f);  // 0x3f99999a
        applyScaleDivide(cfg, decomp::kDat006fcdcc);
        break;

    case 0x32: // jump
        writeU16(cfg + 0x14, 0x14);
        writeU16(cfg + 0x18, 0x16);
        writeVec3(cfg + 0x70, 0.f, 3.5f, 2.f);
        writeVec3(cfg + 0x88, 3.5f, 1.f, 0.f);
        writeVec3(cfg + 0x64, 0.f, 0.f, 0.f);
        writeFloat(cfg + 0x6c, -4.f);  // 0xc0800000 impulse
        writeFloat(cfg + 0x24, 1.05f); // 0x3f866666
        writeFloat(cfg + 0x28, 0.8f);  // 0x3f4ccccd
        writeFloat(cfg + 0x08, 0.64f); // 0x3f23d70b
        writeFloat(cfg + 0x0c, 0.64f);
        writeFloat(cfg + 0x10, 1.36f); // 0x3fae147b
        orU16(cfg + 0x06, 0x28);
        return;

    case 0x57: // walk A
        writeU16(cfg + 0x14, 0x84);
        writeU16(cfg + 0x18, 0x86);
        writeVec3(cfg + 0x7c, 9.f, 0.f, 2.f);
        writeVec3(cfg + 0x88, 5.f, 1.f, -8.5f);
        writeVec3(cfg + 0x70, 0.f, 1.f, 2.f);
        writeVec3(cfg + 0x64, 0.f, 0.f, -6.f);
        writeFloat(cfg + 0x50, -30.f);
        writeFloat(cfg + 0x2c, 0.9f);
        orU16(cfg + 0x06, 0x28);
        applyScaleDivide(cfg, decomp::kDat006fcdcc);
        break;

    case 0x58: // walk B
        writeU16(cfg + 0x14, 0x88);
        writeU16(cfg + 0x18, 0x8a);
        writeVec3(cfg + 0x7c, 9.f, 0.f, 2.f);
        writeVec3(cfg + 0x88, 5.f, 1.f, -8.5f);
        writeVec3(cfg + 0x70, 0.f, 1.f, 3.f);
        writeVec3(cfg + 0x64, 0.f, 0.f, -5.f);
        writeFloat(cfg + 0x50, -30.f);
        writeFloat(cfg + 0x2c, 0.9f);
        orU16(cfg + 0x06, 0x28);
        applyScaleMultiply(cfg, decomp::kDat006fd780);
        break;

    default:
        applyScaleDivide(cfg, decomp::kDat006fcdcc);
        break;
    }
}

void ActionConfig::configure(uint32_t actionId, const ActionSeed* seed) {
    blob.fill(0);
    uint32_t id = actionId;
    configureActionRaw(&id, blob.data(),
                       seed ? reinterpret_cast<uint32_t*>(const_cast<ActionSeed*>(seed)) : nullptr);
}

AnimPose ActionConfig::toPose() const {
    AnimPose pose{};
    auto at = [this](size_t off) {
        float v{};
        std::memcpy(&v, blob.data() + off, 4);
        return v;
    };
    pose.scaleXY = at(0x08);
    pose.scaleZ = at(0x10);
    std::memcpy(&pose.body[0], blob.data() + 0x70, 12);
    std::memcpy(&pose.camera[0], blob.data() + 0x7c, 12);
    std::memcpy(&pose.torso[0], blob.data() + 0x88, 12);
    std::memcpy(&pose.feet[0], blob.data() + 0x64, 12);
    pose.playback = at(0x24);
    if (pose.playback <= 0.f) {
        pose.playback = at(0x2c);
    }
    return pose;
}

AnimPose poseForAction(ActionId action, bool walkAltPhase) {
    ActionConfig cfg;
    const uint32_t id = walkAltPhase && action == ActionId::Walk
                            ? static_cast<uint32_t>(ActionId::WalkRelease)
                            : static_cast<uint32_t>(action);
    cfg.configure(id, nullptr);
    return cfg.toPose();
}

} // namespace cube
