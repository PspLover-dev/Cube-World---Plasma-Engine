#pragma once

#include "cube/Behavior.hpp"
#include "cube/Types.hpp"

#include <vector>

namespace cube {

class SpawnLocationBehavior;

// cube::WalkPathBehavior — follow waypoint path at configured speed.
class WalkPathBehavior : public Behavior {
public:
    WalkPathBehavior() = default;
    explicit WalkPathBehavior(float speed);
    WalkPathBehavior(std::vector<PathWaypoint> waypoints, float speed);

    bool execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) override;
    std::unique_ptr<Behavior> clone() const override;

    void setWaypoints(std::vector<PathWaypoint> waypoints);
    void setSpeed(float speed) { speed_ = speed; }
    float speed() const { return speed_; }

private:
    std::vector<PathWaypoint> waypoints_;
    size_t index_{0};
    int waitMs_{0};
    float speed_{kWalkSpeed};
};

} // namespace cube
