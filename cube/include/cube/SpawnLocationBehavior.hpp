#pragma once

#include "cube/Constants.hpp"
#include "cube/Field.hpp"
#include "cube/Types.hpp"

#include <cstdint>
#include <vector>

namespace cube {

// cube::SpawnLocationBehavior — world spawn / tile context for AI behaviors.
class SpawnLocationBehavior {
public:
    SpawnLocationBehavior();
    ~SpawnLocationBehavior();

    void resetPathState(EntityState& entity);
    uint32_t tileAt(uint32_t x, uint32_t y) const;

    Field& field() { return field_; }
    const Field& field() const { return field_; }

    void setOrigin(float x, float y, float z);
    float originX() const { return originX_; }
    float originY() const { return originY_; }
    float originZ() const { return originZ_; }

private:
    Field field_;
    float originX_{0.f};
    float originY_{0.f};
    float originZ_{0.f};
};

} // namespace cube
