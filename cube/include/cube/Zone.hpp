#pragma once

#include "cube/Field.hpp"
#include "cube/ZoneTile.hpp"

#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

namespace cube {

class Zone {
public:
    static constexpr int kGrid = 256;
    static constexpr int kFieldCount = kGrid * kGrid;
    static constexpr int kTileEntryCount = 0x400;

    Zone();
    ~Zone();

    Zone(const Zone&) = delete;
    Zone& operator=(const Zone&) = delete;

    Field& fieldAt(int x, int y);
    const Field& fieldAt(int x, int y) const;

    Field& fieldAtIndex(size_t index);
    const Field& fieldAtIndex(size_t index) const;

    ZoneTileEntry& tileEntry(int index);
    const ZoneTileEntry& tileEntry(int index) const;

    ZoneTileEntry& tileEntryAt(int chunkX, int chunkY);
    const ZoneTileEntry& tileEntryAt(int chunkX, int chunkY) const;

    bool flag70() const { return flag70_; }
    bool flag7c() const { return flag7c_; }
    std::mutex& lock() { return mutex_; }

private:
    std::vector<Field> fields_;
    std::array<ZoneTileEntry, kTileEntryCount> tileEntries_{};
    bool flag70_{true};
    bool flag7c_{true};
    std::mutex mutex_;
};

} // namespace cube
