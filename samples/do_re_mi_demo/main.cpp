/**
 * DO RE MI 音阶演示程序
 * 使用 Pico-extras 官方音频库
 * 
 * 硬件连接：
 * GPIO 26 -> DIN   (数据输入)
 * GPIO 27 -> BCLK  (位时钟)
 * GPIO 28 -> LRCLK (左右声道时钟)
 * GPIO 22 -> XMT   (PCM5102静音控制，高电平解除静音)
 */

#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/audio.h"
#include "pico/audio_i2s.h"
#include "hardware/gpio.h"

// PCM5102 XMT静音控制引脚
#define PCM5102_XMT_PIN 22

// 音阶频率定义 (DO RE MI FA SOL LA SI)
static const float note_frequencies[] = {
    261.63f,  // DO (C4)
    293.66f,  // RE (D4)  
    329.63f,  // MI (E4)
    349.23f,  // FA (F4)
    392.00f,  // SOL (G4)
    440.00f,  // LA (A4)
    493.88f   // SI (B4)
};

static const char* note_names[] = {
    "DO", "RE", "MI", "FA", "SOL", "LA", "SI"
};

#define NUM_NOTES (sizeof(note_frequencies) / sizeof(note_frequencies[0]))
#define SINE_WAVE_TABLE_LEN 2048
#define SAMPLES_PER_BUFFER 1156

// ============================================================================
// 🎵 音频播放时间配置 (方便调试修改)
// ============================================================================
#define DEFAULT_NOTE_DURATION_MS 100     // 默认音符播放时间 (毫秒)
#define DEFAULT_PAUSE_DURATION_MS 50     // 默认音符间暂停时间 (毫秒)

#define SPEED_FAST_NOTE_MS 5            // 快速模式音符时间
#define SPEED_FAST_PAUSE_MS 5           // 快速模式暂停时间

#define SPEED_MEDIUM_NOTE_MS 10         // 中速模式音符时间  
#define SPEED_MEDIUM_PAUSE_MS 5         // 中速模式暂停时间

#define SPEED_SLOW_NOTE_MS 20           // 慢速模式音符时间
#define SPEED_SLOW_PAUSE_MS 5           // 慢速模式暂停时间

// ============================================================================
// 🎹 钢琴音色参数
// ============================================================================
#define NUM_HARMONICS 6              // 谐波数量
#define ATTACK_SAMPLES (44100 * 20 / 1000)   // 20ms 攻击时间
#define DECAY_SAMPLES (44100 * 100 / 1000)   // 100ms 衰减时间
#define SUSTAIN_LEVEL 0.4f           // 持续音量 (40%)
#define RELEASE_SAMPLES (44100 * 200 / 1000) // 200ms 释放时间

// 全局变量
static int16_t sine_wave_table[SINE_WAVE_TABLE_LEN];
static audio_buffer_pool_t *audio_pool;
static uint32_t current_note = 0;
static uint32_t note_duration_ms = DEFAULT_NOTE_DURATION_MS;   // 音符播放时间
static uint32_t pause_duration_ms = DEFAULT_PAUSE_DURATION_MS; // 音符间暂停时间
static uint32_t last_note_change = 0;
static uint32_t volume = 80;
static bool is_playing_note = true;       // 当前是否在播放音符（false表示在暂停）

// 钢琴音色相关变量
static uint32_t note_sample_count = 0;   // 当前音符已播放的采样数
static float harmonic_amplitudes[NUM_HARMONICS] = {1.0f, 0.5f, 0.3f, 0.2f, 0.15f, 0.1f}; // 谐波强度
static bool piano_mode = true;           // true=钢琴音色, false=纯正弦波
static bool is_muted = false;            // PCM5102静音状态
static bool auto_play = false;          // 自动播放模式
static bool note_playing = false;       // 当前是否有音符在播放

// 音频格式配置
static audio_format_t audio_format = {
    .sample_freq = 44100,
    .format = AUDIO_BUFFER_FORMAT_PCM_S16,
    .channel_count = 2
};

static audio_buffer_format_t producer_format = {
    .format = &audio_format,
    .sample_stride = 4
};

