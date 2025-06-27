#include <stdio.h>
#include <memory>
#include <map>
#include <cstdio>
#include <cctype>
#include <vector>
#include <cmath>
#include <algorithm>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "AudioAPI.hpp"
#include "PicoAudioCore.hpp"

using namespace Audio;

/**
 * @brief 简化版交互式MIDI电子合成器
 * 只支持数字按键1-7和组合键控制
 * 去掉显示器和字母按键功能
 */
class SimpleMIDISynth {
private:
    std::unique_ptr<AudioAPI> audio_api;
    bool shift_pressed = false;  // [ 键激活低音区
    bool alt_pressed = false;    // ] 键激活高音区
    uint8_t current_octave = 4;
    WaveType current_wave = WaveType::PIANO;
    bool running = true;

    // 简化的音符频率映射 - 只支持3个八度
    std::map<int, std::map<int, float>> note_frequencies = {
        // 第3八度（低音区）
        {3, {
            {1, 130.81f}, // C3 - 低音DO
            {2, 146.83f}, // D3 - 低音RE
            {3, 164.81f}, // E3 - 低音MI
            {4, 174.61f}, // F3 - 低音FA
            {5, 196.00f}, // G3 - 低音SOL
            {6, 220.00f}, // A3 - 低音LA
            {7, 246.94f}  // B3 - 低音SI
        }},
        // 第4八度（标准音区）
        {4, {
            {1, 261.63f}, // C4 - DO
            {2, 293.66f}, // D4 - RE
            {3, 329.63f}, // E4 - MI
            {4, 349.23f}, // F4 - FA
            {5, 392.00f}, // G4 - SOL
            {6, 440.00f}, // A4 - LA
            {7, 493.88f}  // B4 - SI
        }},
        // 第5八度（高音区）
        {5, {
            {1, 523.25f}, // C5 - 高音DO
            {2, 587.33f}, // D5 - 高音RE
            {3, 659.25f}, // E5 - 高音MI
            {4, 698.46f}, // F5 - 高音FA
            {5, 783.99f}, // G5 - 高音SOL
            {6, 880.00f}, // A5 - 高音LA
            {7, 987.77f}  // B5 - 高音SI
        }}
    };

    // 音符名称映射
    std::map<int, std::map<int, std::string>> note_names = {
        {3, {{1, "低音DO"}, {2, "低音RE"}, {3, "低音MI"}, {4, "低音FA"}, {5, "低音SOL"}, {6, "低音LA"}, {7, "低音SI"}}},
        {4, {{1, "DO"}, {2, "RE"}, {3, "MI"}, {4, "FA"}, {5, "SOL"}, {6, "LA"}, {7, "SI"}}},
        {5, {{1, "高音DO"}, {2, "高音RE"}, {3, "高音MI"}, {4, "高音FA"}, {5, "高音SOL"}, {6, "高音LA"}, {7, "高音SI"}}}
    };

public:
    SimpleMIDISynth() {
        auto audio_core = std::make_unique<PicoAudioCore>();
        audio_api = std::make_unique<AudioAPI>(std::move(audio_core));
    }

    bool initialize() {
        printf("\n🎹 === 简化版MIDI电子合成器 === 🎹\n");
        printf("正在初始化音频系统...\n");
        
        // 创建轻量级音频配置
        AudioConfig config;
        config.sample_rate = 22050;    // 降低采样率节省内存
        config.channels = 2;           // 保持立体声
        config.bit_depth = 16;         // 16位音频
        config.buffer_size = 512;      // 较小的缓冲区
        
        if (!audio_api->initialize(config)) {
            printf("❌ 音频系统初始化失败\n");
            return false;
        }
        
        // 设置默认参数
        audio_api->setMuted(false);
        audio_api->setVolume(70);
        audio_api->setWaveType(current_wave);
        
        // 设置简单的事件回调
        audio_api->setEventCallback([](const AudioEventData& event) {
            if (event.event == AudioEvent::PLAYBACK_STARTED) {
                printf("🎵 %s\n", event.message.c_str());
            } else if (event.event == AudioEvent::ERROR_OCCURRED) {
                printf("❌ %s\n", event.message.c_str());
            }
        });
        
        printf("✅ 音频系统初始化成功（22kHz立体声）\n");
        return true;
    }

