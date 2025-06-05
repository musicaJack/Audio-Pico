/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <math.h>

#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/pio.h"

#include "pico/stdlib.h"
#include "pico/audio.h"
#include "pico/audio_i2s.h"

#define SINE_WAVE_TABLE_LEN 2048
#define SAMPLES_PER_BUFFER 1156 // Samples / channel

static const uint32_t PIN_DCDC_PSM_CTRL = 23;

audio_buffer_pool_t *ap;
static bool decode_flg = false;
static constexpr int32_t DAC_ZERO = 0;

#define audio_pio __CONCAT(pio, PICO_AUDIO_I2S_PIO)

static audio_format_t audio_format = {
    .sample_freq = 44100,
    .pcm_format = AUDIO_PCM_FORMAT_S32,
    .channel_count = AUDIO_CHANNEL_STEREO
};

static audio_buffer_format_t producer_format = {
    .format = &audio_format,
    .sample_stride = 8
};

static audio_i2s_config_t i2s_config = {
    .data_pin = 26,  // gp26 -> DIN
    .clock_pin_base = 27,  // gp27 -> BCLK, gp28 -> LRCLK
    .dma_channel0 = 0,
    .dma_channel1 = 1,
    .pio_sm = 0
};

// 音阶频率定义 (DO RE MI FA SOL LA SI DO)
static const float note_frequencies[] = {
    261.63f,  // DO (C4)
    293.66f,  // RE (D4)  
    329.63f,  // MI (E4)
    349.23f,  // FA (F4)
    392.00f,  // SOL (G4)
    440.00f,  // LA (A4)
    493.88f,  // SI (B4)
    523.25f   // DO (C5)
};

static const char* note_names[] = {
    "DO", "RE", "MI", "FA", "SOL", "LA", "SI", "DO"
};

#define NUM_NOTES (sizeof(note_frequencies) / sizeof(note_frequencies[0]))

static int16_t sine_wave_table[SINE_WAVE_TABLE_LEN];
uint32_t step0 = 0x200000;
uint32_t step1 = 0x200000;
uint32_t pos0 = 0;
uint32_t pos1 = 0;
const uint32_t pos_max = 0x10000 * SINE_WAVE_TABLE_LEN;
uint vol = 80;

// 音阶播放控制变量
static uint32_t note_duration_ms = 1000;  // 每个音符持续1秒
static uint32_t current_note = 0;
static uint32_t last_note_change = 0;

