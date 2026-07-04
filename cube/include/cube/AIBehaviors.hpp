#pragma once

#include "cube/Behavior.hpp"

namespace cube {

class SpawnLocationBehavior;

class RandomWalkBehavior : public Behavior {
public:
    RandomWalkBehavior() = default;
    explicit RandomWalkBehavior(float radius);

    bool execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) override;
    std::unique_ptr<Behavior> clone() const override;

private:
    float radius_{20.f};
    int waitMs_{0};
    float targetX_{0.f};
    float targetY_{0.f};
    bool hasTarget_{false};
};

class RandomInteractionBehavior : public Behavior {
public:
    RandomInteractionBehavior() = default;

    bool execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) override;
    std::unique_ptr<Behavior> clone() const override;
};

class CompanionBehavior : public Behavior {
public:
    CompanionBehavior() = default;
    explicit CompanionBehavior(EntityState* followTarget);

    bool execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) override;
    std::unique_ptr<Behavior> clone() const override;

    void setFollowTarget(EntityState* target) { followTarget_ = target; }

private:
    EntityState* followTarget_{nullptr};
};

class CombatBehavior : public Behavior {
public:
    CombatBehavior() = default;

    bool execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) override;
    std::unique_ptr<Behavior> clone() const override;

    void setAggroRange(float range) { aggroRange_ = range; }

private:
    float aggroRange_{15.f};
};

} // namespace cube
