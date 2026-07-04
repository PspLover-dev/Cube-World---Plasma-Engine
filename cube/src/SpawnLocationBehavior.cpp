#include "cube/SpawnLocationBehavior.hpp"

namespace cube {

SpawnLocationBehavior::SpawnLocationBehavior() = default;

SpawnLocationBehavior::~SpawnLocationBehavior() = default;

void SpawnLocationBehavior::resetPathState(EntityState& entity) {
    entity.resetVelocity();
    entity.lookOffsetX = 0.f;
    entity.lookOffsetY = 0.f;
}

uint32_t SpawnLocationBehavior::tileAt(uint32_t x, uint32_t y) const {
    if (x >= field_.width() || y >= field_.depth()) {
        return 0;
    }
    return field_.at(x, y);
}

void SpawnLocationBehavior::setOrigin(float x, float y, float z) {
    originX_ = x;
    originY_ = y;
    originZ_ = z;
}

} // namespace cube
