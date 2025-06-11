#include "WAVPlayer.h"
#include <cstring>
#include <algorithm>

// Pico SDK includes for SD card support
#include "hardware/spi.h"
#include "hardware/gpio.h"

namespace Audio {

// ============================================================================
// WAVHeader Implementation
// ============================================================================

bool WAVHeader::isValid() const {
    return (strncmp(riff_id, "RIFF", 4) == 0) &&
           (strncmp(wave_id, "WAVE", 4) == 0) &&
           (strncmp(fmt_id, "fmt ", 4) == 0) &&
           (strncmp(data_id, "data", 4) == 0) &&
           (audio_format == 1); // PCM format
}

float WAVHeader::getDuration() const {
    if (sample_rate == 0) return 0.0f;
    return static_cast<float>(data_size) / byte_rate;
}

// ============================================================================
// WAVPlayer Implementation
// ============================================================================

WAVPlayer::WAVPlayer(const SDCardConfig& sd_config)
    : sd_config_(sd_config) {
    // 初始化缓冲区
    std::fill(read_buffer_, read_buffer_ + BUFFER_SIZE, 0);
}

WAVPlayer::~WAVPlayer() {
    if (state_ == WAVPlaybackState::PLAYING) {
        stop();
    }
    
    if (file_loaded_) {
        f_close(&file_);
    }
    
    if (sd_initialized_) {
        f_unmount("");
    }
}

bool WAVPlayer::initializeSD() {
    if (sd_initialized_) {
        return true;
    }

    // 初始化SPI
    if (!initializeSPI()) {
        notifyEvent(WAVEvent::ERROR_OCCURRED, "SPI初始化失败");
        return false;
    }

    // 挂载文件系统
    FRESULT fr = f_mount(&fs_, "", 1);
    if (fr != FR_OK) {
        notifyEvent(WAVEvent::ERROR_OCCURRED, "SD卡挂载失败: " + std::to_string(fr));
        return false;
    }

    sd_initialized_ = true;
    return true;
}

bool WAVPlayer::loadWAV(const std::string& filename) {
    if (!sd_initialized_) {
        notifyEvent(WAVEvent::ERROR_OCCURRED, "SD卡未初始化");
        return false;
    }

    // 关闭之前的文件
    if (file_loaded_) {
        f_close(&file_);
        file_loaded_ = false;
    }

    // 打开WAV文件
    FRESULT fr = f_open(&file_, filename.c_str(), FA_READ);
    if (fr != FR_OK) {
        notifyEvent(WAVEvent::ERROR_OCCURRED, "无法打开文件: " + filename);
        return false;
    }

    // 解析WAV文件头
    if (!parseWAVHeader()) {
        f_close(&file_);
        notifyEvent(WAVEvent::ERROR_OCCURRED, "WAV文件格式错误");
        return false;
    }

    // 验证WAV文件
    if (!wav_header_.isValid()) {
        f_close(&file_);
        notifyEvent(WAVEvent::ERROR_OCCURRED, "不支持的WAV格式");
        return false;
    }

    current_filename_ = filename;
    file_loaded_ = true;
    current_sample_position_ = 0;
    total_samples_ = wav_header_.data_size / (wav_header_.bits_per_sample / 8) / wav_header_.channels;
    
    // 重置缓冲区
    buffer_position_ = 0;
    buffer_size_ = 0;

    return true;
}

bool WAVPlayer::play() {
    if (!file_loaded_) {
        notifyEvent(WAVEvent::ERROR_OCCURRED, "没有加载WAV文件");
        return false;
    }

    if (state_ == WAVPlaybackState::PLAYING) {
        return true; // Already playing
    }

    state_ = WAVPlaybackState::PLAYING;
    notifyEvent(WAVEvent::PLAYBACK_STARTED, "开始播放: " + current_filename_);
    return true;
}

void WAVPlayer::pause() {
    if (state_ == WAVPlaybackState::PLAYING) {
        state_ = WAVPlaybackState::PAUSED;
        notifyEvent(WAVEvent::PLAYBACK_PAUSED, "播放已暂停");
    } else if (state_ == WAVPlaybackState::PAUSED) {
        state_ = WAVPlaybackState::PLAYING;
        notifyEvent(WAVEvent::PLAYBACK_STARTED, "播放已继续");
    }
}

void WAVPlayer::stop() {
    if (state_ == WAVPlaybackState::STOPPED) {
        return;
    }

    state_ = WAVPlaybackState::STOPPED;
    current_sample_position_ = 0;
    buffer_position_ = 0;
    buffer_size_ = 0;
    
    // 回到文件开始位置
    if (file_loaded_) {
        f_lseek(&file_, sizeof(WAVHeader));
    }
    
    notifyEvent(WAVEvent::PLAYBACK_STOPPED, "播放已停止");
}

bool WAVPlayer::seekTo(float position_seconds) {
    if (!file_loaded_) {
        return false;
    }

    // 计算目标样本位置
    uint32_t target_sample = static_cast<uint32_t>(position_seconds * wav_header_.sample_rate);
    target_sample = std::min(target_sample, total_samples_);

    // 计算文件偏移量
    uint32_t byte_offset = target_sample * (wav_header_.bits_per_sample / 8) * wav_header_.channels;
    FSIZE_t file_position = sizeof(WAVHeader) + byte_offset;

    // 定位文件位置
    FRESULT fr = f_lseek(&file_, file_position);
    if (fr != FR_OK) {
        return false;
    }

    current_sample_position_ = target_sample;
    buffer_position_ = 0;
    buffer_size_ = 0;

    notifyEvent(WAVEvent::POSITION_CHANGED, "播放位置已改变");
    return true;
}

float WAVPlayer::getCurrentPosition() const {
    if (wav_header_.sample_rate == 0) return 0.0f;
    return static_cast<float>(current_sample_position_) / wav_header_.sample_rate;
}

float WAVPlayer::getDuration() const {
    return wav_header_.getDuration();
}

void WAVPlayer::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
}

