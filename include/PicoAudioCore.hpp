#pragma once

#include "AudioCore.hpp"
#include "pico/audio_i2s.h"
#include <algorithm>

namespace Audio {

/**
 * @brief Pico I2S配置结构
 */
struct PicoI2SConfig {
    uint8_t data_pin = 26;           // DIN引脚
    uint8_t clock_pin_base = 27;     // BCLK引脚基址（LRCLK=BCLK+1）
    uint8_t dma_channel = 0;         // DMA通道
    uint8_t pio_sm = 0;              // PIO状态机
    uint8_t mute_pin = 22;           // 静音控制引脚（可选）
    bool enable_mute_control = true;  // 是否启用静音控制
};

/**
 * @brief Raspberry Pi Pico音频核心实现
 * 基于I2S接口的音频输出实现
 */
class PicoAudioCore : public AudioCore {
public:
    /**
     * @brief 构造函数
     * @param i2s_config I2S配置
     */
    explicit PicoAudioCore(const PicoI2SConfig& i2s_config = PicoI2SConfig{});

    /**
     * @brief 析构函数
     */
    ~PicoAudioCore() override;

    /**
     * @brief 初始化音频系统
     * @param config 音频配置
     * @return 是否初始化成功
     */
    bool initialize(const AudioConfig& config) override;

    /**
     * @brief 设置音频回调函数
     * @param callback 音频回调函数
     */
    void setAudioCallback(AudioCallback callback) override;

    /**
     * @brief 启动音频输出
     * @return 是否启动成功
     */
    bool start() override;

    /**
     * @brief 停止音频输出
     */
    void stop() override;

    /**
     * @brief 设置音量
     * @param volume 音量值 (0-255)
     */
    void setVolume(uint8_t volume) override;

    /**
     * @brief 获取当前音量
     * @return 当前音量值
     */
    uint8_t getVolume() const override;

    /**
     * @brief 检查音频系统是否正在运行
     * @return 是否正在运行
     */
    bool isRunning() const override;

    /**
     * @brief 获取音频配置
     * @return 当前音频配置
     */
    const AudioConfig& getConfig() const override;

    /**
     * @brief 处理音频缓冲
     * 此方法应在主循环中定期调用
     */
    void processAudio();

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
     * @brief 获取I2S配置
     * @return I2S配置
     */
    const PicoI2SConfig& getI2SConfig() const;

private:
    PicoI2SConfig i2s_config_;
    audio_format_t pico_audio_format_;
    audio_buffer_format_t producer_format_;
    audio_i2s_config_t pico_i2s_config_;
    audio_buffer_pool_t* audio_pool_ = nullptr;
    bool muted_ = false;

    /**
     * @brief 设置音频格式
     */
    void setupAudioFormat();

    /**
     * @brief 创建音频缓冲池
     * @return 是否创建成功
     */
    bool createAudioBufferPool();

    /**
     * @brief 初始化I2S
     * @return 是否初始化成功
     */
    bool initializeI2S();

    /**
     * @brief 应用音量控制
     * @param samples 样本数据
     * @param count 样本数量
     */
    void applyVolumeControl(int16_t* samples, size_t count);

    /**
     * @brief 清理资源
     */
    void cleanupResources();
};

} // namespace Audio 