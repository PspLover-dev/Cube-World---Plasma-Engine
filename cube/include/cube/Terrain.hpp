#pragma once

#include "cube/Field.hpp"
#include "cube/Region.hpp"
#include "cube/Zone.hpp"
#include "cube/ZoneTile.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace cube {

class Chunk {
public:
    Chunk();
    ~Chunk();

    static constexpr int kSize = 32;

    void clear();
    bool empty() const { return empty_; }
    void setEmpty(bool v) { empty_ = v; }

    uint32_t& voxel(int x, int y, int z);
    uint32_t voxel(int x, int y, int z) const;

private:
    bool empty_{true};
    std::vector<uint32_t> voxels_;
};

// cube::ChunkBuffer — streaming buffer for loaded chunks.
class ChunkBuffer {
public:
    ChunkBuffer();
    ~ChunkBuffer();

    Chunk* acquire(int cx, int cy);
    const Chunk* find(int cx, int cy) const;
    void release(int cx, int cy);

private:
    struct Entry {
        int cx{0};
        int cy{0};
        std::unique_ptr<Chunk> chunk;
    };
    std::vector<Entry> entries_;
};

class Dungeon {
public:
    Dungeon() = default;
    Dungeon(int x, int y, int z);

    int x() const { return x_; }
    int y() const { return y_; }
    int z() const { return z_; }

private:
    int x_{0};
    int y_{0};
    int z_{0};
};

class House {
public:
    House() = default;
    House(int x, int y, int z);

    int x() const { return x_; }
    int y() const { return y_; }
    int z() const { return z_; }

private:
    int x_{0};
    int y_{0};
    int z_{0};
};

class LandscapeTile {
public:
    LandscapeTile() = default;

    int biomeId{0};
    float height{0.f};
};

} // namespace cube
