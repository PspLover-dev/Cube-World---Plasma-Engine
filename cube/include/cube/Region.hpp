#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cube {

class Region;

struct RegionTileMeta {
    uint32_t field0{0};
    uint32_t field1{0};
    uint32_t field2{1};
    uint8_t field3{0};
    uint8_t pad[3]{};
};

struct SubRegion {
    std::array<uint32_t, 26> data{};
};

class Region {
public:
    static constexpr int kTileGrid = 64;
    static constexpr int kTileCount = kTileGrid * kTileGrid;
    static constexpr int kSubRegionCount = 64;
    static constexpr int kChunkSize = 64;

    Region();
    explicit Region(int id);
    ~Region();

    Region(const Region&) = delete;
    Region& operator=(const Region&) = delete;

    int id() const { return id_; }
    void setId(int id) { id_ = id; }

    void setName(std::string name) { name_ = std::move(name); }
    const std::string& name() const { return name_; }

    int widthScale() const { return widthScale_; }
    int heightScale() const { return heightScale_; }

    RegionTileMeta& tileMeta(int x, int y);
    const RegionTileMeta& tileMeta(int x, int y) const;

    SubRegion& subRegion(int index);
    const SubRegion& subRegion(int index) const;

    Region* childAt(int x, int y) const;
    void setChildAt(int x, int y, std::unique_ptr<Region> region);

    bool chunkOccupied(int chunkX, int chunkY) const;
    void setChunkOccupied(int chunkX, int chunkY, bool occupied);

private:
    int id_{0};
    int widthScale_{1};
    int heightScale_{1};
    int fieldC_{0};
    int field10_{0};
    std::string name_;
    std::array<RegionTileMeta, kTileCount> tiles_{};
    std::array<SubRegion, kSubRegionCount> subRegions_{};
    std::array<std::unique_ptr<Region>, kTileCount> children_{};
    std::array<uint32_t, kChunkSize * kChunkSize> chunkFlags_{};
    std::unordered_map<uint32_t, uint32_t> spawnMap_;
    uint32_t spawnSeed_{0};
};

} // namespace cube
