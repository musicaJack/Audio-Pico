#include "AudioAPI.h"
#include <algorithm>

namespace Audio {

// ===================== AudioEventData 实现 =====================

AudioEventData::AudioEventData(AudioEvent e, const std::string& msg, int32_t val, float fval)
    : event(e), message(msg), value(val), float_value(fval) {
}

// ===================== AudioAPI 实现 =====================

AudioAPI::AudioAPI(std::unique_ptr<AudioCore> audio_core)
    : audio_core_(std::move(audio_core)) {
    setupSequencer();
}

AudioAPI::~AudioAPI() {
    if (isPlaying()) {
        stop();
    }
}

bool AudioAPI::initialize(const AudioConfig& config) {
    if (!audio_core_) {
        notifyEvent(AudioEvent::ERROR_OCCURRED, "音频核心未设置");
        return false;
    }

    if (!audio_core_->initialize(config)) {
        notifyEvent(AudioEvent::ERROR_OCCURRED, "音频核心初始化失败");
        return false;
    }

    // 设置音频回调
    audio_core_->setAudioCallback([this](int16_t* samples, size_t count) {
        generateAudioSamples(samples, count);
    });

    initialized_ = true;
    return true;
}

bool AudioAPI::playDoReMi(uint32_t note_duration, uint32_t pause_duration, bool loop) {
    if (!checkInitialized()) return false;

    // 创建DO RE MI音阶序列
    MusicSequence sequence;
    const std::vector<std::pair<float, std::string>> notes = {
        {Notes::C4, "DO (C4)"},
        {Notes::D4, "RE (D4)"},
        {Notes::E4, "MI (E4)"},
        {Notes::F4, "FA (F4)"},
        {Notes::G4, "SOL (G4)"},
        {Notes::A4, "LA (A4)"},
        {Notes::B4, "SI (B4)"},
        {523.25f, "DO (C5)"}
    };

    for (const auto& [freq, name] : notes) {
        sequence.emplace_back(freq, note_duration, pause_duration, 1.0f, name);
    }

    return playSequence(sequence, loop);
}

bool AudioAPI::playSequence(const MusicSequence& sequence, bool loop) {
    if (!checkInitialized()) return false;

    // 首先停止当前播放（如果有的话）
    stop();
    
    sequencer_->setSequence(sequence);
    sequencer_->setLoop(loop);
    
    if (!audio_core_->start()) {
        notifyEvent(AudioEvent::ERROR_OCCURRED, "音频输出启动失败");
        return false;
    }

    sequencer_->play();
    loop_enabled_ = loop;
    
    notifyEvent(AudioEvent::PLAYBACK_STARTED, "开始播放音符序列");
    return true;
}

bool AudioAPI::playNote(float frequency, uint32_t duration, const std::string& note_name) {
    if (!checkInitialized()) return false;

    MusicSequence sequence;
    sequence.emplace_back(frequency, duration, 0, 1.0f, note_name);
    
    return playSequence(sequence, false);
}

bool AudioAPI::playNoteByIndex(size_t index) {
    if (!checkInitialized()) return false;

    sequencer_->playNote(index);
    std::string msg = "播放音符索引: " + std::to_string(index);
    notifyEvent(AudioEvent::NOTE_CHANGED, msg, static_cast<int32_t>(index));
    return true;
}

void AudioAPI::pause() {
    if (sequencer_) {
        sequencer_->pause();
        notifyEvent(AudioEvent::PLAYBACK_PAUSED, "播放已暂停");
    }
}

void AudioAPI::stop() {
    if (sequencer_) {
        sequencer_->stop();
    }
    if (audio_core_) {
        audio_core_->stop();
    }
    notifyEvent(AudioEvent::PLAYBACK_STOPPED, "播放已停止");
}

void AudioAPI::setVolume(uint8_t volume) {
    volume = std::min(volume, static_cast<uint8_t>(100));
    uint8_t pico_volume = static_cast<uint8_t>((volume * 255) / 100);
    
    if (audio_core_) {
        audio_core_->setVolume(pico_volume);
    }
    
    current_volume_ = volume;
    notifyEvent(AudioEvent::VOLUME_CHANGED, "音量已设置", volume);
}

uint8_t AudioAPI::getVolume() const {
    return current_volume_;
}

void AudioAPI::setWaveType(WaveType wave_type) {
    if (sequencer_) {
        sequencer_->setWaveType(wave_type);
    }
    current_wave_type_ = wave_type;
    
    std::string type_name = waveTypeToString(wave_type);
    notifyEvent(AudioEvent::NOTE_CHANGED, "波形类型已设置: " + type_name);
}

WaveType AudioAPI::getWaveType() const {
    return current_wave_type_;
}

void AudioAPI::setMuted(bool muted) {
    if (auto* pico_core = static_cast<PicoAudioCore*>(audio_core_.get())) {
        pico_core->setMuted(muted);
    }
    
    std::string status = muted ? "静音已开启" : "静音已关闭";
    notifyEvent(AudioEvent::VOLUME_CHANGED, status, muted ? 0 : current_volume_);
}

bool AudioAPI::isMuted() const {
    if (auto* pico_core = static_cast<PicoAudioCore*>(audio_core_.get())) {
        return pico_core->isMuted();
    }
    return false;
}

void AudioAPI::toggleMute() {
    setMuted(!isMuted());
}

void AudioAPI::toggleWaveType() {
    WaveType new_type = (current_wave_type_ == WaveType::SINE) ? WaveType::PIANO : WaveType::SINE;
    setWaveType(new_type);
}

bool AudioAPI::isPlaying() const {
    return audio_core_ && audio_core_->isRunning() && 
           sequencer_ && sequencer_->getState() == PlaybackState::PLAYING;
}

PlaybackState AudioAPI::getPlaybackState() const {
    return sequencer_ ? sequencer_->getState() : PlaybackState::STOPPED;
}

void AudioAPI::setEventCallback(AudioEventCallback callback) {
    event_callback_ = callback;
}

void AudioAPI::process() {
    // 对于Pico平台，音频处理通过中断和DMA完成
    // 这里可以添加额外的处理逻辑，如状态检查等
    if (audio_core_) {
        if (auto* pico_core = static_cast<PicoAudioCore*>(audio_core_.get())) {
            pico_core->processAudio();
        }
    }
    
    // 检查音频序列是否完成，如果完成且不循环则停止音频核心
    if (sequencer_ && audio_core_ && audio_core_->isRunning()) {
        if (sequencer_->isFinished() && !loop_enabled_) {
            audio_core_->stop();
            notifyEvent(AudioEvent::PLAYBACK_STOPPED, "序列播放完成");
        }
    }
}

std::map<std::string, float> AudioAPI::getPresetNotes() {
    return {
        {"DO", Notes::C4},   {"C4", Notes::C4},
        {"RE", Notes::D4},   {"D4", Notes::D4},
        {"MI", Notes::E4},   {"E4", Notes::E4},
        {"FA", Notes::F4},   {"F4", Notes::F4},
        {"SOL", Notes::G4},  {"G4", Notes::G4},
        {"LA", Notes::A4},   {"A4", Notes::A4},
        {"SI", Notes::B4},   {"B4", Notes::B4},
        {"DO5", 523.25f},    {"C5", 523.25f}
    };
}

bool AudioAPI::playNoteByName(const std::string& note_name, uint32_t duration) {
    auto notes = getPresetNotes();
    auto it = notes.find(note_name);
    
    if (it == notes.end()) {
        notifyEvent(AudioEvent::ERROR_OCCURRED, "未知音符名称: " + note_name);
        return false;
    }
    
    return playNote(it->second, duration, note_name);
}

void AudioAPI::setupSequencer() {
    if (audio_core_) {
        sequencer_ = std::make_unique<MusicSequencer>();
    }
}

bool AudioAPI::checkInitialized() {
    if (!initialized_) {
        notifyEvent(AudioEvent::ERROR_OCCURRED, "音频系统未初始化");
        return false;
    }
    return true;
}

void AudioAPI::generateAudioSamples(int16_t* samples, size_t count) {
    if (sequencer_) {
        uint32_t sample_rate = audio_core_ ? audio_core_->getConfig().sample_rate : 44100;
        sequencer_->generateSamples(samples, count, sample_rate);
        
        // 检查是否需要循环播放
        if (loop_enabled_ && sequencer_->isFinished()) {
            sequencer_->play();
        }
    } else {
        // 如果没有序列器，填充静音
        std::fill(samples, samples + count, 0);
    }
}

void AudioAPI::notifyEvent(AudioEvent event, const std::string& message, int32_t value) {
    if (event_callback_) {
        AudioEventData event_data(event, message, value);
        event_callback_(event_data);
    }
}

std::string AudioAPI::waveTypeToString(WaveType wave_type) const {
    switch (wave_type) {
        case WaveType::SINE: return "正弦波";
        case WaveType::PIANO: return "钢琴音色";
        case WaveType::SQUARE: return "方波";
        case WaveType::TRIANGLE: return "三角波";
        case WaveType::SAWTOOTH: return "锯齿波";
        default: return "未知";
    }
}

bool AudioAPI::initializeSD(const SDCardConfig& sd_config) {
    if (!checkInitialized()) return false;
    
    if (!wav_player_) {
        wav_player_ = std::make_unique<WAVPlayer>(sd_config);
    }
    
    if (wav_player_->initializeSD()) {
        sd_initialized_ = true;
        notifyEvent(AudioEvent::NOTE_CHANGED, "SD卡初始化成功");
        return true;
    } else {
        notifyEvent(AudioEvent::ERROR_OCCURRED, "SD卡初始化失败");
        return false;
    }
}

bool AudioAPI::playWAV(const std::string& filename) {
    if (!checkInitialized() || !sd_initialized_) {
        notifyEvent(AudioEvent::ERROR_OCCURRED, "SD卡未初始化");
        return false;
    }
    
    // 停止当前播放
    stop();
    
    if (!wav_player_->loadWAV(filename)) {
        return false;
    }
    
    // 设置音频回调为WAV播放器
    audio_core_->setAudioCallback([this](int16_t* samples, size_t count) {
        wav_player_->generateSamples(samples, count);
    });
    
    if (!audio_core_->start()) {
        notifyEvent(AudioEvent::ERROR_OCCURRED, "音频输出启动失败");
        return false;
    }
    
    if (wav_player_->play()) {
        notifyEvent(AudioEvent::PLAYBACK_STARTED, "开始播放WAV文件: " + filename);
        return true;
    }
    
    return false;
}

bool AudioAPI::switchToWAV(const std::string& filename) {
    stopWAV();
    return playWAV(filename);
}

bool AudioAPI::isPlayingWAV() const {
    return wav_player_ && wav_player_->getState() == WAVPlaybackState::PLAYING;
}

float AudioAPI::getWAVPosition() const {
    return wav_player_ ? wav_player_->getCurrentPosition() : 0.0f;
}

float AudioAPI::getWAVDuration() const {
    return wav_player_ ? wav_player_->getDuration() : 0.0f;
}

bool AudioAPI::seekWAV(float position_seconds) {
    return wav_player_ ? wav_player_->seekTo(position_seconds) : false;
}

void AudioAPI::pauseWAV() {
    if (wav_player_) {
        wav_player_->pause();
    }
}

void AudioAPI::stopWAV() {
    if (wav_player_) {
        wav_player_->stop();
    }
    if (audio_core_) {
        audio_core_->stop();
    }
}

const WAVHeader* AudioAPI::getWAVInfo() const {
    return wav_player_ && wav_player_->hasFileLoaded() ? &wav_player_->getWAVInfo() : nullptr;
}

void AudioAPI::setWAVEventCallback(WAVEventCallback callback) {
    if (wav_player_) {
        wav_player_->setEventCallback(callback);
    }
}

std::vector<std::string> AudioAPI::getSupportedWAVFormats() {
    return WAVPlayer::getSupportedFormats();
}

} // namespace Audio 