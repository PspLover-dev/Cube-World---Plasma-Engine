#pragma once

#include "cube/Constants.hpp"

#include <cstdint>

namespace cube {

// cube::Controller — key state arrays (field_0x15 pressed, field_0x115 edge).
class Controller {
public:
    explicit Controller(void* owner = nullptr);
    virtual ~Controller() = default;

    void setKeyDown(int key);
    void setKeyUp(int key);
    void setKeyPressed(int key);
    void setKeyReleased(int key);
    bool keyHeld(int key) const;
    bool keyPressed(int key) const;
    void clearFrameKeys();

    void* owner() const { return owner_; }

protected:
    void* owner_{nullptr};
    uint8_t held_[kMaxKeySlots]{};
    uint8_t pressed_[kMaxKeySlots]{};
    uint8_t edgeDown_[kMaxKeySlots]{};
    uint8_t edgeUp_[kMaxKeySlots]{};
};

} // namespace cube
