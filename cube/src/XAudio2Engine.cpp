#include "cube/XAudio2Engine.hpp"

#include "cube/AudioFileLoader.hpp"
#include "cube/WavLoader.hpp"

#define INITGUID
#include <objbase.h>
#include <xaudio2.h>

namespace cube::audio {

namespace {

class VoiceCallback final : public IXAudio2VoiceCallback {
public:
    void OnVoiceProcessingPassStart(UINT32) override {}
    void OnVoiceProcessingPassEnd() override {}
    void OnStreamEnd() override {}
    void OnBufferStart(void*) override {}
    void OnBufferEnd(void*) override {}
    void OnLoopEnd(void*) override {}
    void OnVoiceError(void*, HRESULT) override {}
};

void destroyVoice(IXAudio2SourceVoice*& voice) {
    if (voice != nullptr) {
        voice->Stop(0);
        voice->DestroyVoice();
        voice = nullptr;
    }
}

} // namespace

Sound::Sound() {
    callback_ = new VoiceCallback();
}

Sound::~Sound() {
    destroyVoice();
    delete static_cast<VoiceCallback*>(callback_);
    callback_ = nullptr;
}

void Sound::setEngine(IXAudio2* engine) {
    engine_ = engine;
}

void Sound::destroyVoice() {
    auto* voice = reinterpret_cast<IXAudio2SourceVoice*>(voice_);
    cube::audio::destroyVoice(voice);
    voice_ = voice;
    playing_ = false;
}

bool Sound::loadFromDatabase(Database& db, const std::string& key) {
    destroyVoice();
    wavData_.clear();
    pcm_ = nullptr;
    pcmBytes_ = 0;

    std::vector<uint8_t> blob;
    if (!db.db().fetchBlob(key, blob)) {
        return false;
    }

    ParsedWav wav = parseWavBlob(std::move(blob));
    if (!wav.valid) {
        return false;
    }

    const size_t pcmOffset = static_cast<size_t>(wav.pcm - wav.data.data());
    wavData_ = std::move(wav.data);
    pcm_ = wavData_.data() + pcmOffset;
    pcmBytes_ = wav.pcmBytes;
    channels_ = wav.channels;
    bitsPerSample_ = wav.bitsPerSample;
    sampleRate_ = wav.sampleRate;
    return true;
}

bool Sound::loadFromFile(const std::string& path) {
    destroyVoice();
    wavData_.clear();
    pcm_ = nullptr;
    pcmBytes_ = 0;

    ParsedAudio parsed;
    if (!loadAudioFile(path, parsed)) {
        return false;
    }

    const size_t pcmOffset = static_cast<size_t>(parsed.pcm - parsed.data.data());
    wavData_ = std::move(parsed.data);
    pcm_ = wavData_.data() + pcmOffset;
    pcmBytes_ = parsed.pcmBytes;
    channels_ = parsed.channels;
    bitsPerSample_ = parsed.bitsPerSample;
    sampleRate_ = parsed.sampleRate;
    return true;
}

void Sound::play(bool loop) {
    destroyVoice();
    auto* engine = static_cast<IXAudio2*>(engine_);
    if (engine == nullptr || pcm_ == nullptr || pcmBytes_ == 0) {
        return;
    }

    WAVEFORMATEX format{};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = channels_;
    format.nSamplesPerSec = sampleRate_;
    format.wBitsPerSample = bitsPerSample_;
    format.nBlockAlign = static_cast<WORD>((format.nChannels * format.wBitsPerSample) / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

    IXAudio2SourceVoice* voice = nullptr;
    auto* callback = static_cast<VoiceCallback*>(callback_);
    if (FAILED(engine->CreateSourceVoice(&voice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, callback,
                                         nullptr, nullptr))) {
        return;
    }

    XAUDIO2_BUFFER buffer{};
    buffer.pAudioData = pcm_;
    buffer.AudioBytes = pcmBytes_;
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    if (FAILED(voice->SubmitSourceBuffer(&buffer, nullptr))) {
        cube::audio::destroyVoice(voice);
        return;
    }

    voice->SetVolume(volume_);
    if (FAILED(voice->Start(0))) {
        cube::audio::destroyVoice(voice);
        return;
    }

    voice_ = voice;
    playing_ = true;
}

void Sound::stop() {
    destroyVoice();
}

Music::Music() = default;
Music::~Music() = default;

void Music::setEngine(IXAudio2* engine) {
    engine_ = engine;
}

bool Music::loadFromDatabase(Database& db, const std::string& key) {
    stop();
    track_ = std::make_unique<Sound>();
    track_->setEngine(engine_);
    return track_->loadFromDatabase(db, key);
}

bool Music::loadFromFile(const std::string& path) {
    stop();
    track_ = std::make_unique<Sound>();
    track_->setEngine(engine_);
    return track_->loadFromFile(path);
}

void Music::play(bool loop) {
    if (track_) {
        track_->setVolume(volume_);
        track_->play(loop);
    }
}

void Music::stop() {
    if (track_) {
        track_->stop();
    }
}

} // namespace cube::audio

namespace cube {

XAudio2Engine::XAudio2Engine() = default;

XAudio2Engine::~XAudio2Engine() {
    shutdown();
}

bool XAudio2Engine::init() {
    if (initialized_) {
        return true;
    }
    const HRESULT comHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(comHr)) {
        return false;
    }
    comInitialized_ = (comHr == S_OK);

    if (FAILED(XAudio2Create(&xaudio_, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
        shutdown();
        return false;
    }

    if (FAILED(xaudio_->CreateMasteringVoice(&masterVoice_))) {
        shutdown();
        return false;
    }

    sound_.setEngine(xaudio_);
    music_.setEngine(xaudio_);
    initialized_ = true;
    return true;
}

void XAudio2Engine::shutdown() {
    music_.stop();
    sound_.stop();
    if (masterVoice_ != nullptr) {
        masterVoice_->DestroyVoice();
        masterVoice_ = nullptr;
    }
    if (xaudio_ != nullptr) {
        xaudio_->Release();
        xaudio_ = nullptr;
    }
    database_.close();
    if (comInitialized_) {
        CoUninitialize();
        comInitialized_ = false;
    }
    initialized_ = false;
}

bool XAudio2Engine::loadDatabase(const std::string& data2Path) {
    return database_.open(data2Path);
}

} // namespace cube
