#include "cube/Region.hpp"

#include <stdexcept>

namespace cube {

namespace {

int clampIndex(int value, int max) {
    if (value < 0 || value >= max) {
        throw std::out_of_range("Region index out of range");
    }
    return value;
}

} // namespace

Region::Region() {
    for (auto& sub : subRegions_) {
        sub.data[9] = 1;
    }
    spawnSeed_ = static_cast<uint32_t>(id_);
}

Region::Region(int id) : id_(id) {
    for (auto& sub : subRegions_) {
        sub.data[9] = 1;
    }
    spawnSeed_ = static_cast<uint32_t>(id_);
}

Region::~Region() = default;

RegionTileMeta& Region::tileMeta(int x, int y) {
    return tiles_.at(static_cast<size_t>(clampIndex(y, kTileGrid)) * kTileGrid +
                     static_cast<size_t>(clampIndex(x, kTileGrid)));
}

const RegionTileMeta& Region::tileMeta(int x, int y) const {
    return tiles_.at(static_cast<size_t>(clampIndex(y, kTileGrid)) * kTileGrid +
                     static_cast<size_t>(clampIndex(x, kTileGrid)));
}

SubRegion& Region::subRegion(int index) {
    return subRegions_.at(static_cast<size_t>(clampIndex(index, kSubRegionCount)));
}

const SubRegion& Region::subRegion(int index) const {
    return subRegions_.at(static_cast<size_t>(clampIndex(index, kSubRegionCount)));
}

Region* Region::childAt(int x, int y) const {
    const size_t index =
        static_cast<size_t>(clampIndex(y, kTileGrid)) * kTileGrid + static_cast<size_t>(clampIndex(x, kTileGrid));
    return children_[index].get();
}

void Region::setChildAt(int x, int y, std::unique_ptr<Region> region) {
    const size_t index =
        static_cast<size_t>(clampIndex(y, kTileGrid)) * kTileGrid + static_cast<size_t>(clampIndex(x, kTileGrid));
    children_[index] = std::move(region);
}

bool Region::chunkOccupied(int chunkX, int chunkY) const {
    if (chunkX < 0 || chunkY < 0 || chunkX >= kChunkSize || chunkY >= kChunkSize) {
        return false;
    }
    return chunkFlags_[static_cast<size_t>(chunkY) * kChunkSize + static_cast<size_t>(chunkX)] != 0;
}

void Region::setChunkOccupied(int chunkX, int chunkY, bool occupied) {
    if (chunkX < 0 || chunkY < 0 || chunkX >= kChunkSize || chunkY >= kChunkSize) {
        return;
    }
    chunkFlags_[static_cast<size_t>(chunkY) * kChunkSize + static_cast<size_t>(chunkX)] = occupied ? 1u : 0u;
}

} // namespace cube
