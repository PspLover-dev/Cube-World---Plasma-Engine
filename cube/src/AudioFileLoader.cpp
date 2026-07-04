#include "cube/AudioFileLoader.hpp"

#include "cube/WavLoader.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

extern "C" {
int stb_vorbis_decode_filename(const char* filename, int* channels, int* sample_rate, short** output);
}

namespace cube::audio {

namespace {

bool endsWithIgnoreCase(const std::string& value, const std::string& suffix) {
    if (value.size() < suffix.size()) {
        return false;
    }
    const size_t start = value.size() - suffix.size();
    for (size_t i = 0; i < suffix.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(value[start + i])) !=
            std::tolower(static_cast<unsigned char>(suffix[i]))) {
            return false;
        }
    }
    return true;
}

bool loadOggFile(const std::string& path, ParsedAudio& out) {
    int channels = 0;
    int sampleRate = 0;
    short* samples = nullptr;
    const int sampleCount = stb_vorbis_decode_filename(path.c_str(), &channels, &sampleRate, &samples);
    if (sampleCount <= 0 || samples == nullptr || channels <= 0 || sampleRate <= 0) {
        if (samples != nullptr) {
            free(samples);
        }
        return false;
    }

    const size_t pcmBytes = static_cast<size_t>(sampleCount) * static_cast<size_t>(channels) *
                            sizeof(short);
    out.data.resize(pcmBytes);
    std::memcpy(out.data.data(), samples, pcmBytes);
    free(samples);

    out.pcm = out.data.data();
    out.pcmBytes = static_cast<uint32_t>(pcmBytes);
    out.channels = static_cast<uint16_t>(channels);
    out.bitsPerSample = 16;
    out.sampleRate = static_cast<uint32_t>(sampleRate);
    out.formatTag = 1;
    out.valid = true;
    return true;
}

bool loadWavFile(const std::string& path, ParsedAudio& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    std::vector<uint8_t> blob((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (blob.empty()) {
        return false;
    }

    const ParsedWav wav = parseWavFile(std::move(blob));
    if (!wav.valid) {
        return false;
    }

    const size_t pcmOffset = static_cast<size_t>(wav.pcm - wav.data.data());
    out.data = wav.data;
    out.pcm = out.data.data() + pcmOffset;
    out.pcmBytes = wav.pcmBytes;
    out.channels = wav.channels;
    out.bitsPerSample = wav.bitsPerSample;
    out.sampleRate = wav.sampleRate;
    out.formatTag = 1;
    out.valid = true;
    return true;
}

} // namespace

bool loadAudioFile(const std::string& path, ParsedAudio& out) {
    out = {};
    if (endsWithIgnoreCase(path, ".ogg")) {
        return loadOggFile(path, out);
    }
    if (endsWithIgnoreCase(path, ".wav")) {
        return loadWavFile(path, out);
    }
    return false;
}

std::vector<std::string> listMusicTracks(const std::string& rootDir) {
    std::vector<std::string> tracks;
    const std::filesystem::path musicDir = std::filesystem::path(rootDir) / "Music";
    if (!std::filesystem::is_directory(musicDir)) {
        return tracks;
    }

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(musicDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const std::string ext = entry.path().extension().string();
        if (endsWithIgnoreCase(ext, ".ogg") || endsWithIgnoreCase(ext, ".wav")) {
            tracks.push_back(entry.path().filename().string());
        }
    }

    std::sort(tracks.begin(), tracks.end());
    return tracks;
}

} // namespace cube::audio
