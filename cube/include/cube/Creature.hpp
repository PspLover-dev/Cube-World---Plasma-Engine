#pragma once

#include "cube/Types.hpp"

#include <string>
#include <vector>

namespace cube {

// cube::Creature — player / NPC entity (subset of decompiled layout).
class Creature {
public:
    Creature();
    explicit Creature(uint32_t id, uint32_t typeId = 0);

    EntityState& state() { return state_; }
    const EntityState& state() const { return state_; }

    uint32_t id() const { return id_; }
    uint32_t typeId() const { return typeId_; }

    const std::string& name() const { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    ActionId currentAnimAction() const;
    AnimPose currentPose() const;

    bool isActive() const { return active_; }
    void setActive(bool v) { active_ = v; }

private:
    uint32_t id_{0};
    uint32_t typeId_{0};
    std::string name_;
    EntityState state_;
    bool active_{true};
};

} // namespace cube
