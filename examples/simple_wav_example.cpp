#include <stdio.h>
#include <memory>
#include "pico/stdlib.h"
#include "AudioAPI.h"
#include "PicoAudioCore.h"

using namespace Audio;

/**
 * @brief 最简单的WAV播放示例
 * 展示如何用几行代码播放SD卡上的WAV文件
 */
int main() {
    stdio_init_all();
    sleep_ms(1000);

    printf("\n=== 🎵 简单WAV播放示例 ===\n");
    printf("最简单的WAV文件播放演示\n\n");

    // 创建音频系统 - 只需3行代码！
    auto audio_core = std::make_unique<PicoAudioCore>();
    auto audio_api = std::make_unique<AudioAPI>(std::move(audio_core));
    
    // 初始化音频系统
    if (!audio_api->initialize()) {
        printf("❌ 音频系统初始化失败\n");
        return -1;
    }
    printf("✅ 音频系统就绪\n");

    // 初始化SD卡（使用默认引脚配置）
    if (!audio_api->initializeSD()) {
        printf("❌ SD卡初始化失败\n");
        printf("💡 默认引脚配置:\n");
        printf("   GPIO 16 -> MISO\n");
        printf("   GPIO 17 -> CS\n");
        printf("   GPIO 18 -> SCK\n");
        printf("   GPIO 19 -> MOSI\n");
        return -1;
    }
    printf("✅ SD卡就绪\n");

    // 播放WAV文件
    printf("🎵 播放 test.wav...\n");
    if (audio_api->playWAV("/test.wav")) {
        printf("✅ 播放开始\n");
        
        // 等待播放完成
        while (audio_api->isPlayingWAV()) {
            audio_api->process();
            sleep_ms(100);
        }
        
        printf("✅ 播放完成\n");
    } else {
        printf("❌ 播放失败 - 请确保SD卡根目录有test.wav文件\n");
    }

    printf("\n💡 就是这么简单！\n");
    printf("🎵 更多功能:\n");
    printf("   - audio_api->pauseWAV()     // 暂停/继续\n");
    printf("   - audio_api->stopWAV()      // 停止播放\n");
    printf("   - audio_api->seekWAV(10.0f) // 跳转到10秒\n");
    printf("   - audio_api->setVolume(80)   // 设置音量\n");

    return 0;
} 