    void printHelp() {
        printf("\n📖 === 操作说明 === 📖\n");
        printf("🎹 数字按键控制:\n");
        printf("  [         : 激活低音模式 (按下后再按1-7播放低音区)\n");
        printf("  ]         : 激活高音模式 (按下后再按1-7播放高音区)\n");
        printf("  1-7       : 播放音符 (DO RE MI FA SOL LA SI)\n");
        printf("  ESC       : 取消组合键状态\n");
        printf("\n🎛️ 功能控制:\n");
        printf("  W         : 切换波形 (钢琴音色 ↔ 正弦波)\n");
        printf("  +/-       : 音量调节 (+10/-10)\n");
        printf("  M         : 静音/解除静音\n");
        printf("  O         : 切换八度 (3/4/5)\n");
        printf("  D         : 播放当前八度的DO RE MI音阶\n");
        printf("  S         : 停止当前播放\n");
        printf("  H/?       : 显示帮助\n");
        printf("  Q         : 退出程序\n");
        printf("\n🎼 当前状态:\n");
        printf("  八度: %d  音量: %d%%  波形: %s  静音: %s\n", 
               current_octave, 
               audio_api->getVolume(),
               current_wave == WaveType::PIANO ? "钢琴音色" : "正弦波",
               audio_api->isMuted() ? "是" : "否");
        printf("  组合键状态: %s\n", 
               shift_pressed ? "低音模式激活" : 
               alt_pressed ? "高音模式激活" : "标准模式");
        printf("=====================================\n\n");
    }

    void playNote(int note_num, int octave = -1) {
        if (octave == -1) {
            octave = current_octave;
        }
        
        if (note_frequencies.find(octave) == note_frequencies.end() ||
            note_frequencies[octave].find(note_num) == note_frequencies[octave].end()) {
            printf("❌ 无效的音符: %d (八度: %d)\n", note_num, octave);
            return;
        }
        
        float frequency = note_frequencies[octave][note_num];
        std::string note_name = note_names[octave][note_num];
        
        // 停止当前播放（避免重叠）
        if (audio_api->isPlaying()) {
            audio_api->stop();
        }
        
        // 播放新音符
        audio_api->playNote(frequency, 400, note_name);
    }

    void handleVolumeChange(int delta) {
        int current_volume = audio_api->getVolume();
        int new_volume = current_volume + delta;
        new_volume = std::max(0, std::min(100, new_volume));
        audio_api->setVolume(new_volume);
        printf("🔊 音量调节: %d%% → %d%%\n", current_volume, new_volume);
    }

    void toggleWave() {
        current_wave = (current_wave == WaveType::PIANO) ? WaveType::SINE : WaveType::PIANO;
        audio_api->setWaveType(current_wave);
        printf("🎛️ 波形切换: %s\n", current_wave == WaveType::PIANO ? "钢琴音色" : "正弦波");
    }

    void toggleMute() {
        audio_api->toggleMute();
        printf("🔇 静音状态: %s\n", audio_api->isMuted() ? "已静音" : "已解除静音");
    }

    void switchOctave() {
        current_octave++;
        if (current_octave > 5) {
            current_octave = 3;
        }
        printf("🎼 八度切换: %d (%s)\n", current_octave, 
               current_octave == 3 ? "低音区" : 
               current_octave == 4 ? "标准音区" : "高音区");
    }

    void playDoReMiScale() {
        printf("🎵 播放当前八度的DO RE MI音阶...\n");
        
        // 创建当前八度的音阶序列
        MusicSequence scale_sequence;
        for (int i = 1; i <= 7; i++) {
            if (note_frequencies[current_octave].find(i) != note_frequencies[current_octave].end()) {
                scale_sequence.push_back({
                    note_frequencies[current_octave][i],
                    350,  // 持续时间
                    50,   // 暂停时间
                    1.0f, // 音量
                    note_names[current_octave][i]
                });
            }
        }
        
        audio_api->playSequence(scale_sequence, false);
    }

