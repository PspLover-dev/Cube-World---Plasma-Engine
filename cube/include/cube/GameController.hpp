#pragma once

#include "cube/Controller.hpp"
#include "cube/Creature.hpp"
#include "cube/DecompiledLayouts.hpp"

namespace cube {

struct InputState {
    float moveX{0.f};
    float moveY{0.f};
    bool running{false};
    bool jumpRequested{false};
    float scrollDelta{0.f};
    float orbitYawDelta{0.f};
    float orbitPitchDelta{0.f};
    bool orbitDrag{false};
};

// cube::GameController — retail layout + vfunction7 / FUN_00652c10.
class GameController : public Controller {
public:
    GameController();
    ~GameController() override = default;

    void setPlayer(Creature* player) { player_ = player; }
    Creature* player() const { return player_; }

    GameControllerFields& fields() { return fields_; }
    const GameControllerFields& fields() const { return fields_; }

    CameraState& camera() { return camera_; }
    const CameraState& camera() const { return camera_; }

    // GameController::vfunction7
    void vfunction7(float mouseDeltaX, float mouseDeltaY);

    // GameController::vfunction10 — scroll wheel (param_1 = delta * 120).
    void vfunction10(int wheelDelta);

    // GameController::FUN_00652c10 — mouse input to focused widget.
    void onMouseInput(float deltaX, float deltaY);

    void update(float dt, const InputState& input);

    bool vfunction2() const { return fields_.inWorldUi != 0; }

private:
    void updatePlayer(float dt, const InputState& input);
    void updateAnimation(float dt, bool running);
    void syncEntityLayout();

    Creature* player_{nullptr};
    EntityLayout entity_{};
    GameControllerFields fields_{};
    CameraState camera_{};
    uint32_t lastFrameMs_{0};
    float smoothedFps_{60.f};
};

} // namespace cube
