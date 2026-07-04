#include "cube/Terrain.hpp"

#include <algorithm>

namespace cube {

Field::Field() = default;

Field::~Field() = default;

void Field::resize(uint32_t width, uint32_t depth, uint32_t height) {
    width_ = width;
    depth_ = depth;
    height_ = height;
    cells_.assign(static_cast<size_t>(width) * depth, 0u);
}

uint32_t& Field::at(uint32_t x, uint32_t y) {
    return cells_.at(static_cast<size_t>(y) * width_ + x);
}

uint32_t Field::at(uint32_t x, uint32_t y) const {
    return cells_.at(static_cast<size_t>(y) * width_ + x);
}

Chunk::Chunk() : voxels_(static_cast<size_t>(kSize) * kSize * kSize, 0u) {}

Chunk::~Chunk() = default;

void Chunk::clear() {
    std::fill(voxels_.begin(), voxels_.end(), 0u);
    empty_ = true;
}

uint32_t& Chunk::voxel(int x, int y, int z) {
    empty_ = false;
    return voxels_[static_cast<size_t>(z) * kSize * kSize + static_cast<size_t>(y) * kSize +
                  static_cast<size_t>(x)];
}

uint32_t Chunk::voxel(int x, int y, int z) const {
    return voxels_[static_cast<size_t>(z) * kSize * kSize + static_cast<size_t>(y) * kSize +
                  static_cast<size_t>(x)];
}

ChunkBuffer::ChunkBuffer() = default;
ChunkBuffer::~ChunkBuffer() = default;

Chunk* ChunkBuffer::acquire(int cx, int cy) {
    for (auto& e : entries_) {
        if (e.cx == cx && e.cy == cy) {
            return e.chunk.get();
        }
    }
    Entry entry;
    entry.cx = cx;
    entry.cy = cy;
    entry.chunk = std::make_unique<Chunk>();
    Chunk* ptr = entry.chunk.get();
    entries_.push_back(std::move(entry));
    return ptr;
}

const Chunk* ChunkBuffer::find(int cx, int cy) const {
    for (const auto& e : entries_) {
        if (e.cx == cx && e.cy == cy) {
            return e.chunk.get();
        }
    }
    return nullptr;
}

void ChunkBuffer::release(int cx, int cy) {
    entries_.erase(
        std::remove_if(entries_.begin(), entries_.end(),
                       [cx, cy](const Entry& e) { return e.cx == cx && e.cy == cy; }),
        entries_.end());
}

Dungeon::Dungeon(int x, int y, int z) : x_(x), y_(y), z_(z) {}

House::House(int x, int y, int z) : x_(x), y_(y), z_(z) {}

} // namespace cube
