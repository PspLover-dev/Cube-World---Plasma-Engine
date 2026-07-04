#pragma once

#include "plasma/Font.hpp"

#include <string>

namespace cube::fonts {

inline bool init(const std::string& gameRoot) {
    return plasma::initRetailFonts(gameRoot);
}

inline std::string resource1Path(const std::string& gameRoot) {
    return gameRoot + "/resource1.dat";
}

inline std::string resource2Path(const std::string& gameRoot) {
    return gameRoot + "/resource2.dat";
}

} // namespace cube::fonts
