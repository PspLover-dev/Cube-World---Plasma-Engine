#pragma once

#include "cube/Behavior.hpp"

#include <vector>

namespace cube {

class SpawnLocationBehavior;

// cube::LookAtPlayerBehavior — vfunction1 finds nearest entity, sets +0x160/+0x164/+0x168.
class LookAtPlayerBehavior : public Behavior {
public:
    LookAtPlayerBehavior() = default;

    bool execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) override;
    std::unique_ptr<Behavior> clone() const override;

    void setTargets(const std::vector<EntityState*>* targets) { targets_ = targets; }

private:
    const std::vector<EntityState*>* targets_{nullptr};
};

} // namespace cube
