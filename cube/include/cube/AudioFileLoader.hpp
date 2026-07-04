#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cube::audio {

struct ParsedAudio {
    std::vector<uint8_t> data;
    const uint8_t* pcm{nullptr};
    uint32_t pcmBytes{0};
    uint16_t channels{0};
    uint16_t bitsPerSample{16};
    uint32_t sampleRate{0};
    uint16_t formatTag{1}; // WAVE_FORMAT_PCM
    bool valid{false};
};

bool loadAudioFile(const std::string& path, ParsedAudio& out);

std::vector<std::string> listMusicTracks(const std::string& rootDir);

} // namespace cube::audio
