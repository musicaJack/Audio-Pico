#include "PicoAudioCore.h"
#include "hardware/gpio.h"

namespace Audio {

PicoAudioCore::PicoAudioCore(const PicoI2SConfig& i2s_config)
    : i2s_config_(i2s_config), muted_(false) {
}

PicoAudioCore::~PicoAudioCore() {
    if (running_) {
        stop();
    }
    cleanupResources();
}

bool PicoAudioCore::initialize(const AudioConfig& config) {
    if (running_) {
        stop();
    }

    config_ = config;
    
    // 初始化静音控制引脚
    if (i2s_config_.enable_mute_control) {
        gpio_init(i2s_config_.mute_pin);
        gpio_set_dir(i2s_config_.mute_pin, GPIO_OUT);
        setMuted(false); // 默认不静音
    }

    // 配置音频格式
    setupAudioFormat();

    // 创建音频缓冲池
    if (!createAudioBufferPool()) {
        return false;
    }

    // 初始化I2S
    if (!initializeI2S()) {
        cleanupResources();
        return false;
    }

    return true;
}

void PicoAudioCore::setAudioCallback(AudioCallback callback) {
    audio_callback_ = callback;
}

bool PicoAudioCore::start() {
    if (running_ || !audio_pool_ || !audio_callback_) {
        return false;
    }

    // 连接音频管道
    if (!audio_i2s_connect(audio_pool_)) {
        return false;
    }

    // 启用I2S输出
    audio_i2s_set_enabled(true);
    running_ = true;

    return true;
}

void PicoAudioCore::stop() {
    if (running_) {
        audio_i2s_set_enabled(false);
        running_ = false;
    }
}

void PicoAudioCore::setVolume(uint8_t volume) {
    volume_ = volume;
}

uint8_t PicoAudioCore::getVolume() const {
    return volume_;
}

bool PicoAudioCore::isRunning() const {
    return running_;
}

const AudioConfig& PicoAudioCore::getConfig() const {
    return config_;
}

void PicoAudioCore::processAudio() {
    if (!running_ || !audio_pool_ || !audio_callback_) {
        return;
    }

    // 获取可用的音频缓冲区
    audio_buffer_t* buffer = take_audio_buffer(audio_pool_, false);
    if (!buffer) {
        return; // 没有可用缓冲区
    }

    // 获取缓冲区样本数据
    int16_t* samples = (int16_t*)buffer->buffer->bytes;
    size_t sample_count = buffer->max_sample_count;

    // 调用用户回调生成音频数据
    audio_callback_(samples, sample_count * config_.channels);

    // 应用音量控制
    applyVolumeControl(samples, sample_count * config_.channels);

    // 设置实际样本数并返回缓冲区
    buffer->sample_count = sample_count;
    give_audio_buffer(audio_pool_, buffer);
}

void PicoAudioCore::setMuted(bool muted) {
    if (i2s_config_.enable_mute_control) {
        // PCM5102等DAC通常是高电平解除静音，低电平静音
        gpio_put(i2s_config_.mute_pin, muted ? 0 : 1);
        muted_ = muted;
    }
}

bool PicoAudioCore::isMuted() const {
    return muted_;
}

const PicoI2SConfig& PicoAudioCore::getI2SConfig() const {
    return i2s_config_;
}

void PicoAudioCore::setupAudioFormat() {
    // 配置Pico音频格式
    pico_audio_format_.sample_freq = config_.sample_rate;
    pico_audio_format_.channel_count = config_.channels;
    
    // 根据位深度设置格式 (仅支持16位，这是pico-extras库的标准)
    pico_audio_format_.format = AUDIO_BUFFER_FORMAT_PCM_S16;
    if (config_.bit_depth != 16) {
        config_.bit_depth = 16; // 强制使用16位
    }

    // 配置生产者格式
    producer_format_.format = &pico_audio_format_;
    producer_format_.sample_stride = config_.channels * (config_.bit_depth / 8);
}

bool PicoAudioCore::createAudioBufferPool() {
    audio_pool_ = audio_new_producer_pool(&producer_format_, 3, config_.buffer_size);
    return audio_pool_ != nullptr;
}

bool PicoAudioCore::initializeI2S() {
    // 配置I2S参数
    pico_i2s_config_.data_pin = i2s_config_.data_pin;
    pico_i2s_config_.clock_pin_base = i2s_config_.clock_pin_base;
    pico_i2s_config_.dma_channel = i2s_config_.dma_channel;
    pico_i2s_config_.pio_sm = i2s_config_.pio_sm;

    // 初始化I2S
    const audio_format_t* output_format = audio_i2s_setup(&pico_audio_format_, &pico_i2s_config_);
    return output_format != nullptr;
}

void PicoAudioCore::applyVolumeControl(int16_t* samples, size_t count) {
    if (volume_ == 255) {
        return; // 最大音量，无需处理
    }

    if (volume_ == 0 || muted_) {
        // 静音
        std::fill(samples, samples + count, 0);
        return;
    }

    // 应用音量倍率
    float volume_factor = static_cast<float>(volume_) / 255.0f;
    for (size_t i = 0; i < count; ++i) {
        samples[i] = static_cast<int16_t>(samples[i] * volume_factor);
    }
}

void PicoAudioCore::cleanupResources() {
    if (audio_pool_) {
        // 注意：pico-extras音频库可能不提供显式的清理函数
        // 这里主要是重置指针，实际清理可能由库内部处理
        audio_pool_ = nullptr;
    }
}

} // namespace Audio 