// I2S配置
static const audio_i2s_config_t i2s_config = {
    .data_pin = 26,          // GPIO 26 -> DIN
    .clock_pin_base = 27,    // GPIO 27 -> BCLK, GPIO 28 -> LRCLK
    .dma_channel = 0,
    .pio_sm = 0
};

// 根据频率计算正弦波步进值
static uint32_t frequency_to_step(float frequency) {
    return (uint32_t)((frequency * SINE_WAVE_TABLE_LEN * 65536.0f) / 44100.0f);
}

// 计算包络值 (ADSR)
static float calculate_envelope(uint32_t sample_position) {
    if (sample_position < ATTACK_SAMPLES) {
        // Attack: 线性上升到峰值
        return (float)sample_position / ATTACK_SAMPLES;
    } else if (sample_position < ATTACK_SAMPLES + DECAY_SAMPLES) {
        // Decay: 从峰值衰减到持续音量
        uint32_t decay_pos = sample_position - ATTACK_SAMPLES;
        float decay_ratio = (float)decay_pos / DECAY_SAMPLES;
        return 1.0f - decay_ratio * (1.0f - SUSTAIN_LEVEL);
    } else {
        // Sustain: 保持持续音量
        return SUSTAIN_LEVEL;
    }
}

// 生成钢琴音色（多谐波合成）
static int16_t generate_piano_sample(uint32_t phase, float envelope) {
    float sample = 0.0f;
    
    // 合成多个谐波
    for (int h = 0; h < NUM_HARMONICS; h++) {
        uint32_t harmonic_phase = (phase * (h + 1)) % (SINE_WAVE_TABLE_LEN << 16);
        int16_t harmonic_wave = sine_wave_table[harmonic_phase >> 16];
        
        // 应用谐波强度和衰减
        float harmonic_amplitude = harmonic_amplitudes[h];
        if (h > 0) {
            // 高次谐波衰减更快
            harmonic_amplitude *= powf(envelope, h * 0.5f + 1.0f);
        }
        
        sample += harmonic_wave * harmonic_amplitude;
    }
    
    // 应用包络和音量
    sample *= envelope * volume / 256.0f;
    
    // 限制幅度避免溢出
    if (sample > 32767.0f) sample = 32767.0f;
    if (sample < -32767.0f) sample = -32767.0f;
    
    return (int16_t)sample;
}

// 获取当前时间（毫秒）
static uint32_t get_time_ms(void) {
    return to_ms_since_boot(get_absolute_time());
}

// 开始播放指定音符
static void start_playing_note(uint32_t note_index) {
    if (note_index < NUM_NOTES) {
        current_note = note_index;
        note_playing = true;
        is_playing_note = false;  // 设为false，让audio_callback重新初始化
        last_note_change = get_time_ms();
        note_sample_count = 0;  // 重置音符采样计数
        printf("播放音符 %d: %s (%.2f Hz)\n", 
               note_index + 1,
               note_names[current_note], 
               note_frequencies[current_note]);
    }
}

