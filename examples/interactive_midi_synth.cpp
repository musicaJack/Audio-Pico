#include <stdio.h>
#include <memory>
#include <map>
#include <cstdio>
#include <cctype>

#include "pico/stdlib.h"
#include "AudioAPI.hpp"
#include "PicoAudioCore.hpp"

using namespace Audio;

/**
 * @brief 交互式MIDI电子合成器
 * 支持键盘输入1-7播放不同音符
 * Shift + 1-7: 低音区（第3八度）
 * 1-7: 标准音区（第4八度）  
 * Alt + 1-7: 高音区（第5八度）
 */
class InteractiveMIDISynth {
private:
    std::unique_ptr<AudioAPI> audio_api;
    bool shift_pressed = false;
    bool alt_pressed = false;
    bool ctrl_pressed = false;
    uint8_t current_octave = 4;
    WaveType current_wave = WaveType::PIANO;
    bool running = true;
    
    // LRU音频资源管理
    struct AudioResource {
        float frequency;
        std::string name;
        uint32_t last_used_time;
        bool in_use;
    };
    
    static constexpr size_t MAX_CACHED_RESOURCES = 12; // 最多缓存12个音频资源
    std::vector<AudioResource> cached_resources;
    uint32_t current_time_counter = 0;

    // 音符频率映射 - 支持3个八度
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
    InteractiveMIDISynth() {
        auto audio_core = std::make_unique<PicoAudioCore>();
        audio_api = std::make_unique<AudioAPI>(std::move(audio_core));
    }

    bool initialize() {
        printf("\n🎹 === 交互式MIDI电子合成器 === 🎹\n");
        printf("正在初始化音频系统（内存优化版）...\n");
        
        // 创建平衡的音频配置（音质与内存的平衡）
        AudioConfig optimized_config;
        optimized_config.sample_rate = 32000;    // 适中的采样率32kHz（保持音质）
        optimized_config.channels = 2;           // 保持立体声
        optimized_config.bit_depth = 16;         // 保持16位
        optimized_config.buffer_size = 768;      // 适中的缓冲区大小
        
        if (!audio_api->initialize(optimized_config)) {
            printf("❌ 音频系统初始化失败\n");
            return false;
        }
        
        // 设置默认参数
        audio_api->setMuted(false);
        audio_api->setVolume(70);
        audio_api->setWaveType(current_wave);
        
        // 设置轻量级事件回调
        audio_api->setEventCallback([this](const AudioEventData& event) {
            switch (event.event) {
                case AudioEvent::PLAYBACK_STARTED:
                    printf("🎵 %s\n", event.message.c_str());
                    break;
                case AudioEvent::ERROR_OCCURRED:
                    printf("❌ %s\n", event.message.c_str());
                    break;
                default:
                    break;
            }
        });
        
        printf("✅ 音频系统初始化成功（32kHz立体声）\n");
        printf("💾 内存优化: 采样率32kHz, 立体声, 768样本缓冲\n");
        return true;
    }

    void printHelp() {
        printf("\n📖 === 操作说明 === 📖\n");
        printf("🎹 组合键控制 (推荐方式):\n");
        printf("  [         : 激活Shift模式 (按下后再按1-7播放低音区)\n");
        printf("  ]         : 激活Alt模式 (按下后再按1-7播放高音区)\n");
        printf("  1-7       : 播放当前八度的音符 (DO RE MI FA SOL LA SI)\n");
        printf("  ESC       : 取消组合键状态\n");
        printf("\n🎹 直接按键 (兼容方式):\n");
        printf("  W E R T Y U I : 低音区 DO RE MI FA SOL LA SI\n");
        printf("  C F G J K L ; : 标准区 DO RE MI FA SOL LA SI\n"); 
        printf("  Z X V B N   : 高音区 DO RE MI FA SOL\n");
        printf("\n🎛️ 功能控制:\n");
        printf("  W         : 切换波形 (钢琴音色 ↔ 正弦波)\n");
        printf("  +/-       : 音量调节 (+10/-10)\n");
        printf("  M         : 静音/解除静音\n");
        printf("  O         : 切换八度 (3/4/5)\n");
        printf("  D         : 播放当前八度的DO RE MI音阶\n");
        printf("  S         : 停止当前播放\n");
        printf("  I         : 显示内存使用信息\n");
        printf("  H/?       : 显示帮助\n");
        printf("  Q         : 退出程序\n");
        printf("\n🎼 当前状态:\n");
        printf("  八度: %d  音量: %d%%  波形: %s  静音: %s\n", 
               current_octave, 
               audio_api->getVolume(),
               current_wave == WaveType::PIANO ? "钢琴音色" : "正弦波",
               audio_api->isMuted() ? "是" : "否");
        printf("  组合键状态: %s\n", 
               shift_pressed ? "Shift激活" : 
               alt_pressed ? "Alt激活" : "无");
        printf("=====================================\n\n");
    }