void WAVPlayer::setEventCallback(WAVEventCallback callback) {
    event_callback_ = callback;
}

void WAVPlayer::generateSamples(int16_t* samples, size_t sample_count) {
    if (state_ != WAVPlaybackState::PLAYING || !file_loaded_) {
        // 填充静音
        std::fill(samples, samples + sample_count, int16_t(0));
        return;
    }

    size_t samples_generated = 0;
    
    while (samples_generated < sample_count) {
        // 检查是否需要填充缓冲区
        if (buffer_position_ >= buffer_size_) {
            if (!fillBuffer()) {
                // 文件结束或读取错误
                state_ = WAVPlaybackState::FINISHED;
                notifyEvent(WAVEvent::PLAYBACK_FINISHED, "播放完成");
                
                // 填充剩余样本为静音
                std::fill(samples + samples_generated, 
                         samples + sample_count, int16_t(0));
                return;
            }
        }

        // 计算可以从缓冲区读取的样本数
        size_t bytes_per_sample = (wav_header_.bits_per_sample / 8) * wav_header_.channels;
        size_t available_samples = (buffer_size_ - buffer_position_) / bytes_per_sample;
        size_t samples_to_read = std::min(available_samples, sample_count - samples_generated);

        // 转换并复制样本
        convertSamples(read_buffer_ + buffer_position_, 
                      samples + samples_generated, 
                      samples_to_read);

        // 更新位置
        buffer_position_ += samples_to_read * bytes_per_sample;
        samples_generated += samples_to_read;
        current_sample_position_ += samples_to_read;

        // 检查是否到达文件末尾
        if (current_sample_position_ >= total_samples_) {
            state_ = WAVPlaybackState::FINISHED;
            notifyEvent(WAVEvent::PLAYBACK_FINISHED, "播放完成");
            
            // 填充剩余样本为静音
            if (samples_generated < sample_count) {
                std::fill(samples + samples_generated, 
                         samples + sample_count, int16_t(0));
            }
            return;
        }
    }

    // 更新播放位置
    updatePlaybackPosition();
}