// 音频回调函数
static void audio_callback(void) {
    static uint32_t phase = 0;
    static uint32_t step = 0;
    
    // 自动播放模式的逻辑
    if (auto_play) {
        // 检查是否需要切换状态（音符 <-> 暂停）
        uint32_t current_time = get_time_ms();
        uint32_t elapsed_time = current_time - last_note_change;
        
        if (is_playing_note) {
            // 当前在播放音符，检查是否需要进入暂停
            if (elapsed_time >= note_duration_ms) {
                is_playing_note = false;
                last_note_change = current_time;
                printf("  -> 暂停 %dms\n", pause_duration_ms);
            }
        } else {
            // 当前在暂停，检查是否需要切换到下一个音符
            if (elapsed_time >= pause_duration_ms) {
                current_note = (current_note + 1) % NUM_NOTES;
                is_playing_note = true;
                last_note_change = current_time;
                note_sample_count = 0;  // 重置音符采样计数
                step = frequency_to_step(note_frequencies[current_note]);
                
                printf("播放音符: %s (%.2f Hz)", 
                       note_names[current_note], 
                       note_frequencies[current_note]);
            }
        }
    } else {
        // 手动播放模式
        if (note_playing) {
            if (!is_playing_note) {
                // 开始播放音符，重新计算频率步进值
                is_playing_note = true;
                step = frequency_to_step(note_frequencies[current_note]);
                phase = 0;  // 重置相位，确保从波形开始播放
            }
            
            // 检查音符播放时间（手动模式下播放更长时间）
            uint32_t current_time = get_time_ms();
            uint32_t elapsed_time = current_time - last_note_change;
            if (elapsed_time >= 1000) {  // 播放1秒后停止
                note_playing = false;
                is_playing_note = false;
            }
        }
    }
    
    // 获取音频缓冲区
    audio_buffer_t *buffer = take_audio_buffer(audio_pool, false);
    if (buffer == NULL) {
        return;
    }
    
    int16_t *samples = (int16_t *) buffer->buffer->bytes;
    
    // 生成音频采样
    for (uint i = 0; i < buffer->max_sample_count; i++) {
        int16_t sample = 0;  // 默认静音
        
        if (is_playing_note) {
            if (piano_mode) {
                // 钢琴音色模式
                float envelope = calculate_envelope(note_sample_count);
                sample = generate_piano_sample(phase, envelope);
            } else {
                // 纯正弦波模式
                sample = (volume * sine_wave_table[phase >> 16]) / 256;
            }
            
            // 更新相位
            phase += step;
            if (phase >= (SINE_WAVE_TABLE_LEN << 16)) {
                phase -= (SINE_WAVE_TABLE_LEN << 16);
            }
            
            note_sample_count++;
        }
        // 暂停期间保持静音（sample = 0）
        
        samples[i * 2 + 0] = sample;  // 左声道
        samples[i * 2 + 1] = sample;  // 右声道
    }
    
    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(audio_pool, buffer);
}

