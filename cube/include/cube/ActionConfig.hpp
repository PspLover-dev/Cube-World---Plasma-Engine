#pragma once

#include "cube/DecompiledConstants.hpp"
#include "cube/Types.hpp"

#include <array>
#include <cstdint>
#include <cstring>

namespace cube {

// FUN_0043f7c0 param_3 optional random seed block.
struct ActionSeed {
    uint32_t mode{0};
    uint8_t b0{0};
    uint8_t b1{0};
    uint8_t b2{0};
    uint8_t pad{0};
    int32_t v2{0};
    int32_t v3{0};
};

// FUN_0043f7c0 param_2 action-configuration blob (retail layout, >= 0xA8 bytes used).
struct ActionConfig {
    static constexpr size_t kBlobSize = 0xB0;

    std::array<uint8_t, kBlobSize> blob{};

    void configure(uint32_t actionId, const ActionSeed* seed = nullptr);
    AnimPose toPose() const;
};

// Raw port of FUN_0043f7c0 — see cube/src/ActionConfig.cpp.
void configureActionRaw(uint32_t* actionIdOut, uint8_t* cfg, uint32_t* seed);

} // namespace cube
