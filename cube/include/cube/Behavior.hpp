#pragma once

#include "cube/Types.hpp"

#include <memory>
#include <vector>

namespace cube {

class SpawnLocationBehavior;

// Shared behavior interface (vfunction1 / vfunction2 from decompile).
class Behavior {
public:
    virtual ~Behavior() = default;

    // Returns true when behavior wants to continue (low byte of decompiled return).
    virtual bool execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) = 0;
    virtual std::unique_ptr<Behavior> clone() const = 0;
};

class SequentialBehavior : public Behavior {
public:
    SequentialBehavior() = default;
    explicit SequentialBehavior(std::vector<std::unique_ptr<Behavior>> steps);

    bool execute(EntityState& entity, SpawnLocationBehavior* context, int deltaMs) override;
    std::unique_ptr<Behavior> clone() const override;

    void addStep(std::unique_ptr<Behavior> step);

private:
    std::vector<std::unique_ptr<Behavior>> steps_;
    size_t index_{0};
};

} // namespace cube
