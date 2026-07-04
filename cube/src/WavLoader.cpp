#include "cube/WavLoader.hpp"

#include "cube/BlobDescramble.hpp"

#include <cstring>

namespace cube::audio {

namespace {

uint32_t readU32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

uint16_t readU16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

bool findChunk(const std::vector<uint8_t>& data, const char id[4], size_t& outOffset,
               uint32_t& outSize) {
    if (data.size() < 12) {
        return false;
    }
    size_t pos = 12;
    while (pos + 8 <= data.size()) {
        if (std::memcmp(data.data() + pos, id, 4) == 0) {
            outOffset = pos + 8;
            outSize = readU32(data.data() + pos + 4);
            return true;
        }
        const uint32_t chunkSize = readU32(data.data() + pos + 4);
        pos += 8 + chunkSize + (chunkSize & 1u);
    }
    return false;
}

} // namespace

ParsedWav parseWavBytes(std::vector<uint8_t> blob, bool descramble) {
    ParsedWav out;
    if (descramble) {
        descrambleBlob(blob);
    }
    if (blob.size() < 44 || std::memcmp(blob.data(), "RIFF", 4) != 0 ||
        std::memcmp(blob.data() + 8, "WAVE", 4) != 0) {
        return out;
    }

    size_t fmtOffset = 0;
    uint32_t fmtSize = 0;
    size_t dataOffset = 0;
    uint32_t dataSize = 0;
    if (!findChunk(blob, "fmt ", fmtOffset, fmtSize) ||
        !findChunk(blob, "data", dataOffset, dataSize)) {
        return out;
    }
    if (fmtOffset + 16 > blob.size() || dataOffset + dataSize > blob.size()) {
        return out;
    }

    const uint8_t* fmt = blob.data() + fmtOffset;
    if (readU16(fmt) != 1) {
        return out;
    }

    out.channels = readU16(fmt + 2);
    out.sampleRate = readU32(fmt + 4);
    out.bitsPerSample = readU16(fmt + 14);
    if (out.channels == 0 || out.bitsPerSample == 0 || out.sampleRate == 0) {
        return out;
    }

    out.data = std::move(blob);
    out.pcm = out.data.data() + dataOffset;
    out.pcmBytes = dataSize;
    out.valid = true;
    return out;
}

ParsedWav parseWavBlob(std::vector<uint8_t> blob) {
    return parseWavBytes(std::move(blob), true);
}

ParsedWav parseWavFile(std::vector<uint8_t> blob) {
    return parseWavBytes(std::move(blob), false);
}

} // namespace cube::audio
