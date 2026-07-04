#pragma once

#include "cube/Database.hpp"

#include <memory>
#include <string>
#include <vector>

struct IXAudio2;
struct IXAudio2MasteringVoice;

namespace cube {

class CharacterStyleWidget;

namespace audio {

class Sound {
public:
    Sound();
    ~Sound();

    void setEngine(IXAudio2* engine);

    bool loadFromDatabase(Database& db, const std::string& key);
    bool loadFromFile(const std::string& path);
    void play(bool loop = false);
    void stop();
    void setVolume(float volume) { volume_ = volume; }
    bool playing() const { return playing_; }

private:
    void destroyVoice();

    IXAudio2* engine_{nullptr};
    void* voice_{nullptr};
    void* callback_{nullptr};
    std::vector<uint8_t> wavData_;
    const uint8_t* pcm_{nullptr};
    uint32_t pcmBytes_{0};
    uint16_t channels_{0};
    uint16_t bitsPerSample_{0};
    uint32_t sampleRate_{0};
    float volume_{1.f};
    bool playing_{false};
};

class Music {
public:
    Music();
    ~Music();

    void setEngine(IXAudio2* engine);
    void setVolume(float volume) { volume_ = volume; }
    float volume() const { return volume_; }

    bool loadFromDatabase(Database& db, const std::string& key);
    bool loadFromFile(const std::string& path);
    void play(bool loop = true);
    void stop();

private:
    IXAudio2* engine_{nullptr};
    std::unique_ptr<Sound> track_;
    float volume_{1.f};
};

} // namespace audio

class XAudio2Engine {
public:
    XAudio2Engine();
    ~XAudio2Engine();

    bool init();
    void shutdown();

    bool loadDatabase(const std::string& data2Path);
    Database& database() { return database_; }

    audio::Sound& sound() { return sound_; }
    audio::Music& music() { return music_; }

    void setCharacterStyleWidget(CharacterStyleWidget* widget) { styleWidget_ = widget; }

private:
    Database database_;
    audio::Sound sound_;
    audio::Music music_;
    CharacterStyleWidget* styleWidget_{nullptr};
    IXAudio2* xaudio_{nullptr};
    IXAudio2MasteringVoice* masterVoice_{nullptr};
    bool comInitialized_{false};
    bool initialized_{false};
};

} // namespace cube
