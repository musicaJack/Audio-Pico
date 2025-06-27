#pragma once

#include "AudioCore.hpp"
#include "MusicSequencer.hpp"
#include "PicoAudioCore.hpp"
#include "Notes.hpp"
// WAV功能暂时禁用 - 缺少pico_fatfs依赖
// #include "WAVPlayer.h"
#include <memory>
#include <string>
#include <map>
#include <functional>

namespace Audio {

/**
 * @brief 音频事件类型枚举
 */
enum class AudioEvent {
    PLAYBACK_STARTED,
    PLAYBACK_STOPPED,
    PLAYBACK_PAUSED,
    NOTE_CHANGED,
    VOLUME_CHANGED,
    ERROR_OCCURRED
};

/**
 * @brief 音频事件数据结构
 */
struct AudioEventData {
    AudioEvent event = AudioEvent::PLAYBACK_STARTED;
    std::string message;
    int32_t value = 0;
    float float_value = 0.0f;
    
    AudioEventData() = default;
    AudioEventData(AudioEvent e, const std::string& msg = "", int32_t val = 0, float fval = 0.0f);
};

/**
 * @brief 音频事件回调函数类型
 */
using AudioEventCallback = std::function<void(const AudioEventData&)>;



/**
 * @brief 统一音频API类
 * 提供简化的3行代码集成接口，用于快速在其他项目中集成音频功能
 */
class AudioAPI {
public:
    /**
     * @brief 构造函数
     * @param audio_core 音频核心实现
     */
    explicit AudioAPI(std::unique_ptr<AudioCore> audio_core);

    /**
     * @brief 析构函数
     */
    ~AudioAPI();

    /**
     * @brief 初始化音频系统
     * @param config 音频配置
     * @return 是否初始化成功
     */
    bool initialize(const AudioConfig& config = AudioConfig{});

    /**
     * @brief 播放DO RE MI音阶
     * @param note_duration 音符持续时间（毫秒）
     * @param pause_duration 暂停时间（毫秒）
     * @param loop 是否循环播放
     * @return 是否启动成功
     */
    bool playDoReMi(uint32_t note_duration = 500, uint32_t pause_duration = 100, bool loop = false);

    /**
     * @brief 播放音符序列
     * @param sequence 音符序列
     * @param loop 是否循环播放
     * @return 是否启动成功
     */
    bool playSequence(const MusicSequence& sequence, bool loop = false);

    /**
     * @brief 播放单个音符
     * @param frequency 频率（Hz）
     * @param duration 持续时间（毫秒）
     * @param note_name 音符名称
     * @return 是否启动成功
     */
    bool playNote(float frequency, uint32_t duration = 500, const std::string& note_name = "");

    /**
     * @brief 播放指定索引的音符（用于手动控制）
     * @param index 音符索引
     * @return 是否成功
     */
    bool playNoteByIndex(size_t index);

    /**
     * @brief 暂停播放
     */
    void pause();

    /**
     * @brief 停止播放
     */
    void stop();

    /**
     * @brief 设置音量
     * @param volume 音量值（0-100）
     */
    void setVolume(uint8_t volume);

    /**
     * @brief 获取当前音量
     * @return 音量值（0-100）
     */
    uint8_t getVolume() const;

    /**
     * @brief 设置波形类型
     * @param wave_type 波形类型
     */
    void setWaveType(WaveType wave_type);

    /**
     * @brief 获取当前波形类型
     * @return 波形类型
     */
    WaveType getWaveType() const;

    /**
     * @brief 设置静音状态
     * @param muted 是否静音
     */
    void setMuted(bool muted);

    /**
     * @brief 获取静音状态
     * @return 是否静音
     */
    bool isMuted() const;

    /**
     * @brief 切换静音状态
     */
    void toggleMute();

    /**
     * @brief 切换波形类型（在正弦波和钢琴音色之间切换）
     */
    void toggleWaveType();

    /**
     * @brief 检查是否正在播放
     * @return 是否正在播放
     */
    bool isPlaying() const;

    /**
     * @brief 获取当前播放状态
     * @return 播放状态
     */
    PlaybackState getPlaybackState() const;

    /**
     * @brief 设置事件回调函数
     * @param callback 事件回调函数
     */
    void setEventCallback(AudioEventCallback callback);

    /**
     * @brief 处理音频数据（应在主循环中定期调用）
     */
    void process();

    /**
     * @brief 获取支持的预设音符
     * @return 音符名称到频率的映射
     */
    static std::map<std::string, float> getPresetNotes();

    /**
     * @brief 通过音符名称播放
     * @param note_name 音符名称（如"DO", "RE", "C4", "A4"等）
     * @param duration 持续时间（毫秒）
     * @return 是否启动成功
     */
    bool playNoteByName(const std::string& note_name, uint32_t duration = 500);

    // ========== WAV文件播放功能 (暂时禁用 - 缺少pico_fatfs) ==========
    
    /*
    bool initializeSD(const SDCardConfig& sd_config = SDCardConfig{});
    bool playWAV(const std::string& filename);
    bool switchToWAV(const std::string& filename);
    bool isPlayingWAV() const;
    float getWAVPosition() const;
    float getWAVDuration() const;
    bool seekWAV(float position_seconds);
    void pauseWAV();
    void stopWAV();
    const WAVHeader* getWAVInfo() const;
    void setWAVEventCallback(WAVEventCallback callback);
    static std::vector<std::string> getSupportedWAVFormats();
    */

private:
    std::unique_ptr<AudioCore> audio_core_;
    std::unique_ptr<MusicSequencer> sequencer_;
    // std::unique_ptr<WAVPlayer> wav_player_;  // 暂时禁用
    AudioEventCallback event_callback_;
    bool initialized_ = false;
    // bool sd_initialized_ = false;  // 暂时禁用
    bool loop_enabled_ = false;
    uint8_t current_volume_ = 50;
    WaveType current_wave_type_ = WaveType::PIANO;

    /**
     * @brief 设置音序器
     */
    void setupSequencer();

    /**
     * @brief 检查是否已初始化
     * @return 是否已初始化
     */
    bool checkInitialized();

    /**
     * @brief 生成音频采样数据
     * @param samples 采样缓冲区
     * @param count 采样数量
     */
    void generateAudioSamples(int16_t* samples, size_t count);

    /**
     * @brief 发送事件通知
     * @param event 事件类型
     * @param message 事件消息
     * @param value 事件值
     */
    void notifyEvent(AudioEvent event, const std::string& message = "", int32_t value = 0);

    /**
     * @brief 波形类型转字符串
     * @param wave_type 波形类型
     * @return 波形类型字符串
     */
    std::string waveTypeToString(WaveType wave_type) const;
};

} // namespace Audio 