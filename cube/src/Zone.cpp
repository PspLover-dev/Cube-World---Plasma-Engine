#include "cube/Zone.hpp"

#include <stdexcept>

namespace cube {

Zone::Zone() : fields_(static_cast<size_t>(kFieldCount)) {}

Zone::~Zone() = default;

Field& Zone::fieldAt(int x, int y) {
    if (x < 0 || y < 0 || x >= kGrid || y >= kGrid) {
        throw std::out_of_range("Zone::fieldAt");
    }
    return fields_.at(static_cast<size_t>(y) * kGrid + static_cast<size_t>(x));
}

const Field& Zone::fieldAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= kGrid || y >= kGrid) {
        throw std::out_of_range("Zone::fieldAt");
    }
    return fields_.at(static_cast<size_t>(y) * kGrid + static_cast<size_t>(x));
}

Field& Zone::fieldAtIndex(size_t index) {
    return fields_.at(index);
}

const Field& Zone::fieldAtIndex(size_t index) const {
    return fields_.at(index);
}

ZoneTileEntry& Zone::tileEntry(int index) {
    if (index < 0 || index >= kTileEntryCount) {
        throw std::out_of_range("Zone::tileEntry");
    }
    return tileEntries_[static_cast<size_t>(index)];
}

const ZoneTileEntry& Zone::tileEntry(int index) const {
    if (index < 0 || index >= kTileEntryCount) {
        throw std::out_of_range("Zone::tileEntry");
    }
    return tileEntries_[static_cast<size_t>(index)];
}

ZoneTileEntry& Zone::tileEntryAt(int chunkX, int chunkY) {
    if (chunkX < 0 || chunkY < 0 || chunkX >= 0x20 || chunkY >= 0x20) {
        throw std::out_of_range("Zone::tileEntryAt");
    }
    return tileEntry(chunkY * 0x20 + chunkX);
}

const ZoneTileEntry& Zone::tileEntryAt(int chunkX, int chunkY) const {
    if (chunkX < 0 || chunkY < 0 || chunkX >= 0x20 || chunkY >= 0x20) {
        throw std::out_of_range("Zone::tileEntryAt");
    }
    return tileEntry(chunkY * 0x20 + chunkX);
}

} // namespace cube
