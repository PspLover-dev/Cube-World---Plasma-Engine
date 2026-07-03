#pragma once

#include "cube/Assets.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace plasma {
class Engine;
class Texture;
} // namespace plasma

namespace cube {

struct DecodedImage {
    int width{0};
    int height{0};
    std::vector<uint8_t> rgba;
    bool valid{false};
};

class TextureCatalog {
public:
    bool open(const std::string& data3Path);
    DecodedImage decodePng(const std::string& key) const;
    plasma::Texture* gpuTexture(plasma::Engine& engine, const std::string& key);

    std::vector<std::string> keys() const;
    bool has(const std::string& key) const;

private:
    AssetDatabase db_;
    mutable std::unordered_map<std::string, DecodedImage> decoded_;
    mutable std::unordered_map<std::string, plasma::Texture*> gpu_;
    mutable std::vector<std::unique_ptr<plasma::Texture>> owned_;
};

} // namespace cube
