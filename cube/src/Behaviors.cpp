#include "cube/Behavior.hpp"
#include "cube/LookAtPlayerBehavior.hpp"
#include "cube/WalkPathBehavior.hpp"
#include "cube/AIBehaviors.hpp"
#include "cube/SpawnLocationBehavior.hpp"
#include "cube/DecompiledConstants.hpp"
#include "cube/DecompiledLayouts.hpp"

#include <cmath>
#include <cstdlib>
#include <limits>

namespace cube {

SequentialBehavior::SequentialBehavior(std::vector<std::unique_ptr<Behavior>> steps)
    : steps_(std::move(steps)) {}

void SequentialBehavior::addStep(std::unique_ptr<Behavior> step) {
    steps_.push_back(std::move(step));
}

bool SequentialBehavior::execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) {
    if (index_ >= steps_.size()) {
        return false;
    }
    if (steps_[index_]->execute(entity, context, deltaMs)) {
        return true;
    }
    ++index_;
    return index_ < steps_.size();
}

std::unique_ptr<Behavior> SequentialBehavior::clone() const {
    std::vector<std::unique_ptr<Behavior>> copy;
    copy.reserve(steps_.size());
    for (const auto& step : steps_) {
        copy.push_back(step->clone());
    }
    return std::make_unique<SequentialBehavior>(std::move(copy));
}

// LookAtPlayerBehavior::vfunction1 @ 004c8140
bool LookAtPlayerBehavior::execute(EntityState& entity, SpawnLocationBehavior* context,
                                   int /*deltaMs*/) {
    entity.moveFlags &= ~0x4u;

    if (!targets_ || entity.stateChar == 'S' || entity.stateChar == 'T') {
        return false;
    }

    EntityLayout self{};
    syncEntityFromState(self, entity);

    EntityState* best = nullptr;
    float bestDist = std::numeric_limits<float>::max();

    for (EntityState* target : *targets_) {
        if (!target || target->lookOffsetZ < 0.f) {
            continue;
        }
        EntityLayout other{};
        syncEntityFromState(other, *target);

        const float dx = fixedToWorld(other.posXFixed - self.posXFixed, other.posXHi - self.posXHi);
        const float dy = fixedToWorld(other.posYFixed - self.posYFixed, other.posYHi - self.posYHi);
        const float dz = fixedToWorld(other.posZFixed - self.posZFixed, other.posZHi - self.posZHi);

        const float dist = dx * dx + dy * dy + dz * dz;
        if (dist < bestDist) {
            bestDist = dist;
            best = target;
        }
    }

    if (!best) {
        return false;
    }

    entity.moveFlags |= 0x4u;
    EntityLayout bestE{};
    syncEntityFromState(bestE, *best);

    entity.lookOffsetX = best->posX - entity.posX;
    entity.lookOffsetY = best->posY - entity.posY;
    entity.lookOffsetZ = best->posZ - entity.posZ;
    entity.resetVelocity();
    if (context) {
        context->resetPathState(entity);
    }
    return true;
}

std::unique_ptr<Behavior> LookAtPlayerBehavior::clone() const {
    auto copy = std::make_unique<LookAtPlayerBehavior>();
    copy->setTargets(targets_);
    return copy;
}

WalkPathBehavior::WalkPathBehavior(float speed) : speed_(speed) {}

WalkPathBehavior::WalkPathBehavior(std::vector<PathWaypoint> waypoints, float speed)
    : waypoints_(std::move(waypoints)), speed_(speed) {}

void WalkPathBehavior::setWaypoints(std::vector<PathWaypoint> waypoints) {
    waypoints_ = std::move(waypoints);
    index_ = 0;
    waitMs_ = 0;
}

