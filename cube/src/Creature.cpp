#include "cube/Creature.hpp"

namespace cube {

Creature::Creature() = default;

Creature::Creature(uint32_t id, uint32_t typeId) : id_(id), typeId_(typeId) {}

ActionId Creature::currentAnimAction() const {
    if (state_.action == ActionId::Walk) {
        return state_.walkAlt ? ActionId::WalkRelease : ActionId::Walk;
    }
    return state_.action;
}

AnimPose Creature::currentPose() const {
    return poseForAction(currentAnimAction(), state_.walkAlt);
}

} // namespace cube