    void processInput() {
        int ch = getchar_timeout_us(0); // 非阻塞读取
        if (ch == PICO_ERROR_TIMEOUT) {
            return;
        }

        // 处理数字键 1-7
        if (ch >= '1' && ch <= '7') {
            int note_num = ch - '0';
            int target_octave = current_octave;
            
            // 根据当前组合键状态决定八度
            if (shift_pressed) {
                target_octave = 3; // 低音区
                printf("🎵 [+%d -> ", note_num);
                shift_pressed = false; // 重置状态
            } else if (alt_pressed) {
                target_octave = 5; // 高音区
                printf("🎵 ]+%d -> ", note_num);
                alt_pressed = false; // 重置状态
            } else {
                target_octave = current_octave; // 当前八度
                printf("🎵 %d -> ", note_num);
            }
            
            playNote(note_num, target_octave);
        }
        // 处理组合键
        else if (ch == '[') {
            shift_pressed = true;
            alt_pressed = false; // 确保只有一个模式激活
            printf("🔄 低音模式激活 - 请按1-7播放低音区音符\n");
        }
        else if (ch == ']') {
            alt_pressed = true;
            shift_pressed = false; // 确保只有一个模式激活
            printf("🔄 高音模式激活 - 请按1-7播放高音区音符\n");
        }
        // ESC键取消组合键状态
        else if (ch == 27) { // ESC键
            shift_pressed = false;
            alt_pressed = false;
            printf("🔄 组合键状态已重置为标准模式\n");
        }
        // 功能控制键
        else {
            switch (tolower(ch)) {
                case 'w':
                    toggleWave();
                    break;
                case '+':
                case '=':
                    handleVolumeChange(10);
                    break;
                case '-':
                case '_':
                    handleVolumeChange(-10);
                    break;
                case 'm':
                    toggleMute();
                    break;
                case 'o':
                    switchOctave();
                    break;
                case 'd':
                    playDoReMiScale();
                    break;
                case 's':
                    audio_api->stop();
                    printf("⏹️ 停止播放\n");
                    break;
                case 'h':
                case '?':
                    printHelp();
                    break;
                case 'q':
                    running = false;
                    printf("👋 退出合成器...\n");
                    break;
                default:
                    // 如果处于组合键状态，提示用户
                    if (shift_pressed || alt_pressed) {
                        printf("⚠️ %s模式激活中，请按1-7或ESC取消\n", 
                               shift_pressed ? "低音" : "高音");
                    }
                    break;
            }
        }
    }

    void run() {
        if (!initialize()) {
            return;
        }
        
        printHelp();
        printf("🎹 简化版合成器已就绪！\n");
        printf("💡 使用方法:\n");
        printf("   1. 先按 [ 激活低音模式，再按 1-7 播放低音区\n");
        printf("   2. 先按 ] 激活高音模式，再按 1-7 播放高音区\n");
        printf("   3. 直接按 1-7 播放当前八度的音符\n");
        printf("   4. 按 H 查看完整帮助，按 Q 退出\n");
        printf("🔧 当前配置: 22kHz立体声，轻量级模式\n\n");

        while (running) {
            // 处理音频系统
            audio_api->process();
            
            // 处理键盘输入
            processInput();
            
            // 短暂延迟以避免过度占用CPU
            sleep_ms(10);
        }
    }
};

int main() {
    // 初始化标准I/O
    stdio_init_all();
    sleep_ms(2000); // 等待串口连接稳定

    printf("🚀 启动简化版MIDI合成器\n");
    printf("🔧 轻量级 + 数字按键控制\n");
    printf("⏰ 等待硬件初始化...\n");
    sleep_ms(1000);

    SimpleMIDISynth synth;
    synth.run();

    return 0;
} 