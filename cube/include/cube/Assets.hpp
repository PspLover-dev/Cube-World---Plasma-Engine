#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "cube/BlobDescramble.hpp"

namespace cube {

struct CubModel {
    int width{0};
    int depth{0};
    int height{0};
    std::vector<uint8_t> rgb;

    bool loadFromBlob(const std::vector<uint8_t>& blob, bool descramble = true);
    bool inBounds(int x, int y, int z) const;
    const uint8_t* voxelRgb(int x, int y, int z) const;
    bool isVoxelEmpty(int x, int y, int z) const;
};

struct CubMeshVertex {
    float px, py, pz;
    float nx, ny, nz;
    uint8_t r, g, b;
};

struct CubMesh {
    std::vector<CubMeshVertex> vertices;
    std::vector<uint32_t> indices;
};

class CubMeshBuilder {
public:
    static CubMesh build(const CubModel& model, float voxelSize = 1.f);
};

class AssetDatabase {
public:
    bool open(const std::string& path);
    void close();
    bool isOpen() const { return db_ != nullptr; }
    bool fetchBlob(const std::string& key, std::vector<uint8_t>& out) const;
    std::vector<std::string> listKeys(const std::string& likePattern = "%") const;

private:
    void* db_{nullptr};
    std::string path_;
};

class ModelCatalog {
public:
    bool open(const std::string& data1Path);
    std::vector<std::string> modelNames() const;
    bool loadModel(const std::string& name, CubModel& out) const;
    CubMesh buildMesh(const CubModel& model, float voxelSize = 1.f) const;

private:
    AssetDatabase db_;
    mutable std::vector<std::string> names_;
};

} // namespace cube
