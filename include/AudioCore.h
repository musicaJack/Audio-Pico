#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <vector>

namespace Audio {

/**
 * @brief 音频格式配置结构
 */
struct AudioConfig {
    uint32_t sample_rate = 44100;
    uint16_t bit_depth = 16;
    uint8_t channels = 2;
    uint32_t buffer_size = 1156;
};

/**
 * @brief 音频回调函数类型
 * @param samples 采样数据指针
 * @param sample_count 采样数量
 */
using AudioCallback = std::function<void(int16_t* samples, size_t sample_count)>;

/**
 * @brief 音频核心抽象基类
 * 定义了音频系统的基本接口，支持不同硬件平台的实现
 */
class AudioCore {
public:
    virtual ~AudioCore() = default;

    /**
     * @brief 初始化音频系统
     * @param config 音频配置
     * @return 是否初始化成功
     */
    virtual bool initialize(const AudioConfig& config) = 0;

    /**
     * @brief 设置音频回调函数
     * @param callback 音频回调函数
     */
    virtual void setAudioCallback(AudioCallback callback) = 0;

    /**
     * @brief 启动音频输出
     * @return 是否启动成功
     */
    virtual bool start() = 0;

    /**
     * @brief 停止音频输出
     */
    virtual void stop() = 0;

    /**
     * @brief 设置音量
     * @param volume 音量值 (0-255)
     */
    virtual void setVolume(uint8_t volume) = 0;

    /**
     * @brief 获取当前音量
     * @return 当前音量值
     */
    virtual uint8_t getVolume() const = 0;

    /**
     * @brief 检查音频系统是否正在运行
     * @return 是否正在运行
     */
    virtual bool isRunning() const = 0;

    /**
     * @brief 获取音频配置
     * @return 当前音频配置
     */
    virtual const AudioConfig& getConfig() const = 0;

protected:
    AudioConfig config_;
    AudioCallback audio_callback_;
    uint8_t volume_ = 128;
    bool running_ = false;
};

} // namespace Audio 