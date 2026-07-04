#pragma once

#include "cube/XAudio2Engine.hpp"

#include <cstdint>
#include <vector>

namespace cube::audio {

struct ParsedWav {
    std::vector<uint8_t> data;
    const uint8_t* pcm{nullptr};
    uint32_t pcmBytes{0};
    uint16_t channels{0};
    uint16_t bitsPerSample{0};
    uint32_t sampleRate{0};
    bool valid{false};
};

ParsedWav parseWavBlob(std::vector<uint8_t> blob);
ParsedWav parseWavFile(std::vector<uint8_t> blob);

} // namespace cube::audio
