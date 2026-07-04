#include "cube/WorldInfo.hpp"

#include "cube/World.hpp"

#include "plasma/Math.hpp"
#include "cube/Zone.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>

namespace cube {

namespace {

uint32_t nowMs() {
    using clock = std::chrono::steady_clock;
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(clock::now().time_since_epoch()).count());
}

} // namespace

WorldInfo::WorldInfo() {
    name_.reserve(0xf);
}

WorldInfo::~WorldInfo() = default;

void WorldInfo::resize(size_t capacity) {
    if (capacity >= 0x40000000) {
        throw std::bad_alloc();
    }
    callbacks_.reserve(capacity);
    if (callbacks_.size() > capacity) {
        callbacks_.resize(capacity);
    }
}

void WorldInfo::pushCallback(uint32_t value) {
    if (callbacks_.size() == callbacks_.capacity()) {
        const size_t next = callbacks_.empty() ? 1 : callbacks_.size() + callbacks_.size() / 2;
        resize(std::max(next, callbacks_.size() + 1));
    }
    callbacks_.push_back(value);
}

SpawnSearchResult WorldInfo::findNearestSpawn(const Zone& zone, int targetX, int targetY,
                                              int searchRadius) const {
    SpawnSearchResult result;
    const int minX = std::max(0, targetX - searchRadius);
    const int maxX = std::min(Zone::kGrid - 1, targetX + searchRadius);
    const int minY = std::max(0, targetY - searchRadius);
    const int maxY = std::min(Zone::kGrid - 1, targetY + searchRadius);

    const int targetWorldX = targetX * 0x100 + 0x80;
    const int targetWorldY = targetY * 0x100 + 0x80;

    for (int y = minY; y <= maxY; ++y) {
        const int worldY = y * 0x100 + 0x80;
        for (int x = minX; x <= maxX; ++x) {
            const int worldX = x * 0x100 + 0x80;
            const int distSq = (targetWorldX - worldX) * (targetWorldX - worldX) +
                               (targetWorldY - worldY) * (targetWorldY - worldY);
            if (distSq >= result.distanceSq) {
                continue;
            }

            const int chunkX = x >> 6;
            const int chunkY = y >> 6;
            if (chunkX < 0 || chunkY < 0 || chunkX >= 0x20 || chunkY >= 0x20) {
                continue;
            }

            const int localX = x & 0x3f;
            const int localY = y & 0x3f;
            const Field& field = zone.fieldAt(x, y);
            if (field.width() == 0 && field.height() == 0) {
                continue;
            }

            (void)zone.tileEntryAt(chunkX, chunkY);
            (void)localX;
            (void)localY;

            result.zoneX = x;
            result.zoneY = y;
            result.distanceSq = distSq;
            result.found = true;
        }
    }

    return result;
}

void WorldInfo::update(World* world) {
    if (world == nullptr) {
        return;
    }

    const uint32_t now = nowMs();
    if (now - lastUpdateMs_ < 0x3e9) {
        return;
    }
    lastUpdateMs_ = now;

    Zone& zone = world->zone();
    for (int chunkY = 0; chunkY < 0x20; ++chunkY) {
        for (int chunkX = 0; chunkX < 0x20; ++chunkX) {
            (void)zone.tileEntryAt(chunkX, chunkY);
        }
    }

    if (Creature* player = world->player()) {
        const plasma::Vec3 pos = player->state().position();
        const int px = static_cast<int>(std::lround(pos[0] / 256.f));
        const int py = static_cast<int>(std::lround(pos[1] / 256.f));
        (void)findNearestSpawn(zone, px, py, 3);
    }
}

} // namespace cube
