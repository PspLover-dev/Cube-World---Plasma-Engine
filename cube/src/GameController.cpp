#include "cube/GameController.hpp"

#include "cube/ActionConfig.hpp"
#include "cube/DecompiledConstants.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>

namespace cube {

namespace {

uint32_t frameMillis() {
    using clock = std::chrono::steady_clock;
    static const clock::time_point start = clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start).count());
}

float smoothStep01(float t) {
    t = std::clamp(t, 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

void beginMove(EntityState& player) {
    player.moveFlags |= 1u;
    player.moving = true;
    player.action = ActionId::Walk;
    player.walkPhase = 0.f;
}

void endMove(EntityState& player) {
    player.moveFlags |= 2u;
    player.moving = false;
    player.action = ActionId::Idle;
}

} // namespace

GameController::GameController() : Controller(this) {
    fields_.cameraDistance = kCameraDistInit;
    fields_.orbitYaw = 0.f;
    fields_.pitch = 0.42f;
    fields_.zoomSpeed = 1.f;
    fields_.scrollDirection = 0;
    fields_.scrollInvert = 0;
    fields_.player = &entity_;
    lastFrameMs_ = frameMillis();
}

void GameController::syncEntityLayout() {
    if (!player_) {
        return;
    }
    syncEntityFromState(entity_, player_->state());
    fields_.cameraDistance = camera_.distance;
    fields_.orbitYaw = camera_.orbitYaw;
    fields_.pitch = camera_.pitch;
}

void GameController::onMouseInput(float deltaX, float deltaY) {
    // FUN_00652c10 — store previous/current mouse, notify focus widget on change.
    fields_.base.mousePrevX = fields_.base.mouseX;
    fields_.base.mousePrevY = fields_.base.mouseY;
    fields_.base.mouseX = deltaX;
    fields_.base.mouseY = deltaY;

    const float dx = fields_.base.mouseX - fields_.base.mousePrevX;
    const float dy = fields_.base.mouseY - fields_.base.mousePrevY;
    (void)dx;
    (void)dy;
    fields_.base.fieldC0 = fields_.base.fieldC4;
}

void GameController::vfunction10(int wheelDelta) {
    // GameController::vfunction10 @ 0047ef40
    if (fields_.inWorldUi == 0) {
        fields_.pitch -= static_cast<float>(wheelDelta * 2);
        if (fields_.pitch < 0.f) {
            fields_.pitch = 0.f;
        }
        if (fields_.pitch > decomp::kDat006fce34) {
            fields_.pitch = decomp::kDat006fce34;
        }
        camera_.pitch = fields_.pitch;
        return;
    }

    float speed = fields_.zoomSpeed;
    if (wheelDelta < 0) {
        speed *= decomp::kDat006fd31c;
    } else {
        speed /= decomp::kDat006fd31c;
    }
    fields_.zoomSpeed = speed;
    if (decomp::kDat00745e74 < speed) {
        fields_.zoomSpeed = decomp::kWalkSpeed;
    }
    if (fields_.zoomSpeed <= decomp::kDat006fcda0 &&
        decomp::kDat006fcda0 != fields_.zoomSpeed) {
        fields_.zoomSpeed = decomp::kDat006fcda0;
    }
}

void GameController::vfunction7(float mouseDeltaX, float mouseDeltaY) {
    // GameController::vfunction7 @ 0047ea00 (in-world branch simplified — no chat widgets).
    const uint32_t now = frameMillis();
    const int frameMs = static_cast<int>(now - lastFrameMs_);
    lastFrameMs_ = now;

    const float blend = smoothStep01(static_cast<float>(frameMs) * 0.01f);
    smoothedFps_ = (decomp::kDat00745dc0 - blend) * smoothedFps_ + blend * 60.f;
    (void)smoothedFps_;

    syncEntityLayout();
    onMouseInput(mouseDeltaX, mouseDeltaY);

    const bool menuOpen = vfunction2();
    GameController* self = this;

    if (!menuOpen) {
        // Branch: cVar4 == 0 — scroll zoom + orbit from mouse wheel / drag.
        fields_.cameraDistance +=
            static_cast<float>(fields_.scrollDirection) * decomp::kDat006fcd9c * mouseDeltaY;
        if (fields_.scrollInvert != 0) {
            fields_.cameraDistance -=
                static_cast<float>(fields_.scrollDirection) * decomp::kDat006fcd9c * mouseDeltaY;
        }
        if (fields_.cameraDistance > decomp::kDat006fce5c) {
            fields_.cameraDistance = decomp::kDat006fce5c;
        }
        if (fields_.cameraDistance <= 0.f && fields_.cameraDistance != 0.f) {
            fields_.cameraDistance = 0.f;
        }
        fields_.orbitYaw -= static_cast<float>(fields_.scrollDirection) * decomp::kDat006fcd9c *
                            mouseDeltaX;
    } else {
        if (fields_.base.activeFlag != 0) {
            const float wheelScale =
                (fields_.base.mouseY - fields_.base.mousePrevY) *
                static_cast<float>(fields_.scrollDirection) * decomp::kDat006fcd9c;
            if (fields_.scrollInvert == 0) {
                fields_.cameraDistance += wheelScale;
            } else {
                fields_.cameraDistance -= wheelScale;
            }
            if (fields_.cameraDistance > decomp::kDat006fce5c) {
                fields_.cameraDistance = decomp::kDat006fce5c;
            }
            if (fields_.cameraDistance <= 0.f && fields_.cameraDistance != 0.f) {
                fields_.cameraDistance = 0.f;
            }
            fields_.orbitYaw -= (fields_.base.mouseX - fields_.base.mousePrevX) *
                                static_cast<float>(fields_.scrollDirection) *
                                decomp::kDat006fcd9c;
        }

        // Look-at lag when RMB orbit active (+0x0 byte 6) — entity +0x16c / +0x164.
        if (fields_.base.flags0[6] != 0 && player_) {
            const float dx = fields_.base.mouseX - fields_.base.mousePrevX;
            const float dy = fields_.base.mouseY - fields_.base.mousePrevY;
            entity_.lookZ += dx * decomp::kDat00745da0;
            entity_.lookY -= dy * decomp::kDat00745da0;
            syncStateFromEntity(player_->state(), entity_);
        }
    }

    camera_.distance = fields_.cameraDistance;
    camera_.orbitYaw = fields_.orbitYaw;
    camera_.pitch = fields_.pitch;
    (void)self;
}

void GameController::updateAnimation(float dt, bool running) {
    if (!player_) {
        return;
    }
    EntityState& p = player_->state();
    if (p.action == ActionId::Jump) {
        if (p.onGround) {
            p.action = p.moving ? ActionId::Walk : ActionId::Idle;
        }
        return;
    }
    if (p.action == ActionId::Walk) {
        const float rate = (running ? 2.2f : 1.4f) * 0.9f;
        p.walkPhase += dt * rate;
        while (p.walkPhase >= kWalkCycleLen) {
            p.walkPhase -= kWalkCycleLen;
            p.walkAlt = !p.walkAlt;
        }
        return;
    }
    p.walkAlt = false;
    p.walkPhase = 0.f;
}

void GameController::updatePlayer(float dt, const InputState& input) {
    if (!player_) {
        return;
    }
    EntityState& p = player_->state();
    p.savePrevious();

    const bool wantsMove = (input.moveX != 0.f || input.moveY != 0.f);
    if (wantsMove && !p.moving) {
        beginMove(p);
    } else if (!wantsMove && p.moving) {
        endMove(p);
    }

    if (wantsMove) {
        float mx = input.moveX;
        float my = input.moveY;
        const float len = std::sqrt(mx * mx + my * my);
        mx /= len;
        my /= len;

        const float sinY = std::sin(camera_.orbitYaw);
        const float cosY = std::cos(camera_.orbitYaw);
        const float worldX = sinY * my + cosY * mx;
        const float worldY = cosY * my - sinY * mx;

        const float speed = input.running ? kRunSpeed : kWalkSpeed;
        p.posX += worldX * speed * dt;
        p.posY += worldY * speed * dt;

        if (std::abs(worldX) > 0.001f || std::abs(worldY) > 0.001f) {
            p.facingYaw = std::atan2(worldX, worldY);
        }
        if (p.action != ActionId::Jump) {
            p.action = ActionId::Walk;
        }
    }

    p.posX = std::clamp(p.posX, -kWorldHalf, kWorldHalf);
    p.posY = std::clamp(p.posY, -kWorldHalf, kWorldHalf);

    if (input.jumpRequested && p.onGround) {
        p.velZ = kJumpImpulse;
        p.onGround = false;
        p.action = ActionId::Jump;
    }

    if (!p.onGround) {
        p.velZ -= kGravity * dt;
        p.posZ += p.velZ * dt;
        if (p.posZ <= kGroundZ) {
            p.posZ = kGroundZ;
            p.velZ = 0.f;
            p.onGround = true;
        }
    } else {
        p.posZ = kGroundZ;
    }

    updateAnimation(dt, input.running);

    if (wantsMove) {
        p.lookOffsetX += p.deltaX() * decomp::kDat00745da0;
        p.lookOffsetY -= p.deltaY() * decomp::kDat00745da0;
    } else {
        p.lookOffsetX *= 0.9f;
        p.lookOffsetY *= 0.9f;
    }
    p.lookOffsetX = std::clamp(p.lookOffsetX, -kLookLagMax, kLookLagMax);
    p.lookOffsetY = std::clamp(p.lookOffsetY, -kLookLagMax, kLookLagMax);

    syncEntityLayout();
}

void GameController::update(float dt, const InputState& input) {
    if (input.scrollDelta != 0.f) {
        fields_.scrollDirection = input.scrollDelta > 0.f ? 0 : 1;
        vfunction10(static_cast<int>(input.scrollDelta * 120.f));
    }

    if (input.orbitDrag) {
        fields_.base.flags0[6] = 1;
        vfunction7(input.orbitYawDelta, input.orbitPitchDelta);
        camera_.pitch = std::clamp(camera_.pitch + input.orbitPitchDelta * decomp::kDat006fcd9c,
                                   0.12f, 1.05f);
        fields_.pitch = camera_.pitch;
    } else {
        fields_.base.flags0[6] = 0;
        vfunction7(0.f, 0.f);
    }

    updatePlayer(dt, input);
    clearFrameKeys();
}

} // namespace cube
