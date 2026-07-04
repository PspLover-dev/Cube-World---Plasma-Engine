#pragma once

#include "cube/Constants.hpp"
#include "plasma/Math.hpp"

#include <algorithm>
#include <cstdint>

namespace cube {

enum class ActionId : uint8_t {
    Idle = 0x31,
    Jump = 0x32,
    Walk = 0x57,
    WalkRelease = 0x58,
};

enum class PartKind { Body, Foot, Head, Hand };

struct AnimPose {
    plasma::Vec3 body{};
    plasma::Vec3 camera{};
    plasma::Vec3 torso{};
    plasma::Vec3 feet{};
    float playback{1.f};
    float scaleXY{1.f};
    float scaleZ{1.f};
};

// FUN_0043f7c0 action configs — see ActionConfig.cpp.
AnimPose poseForAction(ActionId action, bool walkAltPhase);
plasma::Vec3 partOffset(PartKind kind, const AnimPose& pose);

struct EntityState {
    float posX{0.f};
    float posY{0.f};
    float posZ{0.f};
    float prevX{0.f};
    float prevY{0.f};
    float prevZ{0.f};
    float velZ{0.f};
    float facingYaw{0.f};
    float lookOffsetX{0.f}; // entity +0x160
    float lookOffsetY{0.f}; // entity +0x164
    float lookOffsetZ{1.f}; // entity +0x168 (camera distance scale)
    bool onGround{true};
    bool moving{false};
    uint32_t moveFlags{0};
    char stateChar{0}; // +0x68 — 'S'/'T' skips LookAtPlayerBehavior

    ActionId action{ActionId::Idle};
    float walkPhase{0.f};
    bool walkAlt{false};

    void savePrevious();
    float deltaX() const { return posX - prevX; }
    float deltaY() const { return posY - prevY; }
    void resetVelocity();
    plasma::Vec3 position() const { return {posX, posY, posZ}; }
};

struct CameraState {
    float distance{kCameraDistInit}; // GameController field_0x78
    float orbitYaw{0.f};             // GameController field_0x80
    float pitch{0.42f};
};

struct PathWaypoint {
    double x{0.};
    double y{0.};
    double z{0.};
};

} // namespace cube
