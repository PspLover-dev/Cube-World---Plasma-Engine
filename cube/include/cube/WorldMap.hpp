#pragma once

#include "cube/Sprite.hpp"
#include "cube/WorldInfo.hpp"

#include <vector>

namespace cube {

class WorldMap {
public:
    WorldMap() = default;
    WorldMap(const WorldInfo& info, int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    void setTile(int x, int y, int spriteId);
    int tile(int x, int y) const;

    SpriteManager& sprites() { return sprites_; }
    const WorldInfo& info() const { return info_; }

private:
    WorldInfo info_;
    SpriteManager sprites_;
    int width_{0};
    int height_{0};
    std::vector<int> tiles_;
};

} // namespace cube
