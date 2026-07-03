#include "cube/TextureCatalog.hpp"

#include "cube/Assets.hpp"
#include "cube/BlobDescramble.hpp"
#include "plasma/Engine.hpp"

#include "third_party/stb/stb_image.h"

namespace cube {

bool TextureCatalog::open(const std::string& data3Path) {
    decoded_.clear();
    gpu_.clear();
    owned_.clear();
    return db_.open(data3Path);
}

std::vector<std::string> TextureCatalog::keys() const {
    return db_.listKeys("%.png");
}

bool TextureCatalog::has(const std::string& key) const {
    std::vector<uint8_t> tmp;
    return db_.fetchBlob(key, tmp);
}

DecodedImage TextureCatalog::decodePng(const std::string& key) const {
    const auto it = decoded_.find(key);
    if (it != decoded_.end()) {
        return it->second;
    }

    DecodedImage img;
    std::vector<uint8_t> blob;
    if (!db_.fetchBlob(key, blob)) {
        return img;
    }
    descrambleBlob(blob);

    int w = 0;
    int h = 0;
    int comp = 0;
    unsigned char* pixels =
        stbi_load_from_memory(blob.data(), static_cast<int>(blob.size()), &w, &h, &comp, 4);
    if (!pixels) {
        return img;
    }
    img.width = w;
    img.height = h;
    img.rgba.assign(pixels, pixels + static_cast<size_t>(w) * h * 4);
    img.valid = true;
    stbi_image_free(pixels);
    decoded_[key] = img;
    return img;
}

plasma::Texture* TextureCatalog::gpuTexture(plasma::Engine& engine, const std::string& key) {
    const auto git = gpu_.find(key);
    if (git != gpu_.end()) {
        return git->second;
    }
    const DecodedImage img = decodePng(key);
    if (!img.valid) {
        return nullptr;
    }
    std::unique_ptr<plasma::Texture> tex(
        engine.createTexture(img.width, img.height, img.rgba.data(), true));
    plasma::Texture* ptr = tex.get();
    owned_.push_back(std::move(tex));
    gpu_[key] = ptr;
    return ptr;
}

} // namespace cube
