#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <vector>

// Pico SDK includes for SD card support
#include "pico/stdlib.h"
#include "ff.h"  // FatFS file system

namespace Audio {

/**
 * @brief WAV文件头结构
 */
struct WAVHeader {
    // RIFF header
    char riff_id[4];          // "RIFF"
    uint32_t file_size;       // 文件大小 - 8
    char wave_id[4];          // "WAVE"
    
    // Format chunk
    char fmt_id[4];           // "fmt "
    uint32_t fmt_size;        // 格式块大小（通常为16）
    uint16_t audio_format;    // 音频格式（1 = PCM）
    uint16_t channels;        // 声道数
    uint32_t sample_rate;     // 采样率
    uint32_t byte_rate;       // 字节率
    uint16_t block_align;     // 块对齐
    uint16_t bits_per_sample; // 位深度
    
    // Data chunk
    char data_id[4];          // "data"
    uint32_t data_size;       // 数据大小
    
    // 验证WAV文件头是否有效
    bool isValid() const;
    
    // 获取音频时长（秒）
    float getDuration() const;
};

/**
 * @brief WAV播放状态
 */
enum class WAVPlaybackState {
    STOPPED,
    PLAYING,
    PAUSED,
    FINISHED,
    ERROR
};

/**
 * @brief WAV播放事件
 */
enum class WAVEvent {
    PLAYBACK_STARTED,
    PLAYBACK_STOPPED,
    PLAYBACK_PAUSED,
    PLAYBACK_FINISHED,
    POSITION_CHANGED,
    ERROR_OCCURRED
};

/**
 * @brief WAV事件数据
 */
struct WAVEventData {
    WAVEvent event;
    std::string message;
    float position_seconds = 0.0f;  // 播放位置（秒）
    float duration_seconds = 0.0f;  // 总时长（秒）
    
    WAVEventData(WAVEvent e, const std::string& msg = "", float pos = 0.0f, float dur = 0.0f)
        : event(e), message(msg), position_seconds(pos), duration_seconds(dur) {}
};

/**
 * @brief WAV事件回调函数类型
 */
using WAVEventCallback = std::function<void(const WAVEventData&)>;

/**
 * @brief SD卡配置结构
 */
struct SDCardConfig {
    uint8_t sck_pin = 18;     // SPI时钟引脚
    uint8_t mosi_pin = 19;    // SPI MOSI引脚
    uint8_t miso_pin = 16;    // SPI MISO引脚
    uint8_t cs_pin = 17;      // SPI片选引脚
    uint32_t spi_speed_hz = 12500000;  // SPI速度（12.5MHz）
    uint8_t spi_instance = 0; // SPI实例（0 = spi0, 1 = spi1）
};

/**
 * @brief WAV文件播放器类
 * 支持从SD卡读取WAV文件并通过音频系统播放
 */
class WAVPlayer {
public:
    /**
     * @brief 构造函数
     * @param sd_config SD卡配置
     */
    explicit WAVPlayer(const SDCardConfig& sd_config = SDCardConfig{});
    
    /**
     * @brief 析构函数
     */
    ~WAVPlayer();

    /**
     * @brief 初始化SD卡
     * @return 是否初始化成功
     */
    bool initializeSD();

    /**
     * @brief 加载WAV文件
     * @param filename 文件路径（如 "/test.wav"）
     * @return 是否加载成功
     */
    bool loadWAV(const std::string& filename);

    /**
     * @brief 开始播放当前加载的WAV文件
     * @return 是否开始播放成功
     */
    bool play();

    /**
     * @brief 暂停播放
     */
    void pause();

    /**
     * @brief 停止播放
     */
    void stop();

    /**
     * @brief 设置播放位置
     * @param position_seconds 播放位置（秒）
     * @return 是否设置成功
     */
    bool seekTo(float position_seconds);

    /**
     * @brief 获取当前播放状态
     * @return 播放状态
     */
    WAVPlaybackState getState() const { return state_; }

    /**
     * @brief 获取当前播放位置
     * @return 播放位置（秒）
     */
    float getCurrentPosition() const;

    /**
     * @brief 获取音频总时长
     * @return 总时长（秒）
     */
    float getDuration() const;

    /**
     * @brief 获取WAV文件信息
     * @return WAV文件头信息
     */
    const WAVHeader& getWAVInfo() const { return wav_header_; }

    /**
     * @brief 设置音量
     * @param volume 音量值（0.0 - 1.0）
     */
    void setVolume(float volume);

    /**
     * @brief 获取当前音量
     * @return 音量值（0.0 - 1.0）
     */
    float getVolume() const { return volume_; }

    /**
     * @brief 设置事件回调函数
     * @param callback 事件回调函数
     */
    void setEventCallback(WAVEventCallback callback);

    /**
     * @brief 生成音频样本数据
     * 这个方法会被AudioCore回调调用
     * @param samples 输出样本缓冲区
     * @param sample_count 样本数量
     */
    void generateSamples(int16_t* samples, size_t sample_count);

    /**
     * @brief 检查是否有文件加载
     * @return 是否有文件加载
     */
    bool hasFileLoaded() const { return file_loaded_; }

    /**
     * @brief 获取支持的音频格式列表
     * @return 支持的格式描述
     */
    static std::vector<std::string> getSupportedFormats();

private:
    // SD卡相关
    SDCardConfig sd_config_;
    FATFS fs_;                    // FatFS文件系统
    bool sd_initialized_ = false;

    // WAV文件相关
    FIL file_;                    // 文件句柄
    WAVHeader wav_header_;        // WAV文件头
    bool file_loaded_ = false;    // 是否有文件加载
    std::string current_filename_; // 当前文件名

    // 播放状态
    WAVPlaybackState state_ = WAVPlaybackState::STOPPED;
    float volume_ = 1.0f;
    uint32_t current_sample_position_ = 0;  // 当前样本位置
    uint32_t total_samples_ = 0;            // 总样本数

    // 缓冲区
    static constexpr size_t BUFFER_SIZE = 4096;  // 4KB缓冲区
    uint8_t read_buffer_[BUFFER_SIZE];
    size_t buffer_position_ = 0;
    size_t buffer_size_ = 0;

    // 事件回调
    WAVEventCallback event_callback_;

    // 私有方法
    bool parseWAVHeader();
    bool initializeSPI();
    void notifyEvent(WAVEvent event, const std::string& message = "");
    void updatePlaybackPosition();
    bool fillBuffer();
    void convertSamples(const uint8_t* input, int16_t* output, size_t sample_count);
};

} // namespace Audio 