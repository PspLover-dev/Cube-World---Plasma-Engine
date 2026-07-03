#include "cube/BlobDescramble.hpp"

namespace cube {

namespace {

constexpr int kOffsetLookupTable[44] = {
    0x1092, 0x254F, 0x348, 0x14B40, 0x241A, 0x2676, 0x7F, 0x9, 0x250B, 0x18A, 0x7B, 0x12E2,
    0x7EBC, 0x5F23, 0x981, 0x11, 0x85BA, 0x0A566, 0x1093, 0x0E, 0x2D266, 0x7C3, 0x0C16, 0x76D,
    0x15D41, 0x12CD, 0x25, 0x8F, 0x0DA2, 0x4C1B, 0x53F, 0x1B0, 0x14AFC, 0x23E0, 0x258C, 0x4D1,
    0x0D6A, 0x72F, 0x0BA8, 0x7C9, 0x0BA8, 0x131F, 0x0C75C7, 0x0D};

} // namespace

void descrambleBlob(std::vector<uint8_t>& data) {
    if (data.empty()) {
        return;
    }
    const int len = static_cast<int>(data.size());
    for (int curr = len - 1; curr >= 0; --curr) {
        const int offset = (curr + kOffsetLookupTable[curr % 44]) % len;
        std::swap(data[static_cast<size_t>(curr)], data[static_cast<size_t>(offset)]);
    }
    for (auto& b : data) {
        b = static_cast<uint8_t>(-1 - static_cast<int>(b));
    }
}

} // namespace cube