// WalkPathBehavior::vfunction1 @ 0058d050 (simplified path follower; full nav needs SpawnLocationBehavior tile queries).
bool WalkPathBehavior::execute(EntityState& entity, SpawnLocationBehavior* /*context*/,
                               int deltaMs) {
    if (waypoints_.empty()) {
        return false;
    }
    if (waitMs_ > 0) {
        waitMs_ -= deltaMs;
        return waitMs_ > 0;
    }

    const PathWaypoint& wp = waypoints_[index_];
    const float dx = static_cast<float>(wp.x) - entity.posX;
    const float dy = static_cast<float>(wp.y) - entity.posY;
    const float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 0.5f) {
        index_ = (index_ + 1) % waypoints_.size();
        waitMs_ = 2000 + (std::rand() % 4000);
        return true;
    }

    const float step = speed_ * (deltaMs * 0.001f);
    entity.posX += (dx / dist) * step;
    entity.posY += (dy / dist) * step;
    entity.facingYaw = std::atan2(dx, dy);
    entity.moving = true;
    entity.action = ActionId::Walk;
    entity.resetVelocity();
    return true;
}

std::unique_ptr<Behavior> WalkPathBehavior::clone() const {
    return std::make_unique<WalkPathBehavior>(waypoints_, speed_);
}

bool RandomWalkBehavior::execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) {
    if (waitMs_ > 0) {
        waitMs_ -= deltaMs;
        return waitMs_ > 0;
    }
    if (!hasTarget_) {
        targetX_ = context->originX() + (static_cast<float>(std::rand()) / RAND_MAX * 2.f - 1.f) *
                                           radius_;
        targetY_ = context->originY() + (static_cast<float>(std::rand()) / RAND_MAX * 2.f - 1.f) *
                                           radius_;
        hasTarget_ = true;
    }
    const float dx = targetX_ - entity.posX;
    const float dy = targetY_ - entity.posY;
    const float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 1.f) {
        hasTarget_ = false;
        waitMs_ = 3000 + (std::rand() % 5000);
        entity.moving = false;
        entity.action = ActionId::Idle;
        return true;
    }
    const float step = decomp::kWalkSpeed * (deltaMs * 0.001f);
    entity.posX += (dx / dist) * step;
    entity.posY += (dy / dist) * step;
    entity.facingYaw = std::atan2(dx, dy);
    entity.moving = true;
    entity.action = ActionId::Walk;
    return true;
}

std::unique_ptr<Behavior> RandomWalkBehavior::clone() const {
    return std::make_unique<RandomWalkBehavior>(radius_);
}

RandomWalkBehavior::RandomWalkBehavior(float radius) : radius_(radius) {}

bool RandomInteractionBehavior::execute(EntityState& /*entity*/,
                                        SpawnLocationBehavior* /*context*/, int /*deltaMs*/) {
    return false;
}

std::unique_ptr<Behavior> RandomInteractionBehavior::clone() const {
    return std::make_unique<RandomInteractionBehavior>();
}

CompanionBehavior::CompanionBehavior(EntityState* followTarget) : followTarget_(followTarget) {}

bool CompanionBehavior::execute(EntityState& entity, SpawnLocationBehavior* /*context*/,
                                int deltaMs) {
    if (!followTarget_) {
        return false;
    }
    const float dx = followTarget_->posX - entity.posX;
    const float dy = followTarget_->posY - entity.posY;
    const float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 3.f) {
        entity.moving = false;
        entity.action = ActionId::Idle;
        return true;
    }
    const float step = decomp::kWalkSpeed * (deltaMs * 0.001f);
    entity.posX += (dx / dist) * step;
    entity.posY += (dy / dist) * step;
    entity.facingYaw = std::atan2(dx, dy);
    entity.moving = true;
    entity.action = ActionId::Walk;
    return true;
}

std::unique_ptr<Behavior> CompanionBehavior::clone() const {
    return std::make_unique<CompanionBehavior>(followTarget_);
}

bool CombatBehavior::execute(EntityState& entity, SpawnLocationBehavior* /*context*/,
                             int /*deltaMs*/) {
    (void)aggroRange_;
    entity.action = ActionId::Idle;
    return false;
}

std::unique_ptr<Behavior> CombatBehavior::clone() const {
    return std::make_unique<CombatBehavior>();
}

} // namespace cube