    // LRU资源管理方法
    void updateResourceCache(float frequency, const std::string& name) {
        current_time_counter++;
        
        // 查找是否已经缓存
        for (auto& resource : cached_resources) {
            if (std::abs(resource.frequency - frequency) < 0.1f) {
                resource.last_used_time = current_time_counter;
                resource.in_use = true;
                return;
            }
        }
        
        // 如果缓存已满，移除最旧的资源
        if (cached_resources.size() >= MAX_CACHED_RESOURCES) {
            auto oldest = std::min_element(cached_resources.begin(), cached_resources.end(),
                [](const AudioResource& a, const AudioResource& b) {
                    return a.last_used_time < b.last_used_time;
                });
            
            if (oldest != cached_resources.end()) {
                printf("💾 LRU: 释放音频资源 %s (%.1fHz)\n", oldest->name.c_str(), oldest->frequency);
                cached_resources.erase(oldest);
            }
        }
        
        // 添加新资源
        cached_resources.push_back({frequency, name, current_time_counter, true});
        printf("💾 LRU: 缓存音频资源 %s (%.1fHz)\n", name.c_str(), frequency);
    }
    
    void releaseUnusedResources() {
        // 更智能的资源释放策略
        for (auto& resource : cached_resources) {
            if (resource.in_use && (current_time_counter - resource.last_used_time) > 15) {
                resource.in_use = false;
                // 只在调试时显示详细信息
                #ifdef DEBUG_LRU
                printf("💾 LRU: 标记资源为未使用 %s\n", resource.name.c_str());
                #endif
            }
        }
        
        // 如果缓存接近满载，主动清理最旧的未使用资源
        if (cached_resources.size() > (MAX_CACHED_RESOURCES * 3 / 4)) {
            auto it = std::remove_if(cached_resources.begin(), cached_resources.end(),
                [this](const AudioResource& resource) {
                    return !resource.in_use && 
                           (current_time_counter - resource.last_used_time) > 8;
                });
            
            if (it != cached_resources.end()) {
                #ifdef DEBUG_LRU
                size_t removed = cached_resources.end() - it;
                #endif
                cached_resources.erase(it, cached_resources.end());
                #ifdef DEBUG_LRU
                printf("💾 LRU: 清理了 %zu 个未使用资源\n", removed);
                #endif
            }
        }
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
        
        // 更新LRU缓存
        updateResourceCache(frequency, note_name);
        
        // 停止当前播放的音符（立即释放资源）
        if (audio_api->isPlaying()) {
            audio_api->stop();
        }
        
        // 播放新音符 (持续300ms)
        audio_api->playNote(frequency, 300, note_name);
        
        // 定期清理未使用的资源
        if (current_time_counter % 20 == 0) {
            releaseUnusedResources();
        }
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

    void printMemoryInfo() {
        printf("\n💾 === 内存使用信息 === 💾\n");
        printf("音频配置:\n");
        printf("  采样率: 32000 Hz (平衡优化)\n");
        printf("  声道数: 2 (立体声)\n");
        printf("  缓冲区: 768 样本 (平衡优化)\n");
        printf("\nLRU缓存状态:\n");
        printf("  缓存资源: %zu/%zu\n", cached_resources.size(), MAX_CACHED_RESOURCES);
        printf("  当前时间: %lu\n", (unsigned long)current_time_counter);
        
        if (!cached_resources.empty()) {
            printf("  已缓存音符:\n");
            for (const auto& resource : cached_resources) {
                printf("    %s (%.1fHz) - %s, 时间:%lu\n", 
                       resource.name.c_str(), 
                       resource.frequency,
                       resource.in_use ? "使用中" : "空闲",
                       (unsigned long)resource.last_used_time);
            }
        }
        printf("===============================\n\n");
    }

    void playDoReMiScale() {
        printf("🎵 播放当前八度的DO RE MI音阶...\n");
        
        // 创建当前八度的音阶序列（优化：减少持续时间）
        MusicSequence scale_sequence;
        for (int i = 1; i <= 7; i++) {
            if (note_frequencies[current_octave].find(i) != note_frequencies[current_octave].end()) {
                scale_sequence.push_back({
                    note_frequencies[current_octave][i],
                    300,  // 持续时间（优化：从500ms减少到300ms）
                    50,   // 暂停时间（优化：从100ms减少到50ms）
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

        // 处理组合键状态
        // 检查是否为数字键 1-7
        if (ch >= '1' && ch <= '7') {
            int note_num = ch - '0';
            int target_octave = current_octave;
            
            // 根据当前组合键状态决定八度
            if (shift_pressed) {
                target_octave = 3; // 低音区
                printf("🎵 Shift+%d -> ", note_num);
                shift_pressed = false; // 重置状态
            } else if (alt_pressed) {
                target_octave = 5; // 高音区
                printf("🎵 Alt+%d -> ", note_num);
                alt_pressed = false; // 重置状态
            } else {
                target_octave = current_octave; // 当前八度
                printf("🎵 %d -> ", note_num);
            }
            
            playNote(note_num, target_octave);
        }
        // 检查Shift键 (使用左方括号 [ 作为Shift键)
        else if (ch == '[') {
            shift_pressed = true;
            printf("🔄 Shift模式激活 - 请按1-7播放低音区音符\n");
        }
        // 检查Alt键 (使用右方括号 ] 作为Alt键)
        else if (ch == ']') {
            alt_pressed = true;
            printf("🔄 Alt模式激活 - 请按1-7播放高音区音符\n");
        }
        // ESC键取消组合键状态
        else if (ch == 27) { // ESC键
            shift_pressed = false;
            alt_pressed = false;
            printf("🔄 组合键状态已重置\n");
        }
        // 兼容原有的字母键快捷方式
        // 低音区快捷键 (q-u 对应 1-7)
        else if (ch >= 'q' && ch <= 'u' && ch != 'q') { // 避免与退出键冲突
            int note_num = ch - 'q' + 1;
            if (note_num <= 7) {
                playNote(note_num, 3); // 第3八度
            }
        }
        // 标准音区快捷键 (a-j 对应 1-7，避免与功能键冲突) 
        else if ((ch >= 'a' && ch <= 'j') && ch != 'a' && ch != 'd' && ch != 'h' && ch != 'm') {
            int note_num = ch - 'a' + 1;
            if (note_num <= 7) {
                playNote(note_num, 4); // 第4八度
            }
        }
        // 高音区快捷键 (z-m的部分键，避免功能键冲突)
        else if ((ch >= 'z' && ch <= 'n') && ch != 'm') {
            int note_num = ch - 'z' + 1;
            if (note_num <= 7) {
                playNote(note_num, 5); // 第5八度
            }
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
                case 'i':
                    printMemoryInfo();
                    break;
                case 'q':
                    running = false;
                    printf("👋 退出合成器...\n");
                    break;
                default:
                    // 如果处于组合键状态，提示用户
                    if (shift_pressed || alt_pressed) {
                        printf("⚠️ 组合键状态激活中，请按1-7或ESC取消\n");
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
        printf("🎹 合成器已就绪！请开始演奏...\n");
        printf("💡 使用方法:\n");
        printf("   方法1: 先按 [ 或 ] 激活组合键模式，再按 1-7\n");
        printf("         [ + 1-7 = 低音区音符\n");
        printf("         ] + 1-7 = 高音区音符\n");
        printf("         直接按 1-7 = 当前八度音符\n");
        printf("   方法2: 直接使用字母键快捷方式 (见帮助信息)\n");
        printf("   按 H 查看完整帮助，按 I 查看内存状态，按 Q 退出\n");
        printf("🔧 当前模式: 平衡模式 (32kHz立体声，节省内存同时保持音质)\n\n");

        uint32_t loop_counter = 0;
        while (running) {
            // 处理音频系统
            audio_api->process();
            
            // 处理键盘输入
            processInput();
            
            // 每100次循环进行一次内存清理
            if (++loop_counter % 100 == 0) {
                releaseUnusedResources();
                
                // 每1000次循环显示内存状态
                if (loop_counter % 1000 == 0) {
                    printf("💾 缓存状态: %zu/%zu 资源\n", 
                           cached_resources.size(), MAX_CACHED_RESOURCES);
                }
            }
            
            // 短暂延迟以避免过度占用CPU
            sleep_ms(10);
        }
    }
};

int main() {
    // 初始化标准I/O
    stdio_init_all();
    sleep_ms(2000); // 等待串口连接稳定

    InteractiveMIDISynth synth;
    synth.run();

    return 0;
} 