#if 0
audio_buffer_pool_t *init_audio() {

    static audio_format_t audio_format = {
        .pcm_format = AUDIO_PCM_FORMAT_S32,
        .sample_freq = 44100,
        .channel_count = 2
    };

    static audio_buffer_format_t producer_format = {
        .format = &audio_format,
        .sample_stride = 8
    };

    audio_buffer_pool_t *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const audio_format_t *output_format;
#if USE_AUDIO_I2S
    audio_i2s_config_t config = {
        .data_pin = PICO_AUDIO_I2S_DATA_PIN,
        .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
        .dma_channel = 0,
        .pio_sm = 0
    };

    output_format = audio_i2s_setup(&audio_format, &audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    { // initial buffer data
        audio_buffer_t *buffer = take_audio_buffer(producer_pool, true);
        int32_t *samples = (int32_t *) buffer->buffer->bytes;
        for (uint i = 0; i < buffer->max_sample_count; i++) {
            samples[i*2+0] = 0;
            samples[i*2+1] = 0;
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(producer_pool, buffer);
    }
    audio_i2s_set_enabled(true);
#elif USE_AUDIO_PWM
    output_format = audio_pwm_setup(&audio_format, -1, &default_mono_channel_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    ok = audio_pwm_default_connect(producer_pool, false);
    assert(ok);
    audio_pwm_set_enabled(true);
#elif USE_AUDIO_SPDIF
    output_format = audio_spdif_setup(&audio_format, &audio_spdif_default_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    //ok = audio_spdif_connect(producer_pool);
    ok = audio_spdif_connect(producer_pool);
    assert(ok);
    audio_spdif_set_enabled(true);
#endif
    return producer_pool;
}
#endif

static inline uint32_t _millis(void)
{
	return to_ms_since_boot(get_absolute_time());
}

// 根据频率计算步进值
static uint32_t frequency_to_step(float frequency) {
    // step = (frequency * SINE_WAVE_TABLE_LEN * 0x10000) / sample_rate
    return (uint32_t)((frequency * SINE_WAVE_TABLE_LEN * 0x10000) / 44100.0f);
}

// 更新当前音符
static void update_current_note() {
    uint32_t current_time = _millis();
    
    if (current_time - last_note_change >= note_duration_ms) {
        current_note = (current_note + 1) % NUM_NOTES;
        last_note_change = current_time;
        
        // 计算新的步进值
        step0 = step1 = frequency_to_step(note_frequencies[current_note]);
        
        printf("播放音符: %s (%.2f Hz)   \r", 
               note_names[current_note], 
               note_frequencies[current_note]);
    }
}

void i2s_audio_deinit()
{
    decode_flg = false;

    audio_i2s_set_enabled(false);
    audio_i2s_end();

    audio_buffer_t* ab;
    ab = take_audio_buffer(ap, false);
    while (ab != nullptr) {
        free(ab->buffer->bytes);
        free(ab->buffer);
        ab = take_audio_buffer(ap, false);
    }
    ab = get_free_audio_buffer(ap, false);
    while (ab != nullptr) {
        free(ab->buffer->bytes);
        free(ab->buffer);
        ab = get_free_audio_buffer(ap, false);
    }
    ab = get_full_audio_buffer(ap, false);
    while (ab != nullptr) {
        free(ab->buffer->bytes);
        free(ab->buffer);
        ab = get_full_audio_buffer(ap, false);
    }
    free(ap);
    ap = nullptr;
}

audio_buffer_pool_t *i2s_audio_init(uint32_t sample_freq)
{
    audio_format.sample_freq = sample_freq;

    audio_buffer_pool_t *producer_pool = audio_new_producer_pool(&producer_format, 3, SAMPLES_PER_BUFFER);
    ap = producer_pool;

    bool __unused ok;
    const audio_format_t *output_format;

    output_format = audio_i2s_setup(&audio_format, &audio_format, &i2s_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    { // initial buffer data
        audio_buffer_t *ab = take_audio_buffer(producer_pool, true);
        int32_t *samples = (int32_t *) ab->buffer->bytes;
        for (uint i = 0; i < ab->max_sample_count; i++) {
            samples[i*2+0] = DAC_ZERO;
            samples[i*2+1] = DAC_ZERO;
        }
        ab->sample_count = ab->max_sample_count;
        give_audio_buffer(producer_pool, ab);
    }
    audio_i2s_set_enabled(true);

    decode_flg = true;
    return producer_pool;
}

int main() {

    stdio_init_all();
    
    // 添加LED指示
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1); // 点亮LED表示程序开始运行
    
    printf("=== CJMCU-5102 PCM5102 诊断测试 ===\n");
    printf("正在初始化系统...\n");
    printf("LED应该已经点亮，表示程序正在运行\n");
    printf("注意：如果只听到噪音，说明跳线设置不正确\n\n");

    // Set PLL_USB 96MHz
    pll_init(pll_usb, 1, 1536 * MHZ, 4, 4);
    clock_configure(clk_usb,
        0,
        CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
        96 * MHZ,
        48 * MHZ);
    // Change clk_sys to be 96MHz.
    clock_configure(clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
        96 * MHZ,
        96 * MHZ);
    // CLK peri is clocked from clk_sys so need to change clk_peri's freq
    clock_configure(clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
        96 * MHZ,
        96 * MHZ);
    // Reinit uart now that clk_peri has changed
    stdio_init_all();

    printf("时钟配置完成 (96MHz).\n");

    // DCDC PSM control
    // 0: PFM mode (best efficiency)
    // 1: PWM mode (improved ripple)
    gpio_init(PIN_DCDC_PSM_CTRL);
    gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
    gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise

    printf("DCDC PSM 控制设置为PWM模式以获得更好的音频质量.\n");

    for (int i = 0; i < SINE_WAVE_TABLE_LEN; i++) {
        sine_wave_table[i] = 32767 * cosf(i * 2 * (float) (M_PI / SINE_WAVE_TABLE_LEN));
    }

    printf("正弦波表已生成 (%d 个采样点).\n", SINE_WAVE_TABLE_LEN);

    ap = i2s_audio_init(44100);
    
    if (ap) {
        printf("✓ I2S 音频初始化成功，采样率 44.1kHz.\n");
        printf("✓ 音频缓冲池已创建\n");
        printf("\n=== DO RE MI 音阶演示 ===\n");
        printf("GPIO 26 -> DIN   (数据输入)\n");
        printf("GPIO 27 -> BCLK  (位时钟)\n");
        printf("GPIO 28 -> LRCLK (左右声道时钟)\n");
        printf("\n音阶序列: DO RE MI FA SOL LA SI DO\n");
        printf("每个音符持续: %d 毫秒\n", note_duration_ms);
        printf("当前音量: %d (可用+/-调节)\n", vol);
        printf("\n✓ 正在开始音阶播放...\n");
        printf("您应该听到循环播放的音阶\n");
        printf("\n控制键：\n");
        printf("  +/- : 音量控制 (0-256)\n");
        printf("  n   : 切换到下一个音符\n");
        printf("  s   : 速度切换 (快/中/慢)\n");
        printf("  q   : 退出\n");
        printf("\n=== 调试信息 ===\n");
        printf("如果没有声音，请检查：\n");
        printf("1. 接线是否正确\n");
        printf("2. 跳线设置是否正确\n");
        printf("3. 耳机/音箱是否连接到LOUT和ROUT\n");
        printf("4. 电源是否正常(VCC=5V, GND连接)\n");
    } else {
        printf("✗ 错误：I2S 音频初始化失败！\n");
        printf("请检查连接和跳线设置.\n");
        // 闪烁LED表示错误
        for(int i = 0; i < 10; i++) {
            gpio_put(LED_PIN, 0);
            sleep_ms(200);
            gpio_put(LED_PIN, 1);
            sleep_ms(200);
        }
        return -1;
    }

    // 初始化第一个音符
    last_note_change = _millis();
    step0 = step1 = frequency_to_step(note_frequencies[0]);
    printf("开始播放音符: %s (%.2f Hz)\n", note_names[0], note_frequencies[0]);
    
    while (true) {
        int c = getchar_timeout_us(0);
        if (c >= 0) {
            if (c == '-' && vol) {
                vol--;
                printf("音量: %d\n", vol);
            }
            if ((c == '=' || c == '+') && vol < 256) {
                vol++;
                printf("音量: %d\n", vol);
            }
            if (c == 'n') {
                // 手动切换到下一个音符
                current_note = (current_note + 1) % NUM_NOTES;
                last_note_change = _millis();
                step0 = step1 = frequency_to_step(note_frequencies[current_note]);
                printf("切换到音符: %s (%.2f Hz)\n", note_names[current_note], note_frequencies[current_note]);
            }
            if (c == 's') {
                // 切换播放速度
                static int speed_mode = 1; // 0=快, 1=中, 2=慢
                speed_mode = (speed_mode + 1) % 3;
                switch(speed_mode) {
                    case 0: note_duration_ms = 500; printf("播放速度: 快 (500ms/音符)\n"); break;
                    case 1: note_duration_ms = 1000; printf("播放速度: 中 (1000ms/音符)\n"); break;
                    case 2: note_duration_ms = 2000; printf("播放速度: 慢 (2000ms/音符)\n"); break;
                }
            }
            if (c == 'q') break;
        }
        sleep_ms(10); // 减少CPU占用
    }
    puts("\n");
    return 0;
}

void decode()
{
    // 更新当前音符
    update_current_note();
    
    audio_buffer_t *buffer = take_audio_buffer(ap, false);
    if (buffer == NULL) { return; }
    int32_t *samples = (int32_t *) buffer->buffer->bytes;
    for (uint i = 0; i < buffer->max_sample_count; i++) {
        int32_t value0 = (vol * sine_wave_table[pos0 >> 16u]) << 8u;
        int32_t value1 = (vol * sine_wave_table[pos1 >> 16u]) << 8u;
        // use 32bit full scale
        samples[i*2+0] = value0 + (value0 >> 16u);  // L
        samples[i*2+1] = value1 + (value1 >> 16u);  // R
        pos0 += step0;
        pos1 += step1;
        if (pos0 >= pos_max) pos0 -= pos_max;
        if (pos1 >= pos_max) pos1 -= pos_max;
    }
    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(ap, buffer);
    return;
}

extern "C" {
// callback from:
//   void __isr __time_critical_func(audio_i2s_dma_irq_handler)()
//   defined at my_pico_audio_i2s/audio_i2s.c
//   where i2s_callback_func() is declared with __attribute__((weak))
void i2s_callback_func()
{
    if (decode_flg) {
        decode();
    }
}
}