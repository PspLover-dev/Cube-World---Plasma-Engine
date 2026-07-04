#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace cube {

// cube::Field — voxel column storage for a zone tile.
class Field {
public:
    Field();
    ~Field();

    Field(const Field&) = delete;
    Field& operator=(const Field&) = delete;

    void resize(uint32_t width, uint32_t depth, uint32_t height);
    uint32_t width() const { return width_; }
    uint32_t depth() const { return depth_; }
    uint32_t height() const { return height_; }

    uint32_t& at(uint32_t x, uint32_t y);
    uint32_t at(uint32_t x, uint32_t y) const;

private:
    uint32_t width_{0};
    uint32_t depth_{0};
    uint32_t height_{0};
    std::vector<uint32_t> cells_;
};

} // namespace cube
