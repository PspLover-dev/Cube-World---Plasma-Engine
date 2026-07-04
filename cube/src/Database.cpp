#include "cube/Database.hpp"

namespace cube {

Database::Database() = default;

Database::~Database() {
    close();
}

bool Database::open(const std::string& path) {
    return db_.open(path);
}

void Database::close() {
    db_.close();
}

} // namespace cube
