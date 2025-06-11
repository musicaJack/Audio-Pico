#include <stdio.h>
#include <memory>

#include "pico/stdlib.h"
#include "AudioAPI.h"
#include "PicoAudioCore.h"

using namespace Audio;

/**
 * @brief 简单的API演示程序
 * 展示如何轻松集成和使用重构后的音频系统
 */
int main() {
    // 初始化标准I/O
    stdio_init_all();
    sleep_ms(1000);

    printf("\n=== 🎵 简单API演示程序 === \n");
    printf("展示C++音频框架的易用性\n");
    printf("=========================\n\n");

    // 步骤1：创建音频系统 - 只需3行代码！
    printf("📝 步骤1: 创建音频系统...\n");
    auto audio_core = std::make_unique<PicoAudioCore>();
    auto audio_api = std::make_unique<AudioAPI>(std::move(audio_core));
    
    // 步骤2：初始化音频系统 - 使用默认配置
    printf("📝 步骤2: 初始化音频系统...\n");
    if (!audio_api->initialize()) {
        printf("❌ 初始化失败\n");
        return -1;
    }
    printf("✅ 音频系统初始化成功\n");
    
    // 确保解除静音
    audio_api->setMuted(false);
    printf("🔊 静音状态: %s\n", audio_api->isMuted() ? "已静音" : "已解除静音");
    printf("🔊 当前音量: %d%%\n\n", audio_api->getVolume());

    // 步骤3：设置事件回调（可选）
    printf("📝 步骤3: 设置事件回调...\n");
    audio_api->setEventCallback([](const AudioEventData& event) {
        switch (event.event) {
            case AudioEvent::PLAYBACK_STARTED:
                printf("  🎵 开始播放: %s\n", event.message.c_str());
                break;
            case AudioEvent::PLAYBACK_STOPPED:
                printf("  ⏹️ 停止播放: %s\n", event.message.c_str());
                break;
            case AudioEvent::ERROR_OCCURRED:
                printf("  ❌ 错误: %s\n", event.message.c_str());
                break;
            default:
                break;
        }
    });
    printf("✅ 事件回调设置完成\n\n");

    printf("🎵 开始音频演示...\n\n");

    // 演示1：播放单个音符
    printf("🎼 演示1: 播放单个音符 (DO - 261.63Hz)\n");
    audio_api->playNote(Notes::C4, 1000, "DO");
    
    // 等待播放完成（添加超时保护）
    int timeout_counter = 0;
    while (audio_api->isPlaying() && timeout_counter < 500) { // 最多5秒
        audio_api->process();
        sleep_ms(10);
        timeout_counter++;
    }
    if (timeout_counter >= 500) {
        printf("⚠️ 播放超时，继续下一个演示\n");
    }
    sleep_ms(500);

    // 演示2：通过音符名称播放
    printf("🎼 演示2: 通过音符名称播放 (LA)\n");
    audio_api->playNoteByName("LA", 1000);
    
    timeout_counter = 0;
    while (audio_api->isPlaying() && timeout_counter < 500) {
        audio_api->process();
        sleep_ms(10);
        timeout_counter++;
    }
    sleep_ms(500);

    // 演示3：切换到正弦波并播放
    printf("🎼 演示3: 切换到正弦波音色并播放 (SOL)\n");
    audio_api->setWaveType(WaveType::SINE);
    audio_api->playNoteByName("SOL", 1000);
    
    timeout_counter = 0;
    while (audio_api->isPlaying() && timeout_counter < 500) {
        audio_api->process();
        sleep_ms(10);
        timeout_counter++;
    }
    sleep_ms(500);

    // 演示4：切换回钢琴音色
    printf("🎼 演示4: 切换回钢琴音色\n");
    audio_api->setWaveType(WaveType::PIANO);
    sleep_ms(500);

    // 演示5：播放完整的DO RE MI音阶
    printf("🎼 演示5: 播放完整的DO RE MI音阶\n");
    audio_api->playDoReMi(600, 100, false);
    
    timeout_counter = 0;
    while (audio_api->isPlaying() && timeout_counter < 1000) { // DO RE MI需要更长时间
        audio_api->process();
        sleep_ms(10);
        timeout_counter++;
    }
    sleep_ms(1000);

    // 演示6：音量控制
    printf("🎼 演示6: 音量控制演示\n");
    printf("  设置音量: 30%%\n");
    audio_api->setVolume(30);
    audio_api->playNoteByName("DO", 800);
    
    timeout_counter = 0;
    while (audio_api->isPlaying() && timeout_counter < 400) {
        audio_api->process();
        sleep_ms(10);
        timeout_counter++;
    }
    sleep_ms(300);

    printf("  设置音量: 80%%\n");
    audio_api->setVolume(80);
    audio_api->playNoteByName("DO", 800);
    
    timeout_counter = 0;
    while (audio_api->isPlaying() && timeout_counter < 400) {
        audio_api->process();
        sleep_ms(10);
        timeout_counter++;
    }
    sleep_ms(500);

    // 演示7：自定义音符序列
    printf("🎼 演示7: 自定义音符序列 (快乐生日片段)\n");
    MusicSequence custom_sequence = {
        {Notes::C4, 400, 100, 1.0f, "DO"},
        {Notes::C4, 200, 100, 1.0f, "DO"},
        {Notes::D4, 600, 100, 1.0f, "RE"},
        {Notes::C4, 600, 100, 1.0f, "DO"},
        {Notes::F4, 600, 100, 1.0f, "FA"},
        {Notes::E4, 800, 200, 1.0f, "MI"}
    };
    
    audio_api->playSequence(custom_sequence, false);
    
    timeout_counter = 0;
    while (audio_api->isPlaying() && timeout_counter < 800) { // 自定义序列较长
        audio_api->process();
        sleep_ms(10);
        timeout_counter++;
    }
    sleep_ms(1000);

    // 演示8：显示系统状态
    printf("🎼 演示8: 系统状态信息\n");
    printf("  当前音量: %d%%\n", audio_api->getVolume());
    printf("  静音状态: %s\n", audio_api->isMuted() ? "已静音" : "已解除静音");
    printf("  当前波形: %s\n", 
           audio_api->getWaveType() == WaveType::PIANO ? "钢琴音色" : "正弦波");
    printf("  播放状态: %s\n", 
           audio_api->isPlaying() ? "播放中" : "停止");
    
    printf("\n📋 支持的预设音符:\n");
    printf("  DO: 261.6Hz  RE: 293.7Hz  MI: 329.6Hz\n");
    printf("  FA: 349.2Hz  SOL: 392.0Hz LA: 440.0Hz\n");
    printf("  SI: 493.9Hz  DO5: 523.3Hz\n");

    printf("\n✅ 演示完成！\n");
    printf("💡 集成总结:\n");
    printf("  - 只需包含 AudioAPI.h 和 PicoAudioCore.h\n");
    printf("  - 3行代码即可创建完整音频系统\n");
    printf("  - 支持事件回调、音量控制、多种波形\n");
    printf("  - 易于扩展和迁移到其他平台\n");
    printf("  - 基于现代C++特性，类型安全\n\n");

    printf("🎵 程序结束，感谢使用！\n");

    return 0;
} 