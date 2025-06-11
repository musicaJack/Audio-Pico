#include <stdio.h>
#include <memory>

#include "pico/stdlib.h"
#include "AudioAPI.h"
#include "PicoAudioCore.h"

using namespace Audio;

/**
 * @brief WAV文件播放演示程序
 * 展示如何使用AudioAPI播放SD卡上的WAV文件
 */
int main() {
    // 初始化标准I/O
    stdio_init_all();
    sleep_ms(2000); // 等待串口连接

    printf("\n=== 🎵 WAV文件播放演示 === \n");
    printf("展示SD卡WAV文件播放功能\n");
    printf("=========================\n\n");

    // 步骤1：创建音频系统
    printf("📝 步骤1: 创建音频系统...\n");
    auto audio_core = std::make_unique<PicoAudioCore>();
    auto audio_api = std::make_unique<AudioAPI>(std::move(audio_core));
    
    // 步骤2：初始化音频系统
    printf("📝 步骤2: 初始化音频系统...\n");
    if (!audio_api->initialize()) {
        printf("❌ 音频系统初始化失败\n");
        return -1;
    }
    printf("✅ 音频系统初始化成功\n");
    
    // 步骤3：配置SD卡参数并初始化
    printf("📝 步骤3: 初始化SD卡...\n");
    SDCardConfig sd_config;
    sd_config.sck_pin = 18;     // SPI时钟引脚
    sd_config.mosi_pin = 19;    // SPI MOSI引脚  
    sd_config.miso_pin = 16;    // SPI MISO引脚
    sd_config.cs_pin = 17;      // SPI片选引脚
    sd_config.spi_speed_hz = 12500000; // 12.5MHz
    
    if (!audio_api->initializeSD(sd_config)) {
        printf("❌ SD卡初始化失败\n");
        printf("💡 请检查:\n");
        printf("   - SD卡是否正确插入\n");
        printf("   - SPI引脚连接是否正确\n");
        printf("   - SD卡格式是否为FAT32\n");
        return -1;
    }
    printf("✅ SD卡初始化成功\n");

    // 步骤4：设置事件回调
    printf("📝 步骤4: 设置事件回调...\n");
    
    // 音频系统事件回调
    audio_api->setEventCallback([](const AudioEventData& event) {
        switch (event.event) {
            case AudioEvent::PLAYBACK_STARTED:
                printf("  🎵 音频开始播放: %s\n", event.message.c_str());
                break;
            case AudioEvent::PLAYBACK_STOPPED:
                printf("  ⏹️ 音频停止播放: %s\n", event.message.c_str());
                break;
            case AudioEvent::ERROR_OCCURRED:
                printf("  ❌ 音频错误: %s\n", event.message.c_str());
                break;
            default:
                break;
        }
    });
    
    // WAV播放器事件回调
    audio_api->setWAVEventCallback([](const WAVEventData& event) {
        switch (event.event) {
            case WAVEvent::PLAYBACK_STARTED:
                printf("  🎵 WAV播放开始: %s\n", event.message.c_str());
                break;
            case WAVEvent::PLAYBACK_FINISHED:
                printf("  ✅ WAV播放完成: %s\n", event.message.c_str());
                break;
            case WAVEvent::POSITION_CHANGED:
                printf("  📍 播放位置: %.1fs / %.1fs\n", 
                       event.position_seconds, event.duration_seconds);
                break;
            case WAVEvent::ERROR_OCCURRED:
                printf("  ❌ WAV错误: %s\n", event.message.c_str());
                break;
            default:
                break;
        }
    });
    
    printf("✅ 事件回调设置完成\n\n");

    // 步骤5：显示支持的格式
    printf("📋 支持的WAV格式:\n");
    auto formats = audio_api->getSupportedWAVFormats();
    for (const auto& format : formats) {
        printf("  ✓ %s\n", format.c_str());
    }
    printf("\n");

    // 步骤6：尝试播放测试文件
    printf("🎵 开始WAV文件播放演示...\n\n");

    // 演示1：播放test.wav文件
    printf("🎼 演示1: 播放 test.wav\n");
    if (audio_api->playWAV("/test.wav")) {
        printf("✅ 开始播放 test.wav\n");
        
        // 等待播放完成或超时
        int timeout = 0;
        while (audio_api->isPlayingWAV() && timeout < 3000) { // 最多30秒
            audio_api->process();
            sleep_ms(10);
            timeout++;
            
            // 每5秒显示一次播放进度
            if (timeout % 500 == 0) {
                printf("  ⏰ 播放中... %.1fs / %.1fs\n", 
                       audio_api->getWAVPosition(), 
                       audio_api->getWAVDuration());
            }
        }
        
        if (timeout >= 3000) {
            printf("⚠️ 播放超时，停止播放\n");
            audio_api->stopWAV();
        }
    } else {
        printf("❌ 无法播放 test.wav\n");
        printf("💡 请确保SD卡根目录有 test.wav 文件\n");
    }
    
    sleep_ms(1000);

    // 演示2：播放另一个文件（如果存在）
    printf("\n🎼 演示2: 播放 music.wav\n");
    if (audio_api->playWAV("/music.wav")) {
        printf("✅ 开始播放 music.wav\n");
        
        // 播放5秒后暂停
        sleep_ms(5000);
        printf("⏸️ 暂停播放\n");
        audio_api->pauseWAV();
        
        sleep_ms(2000);
        
        // 继续播放
        printf("▶️ 继续播放\n");
        audio_api->pauseWAV();
        
        // 再播放5秒
        sleep_ms(5000);
        
        // 跳转到中间位置
        float duration = audio_api->getWAVDuration();
        if (duration > 10.0f) {
            printf("⏩ 跳转到中间位置\n");
            audio_api->seekWAV(duration / 2.0f);
            sleep_ms(3000);
        }
        
        printf("⏹️ 停止播放\n");
        audio_api->stopWAV();
    } else {
        printf("❌ 无法播放 music.wav (文件可能不存在)\n");
    }

    sleep_ms(1000);

    // 演示3：文件信息显示
    printf("\n🎼 演示3: 显示文件信息\n");
    if (audio_api->playWAV("/test.wav")) {
        audio_api->stopWAV(); // 只是加载，不播放
        
        const WAVHeader* info = audio_api->getWAVInfo();
        if (info) {
            printf("📋 WAV文件信息:\n");
            printf("  采样率: %d Hz\n", info->sample_rate);
            printf("  声道数: %d\n", info->channels);
            printf("  位深度: %d bit\n", info->bits_per_sample);
            printf("  文件大小: %d bytes\n", info->file_size);
            printf("  音频时长: %.2f 秒\n", info->getDuration());
        }
    }

    printf("\n✅ WAV播放演示完成！\n");
    printf("💡 使用说明:\n");
    printf("  - 将WAV文件放在SD卡根目录\n");
    printf("  - 支持16位PCM格式，44.1kHz\n");
    printf("  - 文件名示例: test.wav, music.wav\n");
    printf("  - 确保SD卡格式为FAT32\n\n");

    printf("🔌 硬件连接 (SD卡模块):\n");
    printf("  GPIO 16 -> MISO\n");
    printf("  GPIO 17 -> CS\n");
    printf("  GPIO 18 -> SCK\n");
    printf("  GPIO 19 -> MOSI\n");
    printf("  3.3V    -> VCC\n");
    printf("  GND     -> GND\n\n");

    printf("🔌 硬件连接 (音频输出):\n");
    printf("  GPIO 26 -> DIN\n");
    printf("  GPIO 27 -> BCLK\n");
    printf("  GPIO 28 -> LRCLK\n");
    printf("  GPIO 22 -> XMT (静音控制)\n\n");

    printf("🎵 程序结束，感谢使用！\n");

    return 0;
} 