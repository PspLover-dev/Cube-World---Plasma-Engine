#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cube {

class World;
class Zone;

struct SpawnSearchResult {
    int zoneX{0};
    int zoneY{0};
    int distanceSq{0x90000};
    bool found{false};
};

// cube::WorldInfo @ 00466a70 — world metadata + callback vector storage.
class WorldInfo {
public:
    WorldInfo();
    ~WorldInfo();

    WorldInfo(const WorldInfo&) = delete;
    WorldInfo& operator=(const WorldInfo&) = delete;

    void setSeed(uint32_t seed) { seed_ = seed; }
    uint32_t seed() const { return seed_; }

    void setName(std::string name) { name_ = std::move(name); }
    const std::string& name() const { return name_; }

    // FUN_0063da20 — reallocate callback pointer vector.
    void resize(size_t capacity);

    const std::vector<uint32_t>& callbacks() const { return callbacks_; }
    std::vector<uint32_t>& callbacks() { return callbacks_; }

    void pushCallback(uint32_t value);

    // Port of WorldInfo_Constructor_or_Destructor @ 0046a8a0 (spawn search path).
    SpawnSearchResult findNearestSpawn(const Zone& zone, int targetX, int targetY,
                                       int searchRadius = 3) const;

    // Periodic world-info refresh (tile iteration @ 0x400×0x40 chunks).
    void update(World* world);

private:
    std::vector<uint32_t> callbacks_;
    std::string name_;
    uint32_t seed_{0};
    uint32_t lastUpdateMs_{0};
};

} // namespace cube
