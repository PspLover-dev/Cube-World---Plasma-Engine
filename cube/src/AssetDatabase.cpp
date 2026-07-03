#include "cube/Assets.hpp"

#include "../../third_party/sqlite3/sqlite3.h"

#include <cstring>

namespace cube {

bool AssetDatabase::open(const std::string& path) {
    close();
    sqlite3* raw = nullptr;
    if (sqlite3_open(path.c_str(), &raw) != SQLITE_OK) {
        if (raw) {
            sqlite3_close(raw);
        }
        return false;
    }
    db_ = raw;
    path_ = path;
    return true;
}

void AssetDatabase::close() {
    if (db_) {
        sqlite3_close(static_cast<sqlite3*>(db_));
        db_ = nullptr;
    }
    path_.clear();
}

bool AssetDatabase::fetchBlob(const std::string& key, std::vector<uint8_t>& out) const {
    out.clear();
    auto* db = static_cast<sqlite3*>(db_);
    if (!db) {
        return false;
    }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT value FROM blobs WHERE key = ?", -1, &stmt, nullptr) !=
        SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    const int step = sqlite3_step(stmt);
    if (step != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }
    const void* blob = sqlite3_column_blob(stmt, 0);
    const int bytes = sqlite3_column_bytes(stmt, 0);
    if (!blob || bytes <= 0) {
        sqlite3_finalize(stmt);
        return false;
    }
    out.resize(static_cast<size_t>(bytes));
    std::memcpy(out.data(), blob, static_cast<size_t>(bytes));
    sqlite3_finalize(stmt);
    return true;
}

std::vector<std::string> AssetDatabase::listKeys(const std::string& likePattern) const {
    std::vector<std::string> keys;
    auto* db = static_cast<sqlite3*>(db_);
    if (!db) {
        return keys;
    }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT key FROM blobs WHERE key LIKE ? ORDER BY key", -1, &stmt,
                           nullptr) != SQLITE_OK) {
        return keys;
    }
    sqlite3_bind_text(stmt, 1, likePattern.c_str(), static_cast<int>(likePattern.size()),
                      SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (key) {
            keys.emplace_back(key);
        }
    }
    sqlite3_finalize(stmt);
    return keys;
}

bool ModelCatalog::open(const std::string& data1Path) {
    names_.clear();
    if (!db_.open(data1Path)) {
        return false;
    }
    names_ = db_.listKeys("%.cub");
    return !names_.empty();
}

std::vector<std::string> ModelCatalog::modelNames() const {
    return names_;
}

bool ModelCatalog::loadModel(const std::string& name, CubModel& out) const {
    std::vector<uint8_t> blob;
    if (!db_.fetchBlob(name, blob)) {
        return false;
    }
    return out.loadFromBlob(blob, true);
}

CubMesh ModelCatalog::buildMesh(const CubModel& model, float voxelSize) const {
    return CubMeshBuilder::build(model, voxelSize);
}

} // namespace cube
