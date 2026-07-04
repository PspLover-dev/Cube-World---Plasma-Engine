#include "cube/Sprite.hpp"

namespace cube {

Sprite::Sprite(int id, std::string name) : id_(id), name_(std::move(name)) {}

Sprite* SpriteManager::add(int id, std::string name) {
    if (auto it = index_.find(id); it != index_.end()) {
        return &sprites_[it->second];
    }
    index_[id] = sprites_.size();
    sprites_.emplace_back(id, std::move(name));
    return &sprites_.back();
}

Sprite* SpriteManager::find(int id) {
    auto it = index_.find(id);
    return it != index_.end() ? &sprites_[it->second] : nullptr;
}

const Sprite* SpriteManager::find(int id) const {
    auto it = index_.find(id);
    return it != index_.end() ? &sprites_[it->second] : nullptr;
}

} // namespace cube
