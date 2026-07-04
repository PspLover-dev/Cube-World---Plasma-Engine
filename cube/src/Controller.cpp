#include "cube/Controller.hpp"

namespace cube {

Controller::Controller(void* owner) : owner_(owner) {}

void Controller::setKeyDown(int key) {
    if (key >= 0 && key < kMaxKeySlots) {
        held_[key] = 1;
    }
}

void Controller::setKeyUp(int key) {
    if (key >= 0 && key < kMaxKeySlots) {
        held_[key] = 0;
    }
}

void Controller::setKeyPressed(int key) {
    if (key >= 0 && key < kMaxKeySlots) {
        pressed_[key] = 1;
        edgeDown_[key] = 1;
    }
}

void Controller::setKeyReleased(int key) {
    if (key >= 0 && key < kMaxKeySlots) {
        pressed_[key] = 0;
        edgeUp_[key] = 1;
    }
}

bool Controller::keyHeld(int key) const {
    return key >= 0 && key < kMaxKeySlots && pressed_[key] != 0;
}

bool Controller::keyPressed(int key) const {
    return key >= 0 && key < kMaxKeySlots && edgeDown_[key] != 0;
}

void Controller::clearFrameKeys() {
    for (int i = 0; i < kMaxKeySlots; ++i) {
        edgeDown_[i] = 0;
        edgeUp_[i] = 0;
    }
}

} // namespace cube
