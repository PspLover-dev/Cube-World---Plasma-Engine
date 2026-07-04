#pragma once

#include "cube/Assets.hpp"

#include <string>

namespace cube {

// cube::Database — asset DB handle (data1.db / data2.db / data3.db).
class Database {
public:
    Database();
    ~Database();

    bool open(const std::string& path);
    void close();
    bool isOpen() const { return db_.isOpen(); }

    AssetDatabase& db() { return db_; }
    const AssetDatabase& db() const { return db_; }

private:
    AssetDatabase db_;
};

} // namespace cube
