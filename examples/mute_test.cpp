#include <stdio.h>
#include <memory>

#include "pico/stdlib.h"
#include "AudioAPI.h"
#include "PicoAudioCore.h"

using namespace Audio;

/**
 * @brief 静音控制测试程序
 * 专门测试静音功能是否正常工作
 */
int main() {
    // 初始化标准I/O
    stdio_init_all();
    sleep_ms(2000);

    printf("\n========================================\n");
    printf("🔇 静音控制测试程序\n");
    printf("========================================\n");
    printf("测试PCM5102 DAC的静音控制功能\n\n");

    // 创建音频系统
    printf("📝 创建音频系统...\n");
    auto audio_core = std::make_unique<PicoAudioCore>();
    auto audio_api = std::make_unique<AudioAPI>(std::move(audio_core));
    
    if (!audio_api->initialize()) {
        printf("❌ 初始化失败\n");
        return -1;
    }
    printf("✅ 音频系统初始化成功\n\n");

    // 显示硬件连接信息
    printf("🔌 硬件连接检查:\n");
    printf("  GPIO 26 -> PCM5102 DIN   (数据)\n");
    printf("  GPIO 27 -> PCM5102 BCLK  (位时钟)\n");
    printf("  GPIO 28 -> PCM5102 LRCLK (左右时钟)\n");
    printf("  GPIO 22 -> PCM5102 XMT   (静音控制)\n");
    printf("  PCM5102 VIN -> 3.3V\n");
    printf("  PCM5102 GND -> GND\n\n");

    // 检查初始静音状态
    printf("🔍 检查初始状态:\n");
    printf("  静音状态: %s\n", audio_api->isMuted() ? "已静音" : "已解除静音");
    printf("  音量设置: %d%%\n\n", audio_api->getVolume());

    // 测试1：确保解除静音
    printf("🔊 测试1: 确保解除静音\n");
    audio_api->setMuted(false);
    audio_api->setVolume(80);
    printf("  ✓ 设置为解除静音，音量80%%\n");
    printf("  ✓ 当前状态: %s\n", audio_api->isMuted() ? "已静音" : "已解除静音");
    sleep_ms(1000);

    // 播放测试音符 - 应该有声音
    printf("\n🎵 播放测试音符 (应该有声音):\n");
    for (int i = 0; i < 3; i++) {
        printf("  播放 DO (第%d次)...\n", i + 1);
        audio_api->playNote(Notes::C4, 800, "DO");
        
        int timeout = 0;
        while (audio_api->isPlaying() && timeout < 400) {
            audio_api->process();
            sleep_ms(10);
            timeout++;
        }
        sleep_ms(500);
    }

    // 测试2：启用静音
    printf("\n🔇 测试2: 启用静音\n");
    audio_api->setMuted(true);
    printf("  ✓ 已启用静音\n");
    printf("  ✓ 当前状态: %s\n", audio_api->isMuted() ? "已静音" : "已解除静音");
    sleep_ms(1000);

    // 播放测试音符 - 应该没有声音
    printf("\n🔇 播放测试音符 (应该没有声音):\n");
    for (int i = 0; i < 3; i++) {
        printf("  播放 RE (第%d次) - 静音状态...\n", i + 1);
        audio_api->playNote(Notes::D4, 800, "RE");
        
        int timeout = 0;
        while (audio_api->isPlaying() && timeout < 400) {
            audio_api->process();
            sleep_ms(10);
            timeout++;
        }
        sleep_ms(500);
    }

    // 测试3：快速切换静音状态
    printf("\n🔄 测试3: 快速切换静音状态\n");
    for (int i = 0; i < 6; i++) {
        bool should_mute = (i % 2 == 0);
        audio_api->setMuted(should_mute);
        printf("  第%d次: %s\n", i + 1, should_mute ? "静音" : "解除静音");
        
        // 播放短音符测试
        audio_api->playNote(Notes::E4, 400, "MI");
        int timeout = 0;
        while (audio_api->isPlaying() && timeout < 200) {
            audio_api->process();
            sleep_ms(10);
            timeout++;
        }
        sleep_ms(300);
    }

    // 测试4：音量与静音的交互
    printf("\n🔊 测试4: 音量与静音交互\n");
    audio_api->setMuted(false);
    
    uint8_t volumes[] = {20, 40, 60, 80, 100};
    for (int i = 0; i < 5; i++) {
        audio_api->setVolume(volumes[i]);
        printf("  音量%d%% - 播放测试音符\n", volumes[i]);
        
        audio_api->playNote(Notes::G4, 600, "SOL");
        int timeout = 0;
        while (audio_api->isPlaying() && timeout < 300) {
            audio_api->process();
            sleep_ms(10);
            timeout++;
        }
        sleep_ms(400);
    }

    // 最终状态
    printf("\n📊 最终状态报告:\n");
    printf("  静音状态: %s\n", audio_api->isMuted() ? "已静音" : "已解除静音");
    printf("  当前音量: %d%%\n", audio_api->getVolume());
    printf("  系统运行: %s\n", audio_api->isPlaying() ? "播放中" : "空闲");

    printf("\n✅ 静音控制测试完成！\n");
    printf("💡 如果您在\"解除静音\"状态下听不到声音:\n");
    printf("   1. 检查硬件连接\n");
    printf("   2. 检查PCM5102的XMT引脚连接(GPIO22)\n");
    printf("   3. 确认PCM5102供电正常\n");
    printf("   4. 检查音频输出设备(耳机/音箱)\n");

    return 0;
} 