std::vector<std::string> WAVPlayer::getSupportedFormats() {
    return {
        "PCM 16-bit, 44.1kHz, Stereo",
        "PCM 16-bit, 44.1kHz, Mono", 
        "PCM 16-bit, 22.05kHz, Stereo",
        "PCM 16-bit, 22.05kHz, Mono"
    };
}

// ============================================================================
// Private Methods
// ============================================================================

bool WAVPlayer::parseWAVHeader() {
    UINT bytes_read;
    FRESULT fr = f_read(&file_, &wav_header_, sizeof(WAVHeader), &bytes_read);
    
    if (fr != FR_OK || bytes_read != sizeof(WAVHeader)) {
        return false;
    }

    return true;
}

bool WAVPlayer::initializeSPI() {
    // 初始化SPI硬件
    spi_inst_t* spi = (sd_config_.spi_instance == 0) ? spi0 : spi1;
    
    // 初始化SPI
    spi_init(spi, sd_config_.spi_speed_hz);
    
    // 设置SPI引脚
    gpio_set_function(sd_config_.sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(sd_config_.mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(sd_config_.miso_pin, GPIO_FUNC_SPI);
    
    // 设置片选引脚
    gpio_init(sd_config_.cs_pin);
    gpio_set_dir(sd_config_.cs_pin, GPIO_OUT);
    gpio_put(sd_config_.cs_pin, 1); // CS高电平
    
    return true;
}

void WAVPlayer::notifyEvent(WAVEvent event, const std::string& message) {
    if (event_callback_) {
        WAVEventData event_data(event, message, getCurrentPosition(), getDuration());
        event_callback_(event_data);
    }
}

void WAVPlayer::updatePlaybackPosition() {
    // 定期通知播放位置更新
    static uint32_t last_position_notify = 0;
    uint32_t current_pos = current_sample_position_;
    
    // 每1000个样本（约0.02秒@44.1kHz）通知一次位置更新
    if (current_pos - last_position_notify > 1000) {
        notifyEvent(WAVEvent::POSITION_CHANGED, "播放位置更新");
        last_position_notify = current_pos;
    }
}

bool WAVPlayer::fillBuffer() {
    UINT bytes_read;
    FRESULT fr = f_read(&file_, read_buffer_, BUFFER_SIZE, &bytes_read);
    
    if (fr != FR_OK) {
        return false;
    }
    
    buffer_size_ = bytes_read;
    buffer_position_ = 0;
    
    return bytes_read > 0;
}

void WAVPlayer::convertSamples(const uint8_t* input, int16_t* output, size_t sample_count) {
    if (wav_header_.bits_per_sample == 16) {
        // 16位PCM数据
        const int16_t* input_16 = reinterpret_cast<const int16_t*>(input);
        
        for (size_t i = 0; i < sample_count; ++i) {
            if (wav_header_.channels == 2) {
                // Stereo: 直接复制左右声道
                output[i * 2] = static_cast<int16_t>(input_16[i * 2] * volume_);
                output[i * 2 + 1] = static_cast<int16_t>(input_16[i * 2 + 1] * volume_);
            } else {
                // Mono: 复制到两个声道
                int16_t sample = static_cast<int16_t>(input_16[i] * volume_);
                output[i * 2] = sample;
                output[i * 2 + 1] = sample;
            }
        }
    } else if (wav_header_.bits_per_sample == 8) {
        // 8位PCM数据 (转换为16位)
        for (size_t i = 0; i < sample_count; ++i) {
            int16_t sample = static_cast<int16_t>((input[i] - 128) * 256 * volume_);
            
            if (wav_header_.channels == 2) {
                output[i * 2] = sample;
                output[i * 2 + 1] = static_cast<int16_t>((input[i + 1] - 128) * 256 * volume_);
            } else {
                output[i * 2] = sample;
                output[i * 2 + 1] = sample;
            }
        }
    }
}

} // namespace Audio