int main() {
    stdio_init_all();
    
    printf("=== DO RE MI 音阶演示 ===\n");
    printf("使用官方 pico-extras 音频库\n");
    printf("硬件连接：\n");
    printf("  GPIO 26 -> DIN   (数据输入)\n");
    printf("  GPIO 27 -> BCLK  (位时钟)\n");
    printf("  GPIO 28 -> LRCLK (左右声道时钟)\n");
    printf("  GPIO 22 -> XMT   (PCM5102静音控制)\n");
    printf("========================\n");
    
    // 初始化LED
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    
    // 初始化PCM5102静音控制引脚（XMT）
    gpio_init(PCM5102_XMT_PIN);
    gpio_set_dir(PCM5102_XMT_PIN, GPIO_OUT);
    gpio_put(PCM5102_XMT_PIN, 1);  // 默认拉高，解除静音
    printf("✓ PCM5102 XMT引脚初始化完成 (GPIO%d，默认解除静音)\n", PCM5102_XMT_PIN);
    
    // 生成正弦波查找表
    for (int i = 0; i < SINE_WAVE_TABLE_LEN; i++) {
        sine_wave_table[i] = (int16_t)(32767 * sinf(i * 2.0f * M_PI / SINE_WAVE_TABLE_LEN));
    }
    
    printf("正弦波表生成完成 (%d 采样点)\n", SINE_WAVE_TABLE_LEN);
    
    // 创建音频缓冲池
    audio_pool = audio_new_producer_pool(&producer_format, 3, SAMPLES_PER_BUFFER);
    if (!audio_pool) {
        printf("❌ 音频缓冲池创建失败\n");
        return -1;
    }
    
    // 初始化I2S音频
    const audio_format_t *output_format = audio_i2s_setup(&audio_format, &i2s_config);
    if (!output_format) {
        printf("❌ I2S音频初始化失败\n");
        return -1;
    }
    
    // 连接音频管道
    if (!audio_i2s_connect(audio_pool)) {
        printf("❌ 音频管道连接失败\n");
        return -1;
    }
    
    printf("✓ I2S音频初始化成功 (44.1kHz, 立体声, 16位)\n");
    
    // 启用I2S输出
    audio_i2s_set_enabled(true);
    
    // 初始化状态（默认不自动播放）
    last_note_change = get_time_ms();
    printf("✓ 音频系统就绪，等待用户输入\n");
    
    printf("\n控制键：\n");
    printf("  1-7 : 播放音符 (1=DO, 2=RE, 3=MI, 4=FA, 5=SOL, 6=LA, 7=SI)\n");
    printf("  +/- : 音量控制\n");
    printf("  a   : 切换自动播放模式\n");
    printf("  s   : 切换速度 (仅自动播放模式)\n");
    printf("  t   : 切换音色 (钢琴/纯音)\n");
    printf("  m   : 切换静音 (PCM5102 XMT控制)\n");
    printf("  q   : 退出\n\n");
    
    // 主循环
    while (true) {
        // 处理音频回调
        audio_callback();
        
        // 处理用户输入
        int c = getchar_timeout_us(0);
        if (c >= 0) {
            switch (c) {
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    if (!auto_play) {
                        uint32_t note_idx = c - '1';
                        printf("按键 '%c' -> 音符索引 %d\n", c, note_idx);
                        start_playing_note(note_idx);  // '1' = 0 (DO), '2' = 1 (RE), etc.
                    } else {
                        printf("当前处于自动播放模式，请先按 'a' 关闭自动播放\n");
                    }
                    break;
                    
                case '-':
                    if (volume > 10) {
                        volume -= 10;
                        printf("音量: %d\n", volume);
                    }
                    break;
                    
                case '+':
                case '=':
                    if (volume < 256) {
                        volume += 10;
                        printf("音量: %d\n", volume);
                    }
                    break;
                    
                case 'a':
                case 'A':
                    auto_play = !auto_play;
                    if (auto_play) {
                        current_note = 0;
                        is_playing_note = true;
                        last_note_change = get_time_ms();
                        note_sample_count = 0;
                        printf("自动播放模式: 开启\n");
                        printf("开始播放音符: %s (%.2f Hz)\n", note_names[0], note_frequencies[0]);
                    } else {
                        note_playing = false;
                        is_playing_note = false;
                        printf("自动播放模式: 关闭\n");
                    }
                    break;
                    
                case 't':
                    piano_mode = !piano_mode;
                    printf("音色模式: %s\n", piano_mode ? "钢琴音色" : "纯正弦波");
                    break;
                    
                case 's':
                case 'S':
                    if (auto_play) {
                        static int speed_mode = 1;
                        speed_mode = (speed_mode + 1) % 3;
                        switch(speed_mode) {
                            case 0: 
                                note_duration_ms = SPEED_FAST_NOTE_MS; 
                                pause_duration_ms = SPEED_FAST_PAUSE_MS;
                                printf("播放速度: 快 (%dms音符 + %dms暂停)\n", 
                                       note_duration_ms, pause_duration_ms); 
                                break;
                            case 1: 
                                note_duration_ms = SPEED_MEDIUM_NOTE_MS; 
                                pause_duration_ms = SPEED_MEDIUM_PAUSE_MS;
                                printf("播放速度: 中 (%dms音符 + %dms暂停)\n", 
                                       note_duration_ms, pause_duration_ms); 
                                break;
                            case 2: 
                                note_duration_ms = SPEED_SLOW_NOTE_MS; 
                                pause_duration_ms = SPEED_SLOW_PAUSE_MS;
                                printf("播放速度: 慢 (%dms音符 + %dms暂停)\n", 
                                       note_duration_ms, pause_duration_ms); 
                                break;
                        }
                    } else {
                        printf("速度切换仅在自动播放模式下有效，请先按 'a' 开启自动播放\n");
                    }
                    break;
                    
                case 'm':
                case 'M':
                    is_muted = !is_muted;
                    gpio_put(PCM5102_XMT_PIN, is_muted ? 0 : 1);
                    printf("PCM5102 静音: %s (XMT引脚: %s)\n", 
                           is_muted ? "开启" : "关闭",
                           is_muted ? "低电平" : "高电平");
                    break;
                    
                case 'q':
                    goto exit_loop;
            }
        }
        
        sleep_ms(10);
    }
    
exit_loop:
    printf("\n正在停止音频输出...\n");
    audio_i2s_set_enabled(false);
    gpio_put(LED_PIN, 0);
    
    printf("再见！\n");
    return 0;
} 