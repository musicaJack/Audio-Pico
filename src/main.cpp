#include "AudioAPI.h"
#include "pico/stdlib.h"
#include <memory>
#include <string>

using namespace Audio;

// 事件回调函数
void onAudioEvent(const AudioEventData& event) {
    switch (event.event) {
        case AudioEvent::PLAYBACK_STARTED:
            printf("🎵 %s\n", event.message.c_str());
            break;
        case AudioEvent::PLAYBACK_STOPPED:
            printf("⏹️ %s\n", event.message.c_str());
            break;
        case AudioEvent::PLAYBACK_PAUSED:
            printf("⏸️ %s\n", event.message.c_str());
            break;
        case AudioEvent::NOTE_CHANGED:
            printf("🎼 %s\n", event.message.c_str());
            break;
        case AudioEvent::VOLUME_CHANGED:
            printf("🔊 %s (音量: %ld)\n", event.message.c_str(), static_cast<long>(event.value));
            break;
        case AudioEvent::ERROR_OCCURRED:
            printf("❌ 错误: %s\n", event.message.c_str());
            break;
    }
}

int main() {
    // 初始化标准I/O
    stdio_init_all();
    
    printf("\n");
    printf("================================================\n");
    printf("🎵 Raspberry Pi Pico C++ 音频框架演示程序\n");
    printf("================================================\n");
    printf("版本: 3.0\n");
    printf("架构: 面向对象 C++17\n");
    printf("硬件: I2S音频输出 (PCM5102)\n");
    printf("引脚: DIN=26, BCLK=27, LRCLK=28, XMT=22\n");
    printf("================================================\n\n");

    // 等待USB连接稳定
    sleep_ms(2000);

    // 创建Pico音频核心
    PicoI2SConfig i2s_config;
    i2s_config.data_pin = 26;
    i2s_config.clock_pin_base = 27;
    i2s_config.mute_pin = 22;
    i2s_config.enable_mute_control = true;

    auto audio_core = std::make_unique<PicoAudioCore>(i2s_config);
    
    // 创建音频API
    AudioAPI audio_api(std::move(audio_core));
    
    // 设置事件回调
    audio_api.setEventCallback(onAudioEvent);

    // 初始化音频系统
    AudioConfig config;
    config.sample_rate = 44100;
    config.channels = 2;
    config.bit_depth = 16;
    config.buffer_size = 1156;

    printf("🔧 正在初始化音频系统...\n");
    if (!audio_api.initialize(config)) {
        printf("❌ 音频系统初始化失败！\n");
        return -1;
    }
    printf("✅ 音频系统初始化成功！\n\n");

    // 设置音量和音色
    audio_api.setVolume(80);
    audio_api.setWaveType(WaveType::PIANO);
    audio_api.setMuted(false); // 确保不静音

    printf("🎹 当前设置:\n");
    printf("   📢 音量: %d/100\n", audio_api.getVolume());
    printf("   🎵 音色: 钢琴\n");
    printf("   🔇 静音: %s\n", audio_api.isMuted() ? "开启" : "关闭");
    printf("\n");

    // 演示1: 播放DO RE MI音阶
    printf("🎼 演示1: 播放完整DO RE MI音阶\n");
    if (audio_api.playDoReMi(600, 200)) {
        // 等待播放完成
        while (audio_api.isPlaying()) {
            sleep_ms(100);
            audio_api.process();
        }
        sleep_ms(1000);
    }

    // 演示2: 播放单个音符
    printf("\n🎼 演示2: 播放单个音符 (LA - 440Hz)\n");
    if (audio_api.playNoteByName("LA", 1000)) {
        while (audio_api.isPlaying()) {
            sleep_ms(100);
            audio_api.process();
        }
        sleep_ms(500);
    }

    // 演示3: 切换音色演示
    printf("\n🎼 演示3: 切换到正弦波音色\n");
    audio_api.setWaveType(WaveType::SINE);
    if (audio_api.playNoteByName("SOL", 800)) {
        while (audio_api.isPlaying()) {
            sleep_ms(100);
            audio_api.process();
        }
        sleep_ms(500);
    }

    // 演示4: 音量控制演示
    printf("\n🎼 演示4: 音量渐变演示\n");
    for (int vol = 20; vol <= 100; vol += 20) {
        printf("   🔊 设置音量: %d%%\n", vol);
        audio_api.setVolume(vol);
        audio_api.playNote(Notes::C4, 400);
        while (audio_api.isPlaying()) {
            sleep_ms(50);
            audio_api.process();
        }
        sleep_ms(200);
    }

    // 演示5: 静音控制演示
    printf("\n🎼 演示5: 静音控制演示\n");
    audio_api.setVolume(80);
    printf("   🔊 正常播放\n");
    audio_api.playNote(Notes::E4, 500);
    while (audio_api.isPlaying()) {
        sleep_ms(50);
        audio_api.process();
    }
    
    printf("   🔇 开启静音\n");
    audio_api.setMuted(true);
    audio_api.playNote(Notes::E4, 500);
    while (audio_api.isPlaying()) {
        sleep_ms(50);
        audio_api.process();
    }
    
    printf("   🔊 关闭静音\n");
    audio_api.setMuted(false);
    audio_api.playNote(Notes::E4, 500);
    while (audio_api.isPlaying()) {
        sleep_ms(50);
        audio_api.process();
    }

    // 演示6: 自定义音符序列
    printf("\n🎼 演示6: 自定义小曲 (小星星)\n");
    MusicSequence twinkle_star;
    // 小星星的音符序列
    const std::vector<std::pair<float, std::string>> star_notes = {
        {Notes::C4, "DO"}, {Notes::C4, "DO"}, {Notes::G4, "SOL"}, {Notes::G4, "SOL"},
        {Notes::A4, "LA"}, {Notes::A4, "LA"}, {Notes::G4, "SOL"},
        {Notes::F4, "FA"}, {Notes::F4, "FA"}, {Notes::E4, "MI"}, {Notes::E4, "MI"},
        {Notes::D4, "RE"}, {Notes::D4, "RE"}, {Notes::C4, "DO"}
    };
    
    for (const auto& [freq, name] : star_notes) {
        twinkle_star.emplace_back(freq, 400, 100, 1.0f, name);
    }
    
    audio_api.setWaveType(WaveType::PIANO);
    if (audio_api.playSequence(twinkle_star)) {
        while (audio_api.isPlaying()) {
            sleep_ms(100);
            audio_api.process();
        }
    }

    printf("\n✨ 演示完成！\n");
    printf("🔧 进入音频处理循环... (按复位键重启)\n\n");

    // 主循环 - 保持音频系统运行
    while (true) {
        audio_api.process();
        sleep_ms(10);
    }

    return 0;
} 