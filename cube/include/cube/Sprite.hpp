#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace cube {

class Sprite {
public:
    Sprite() = default;
    Sprite(int id, std::string name);

    int id() const { return id_; }
    const std::string& name() const { return name_; }

    void setFrameCount(int count) { frameCount_ = count; }
    int frameCount() const { return frameCount_; }

private:
    int id_{0};
    int frameCount_{1};
    std::string name_;
};

class SpriteManager {
public:
    SpriteManager() = default;

    Sprite* add(int id, std::string name);
    Sprite* find(int id);
    const Sprite* find(int id) const;

private:
    std::vector<Sprite> sprites_;
    std::unordered_map<int, size_t> index_;
};

} // namespace cube
