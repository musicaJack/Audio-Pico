#pragma once

/**
 * @file pin_config.hpp
 * @brief 统一的引脚配置管理 - Audio-Pico项目
 * @description 这个文件包含了所有硬件引脚的定义，包括I2S音频输出、SPI SD卡和状态指示的引脚配置
 */

// 前向声明，避免头文件循环依赖
typedef struct spi_inst spi_inst_t;

// 必要的硬件声明 (仅声明外部变量)
#ifdef __cplusplus
extern "C" {
#endif

// SPI实例外部声明
extern spi_inst_t spi0_inst;
extern spi_inst_t spi1_inst;
#define spi0 (&spi0_inst)
#define spi1 (&spi1_inst)

// GPIO默认LED引脚声明
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 25
#endif

#ifdef __cplusplus
}
#endif

// =============================================================================
// I2S 音频输出引脚配置 (PCM5102 DAC)
// =============================================================================

// I2S 音频格式配置
#define AUDIO_SAMPLE_RATE       44100       // 音频采样率 (Hz)
#define AUDIO_CHANNELS          2           // 声道数量 (立体声)
#define AUDIO_BIT_DEPTH         16          // 位深度
#define AUDIO_BUFFER_SIZE       1156        // 音频缓冲区大小

// I2S 信号引脚 (基于用户接线图)
#define AUDIO_PIN_MUTE          22          // XMT - 静音控制引脚 (低:静音/高:解除)
#define AUDIO_PIN_DATA          26          // DIN - 数据输入引脚
#define AUDIO_PIN_BCLK          27          // BCLK - 位时钟引脚
#define AUDIO_PIN_LRCLK         28          // LRCLK - 左右声道时钟引脚

// I2S 内部配置
#define AUDIO_DMA_CHANNEL       0           // DMA通道
#define AUDIO_PIO_SM            0           // PIO状态机
#define AUDIO_CLOCK_PIN_BASE    AUDIO_PIN_BCLK  // 时钟引脚基址

// =============================================================================
// SPI SD卡引脚配置 (可选 - 用于WAV播放)
// =============================================================================

// SPI 接口配置 (当前已禁用WAV功能)
#define SD_SPI_INST             spi1        // SPI接口实例 (推荐SPI1避免冲突)
#define SD_SPI_SPEED_SLOW       400000      // SPI初始化速度 (400kHz)
#define SD_SPI_SPEED_FAST       40000000    // SPI正常操作速度 (40MHz)

// SPI 信号引脚
#define SD_PIN_SCK              10          // SPI时钟引脚
#define SD_PIN_MOSI             11          // SPI数据输出引脚
#define SD_PIN_MISO             12          // SPI数据输入引脚
#define SD_PIN_CS               13          // SPI片选引脚

// SD卡操作配置
#define SD_USE_INTERNAL_PULLUP  true        // 使用内部上拉电阻

// =============================================================================
// 状态指示和控制引脚
// =============================================================================

// LED 状态指示
#define STATUS_PIN_ONBOARD_LED  PICO_DEFAULT_LED_PIN    // 板载LED (通常是GPIO25)
#define STATUS_PIN_AUDIO_LED    25          // 音频状态指示LED
#define STATUS_PIN_ERROR_LED    24          // 错误状态指示LED (可选)

// 控制按键引脚 (可选 - 用于硬件控制)
#define BUTTON_PIN_PLAY_PAUSE   16          // 播放/暂停按键
#define BUTTON_PIN_VOLUME_UP    17          // 音量增加按键
#define BUTTON_PIN_VOLUME_DOWN  18          // 音量减小按键
#define BUTTON_PIN_MUTE_TOGGLE  19          // 静音切换按键
#define BUTTON_PIN_WAVE_CHANGE  20          // 波形切换按键

// 按键配置
#define BUTTON_DEBOUNCE_MS      50          // 按键防抖时间 (毫秒)
#define BUTTON_LONG_PRESS_MS    1000        // 长按检测时间 (毫秒)

// =============================================================================
// 音频处理参数配置
// =============================================================================

// 音频生成参数
#define AUDIO_DEFAULT_VOLUME    80          // 默认音量 (0-100)
#define AUDIO_MAX_POLYPHONY     4           // 最大同时播放音符数
#define AUDIO_NOTE_DURATION_MS  300         // 默认音符持续时间 (毫秒)
#define AUDIO_NOTE_GAP_MS       50          // 音符间隔时间 (毫秒)

// 内存管理配置
#define AUDIO_LRU_CACHE_SIZE    12          // LRU缓存最大资源数
#define AUDIO_MEMORY_POOL_SIZE  8192        // 音频内存池大小 (字节)

// =============================================================================
// 兼容性宏定义（保持向后兼容）
// =============================================================================

// I2S 兼容性定义 (与旧版本spi_config.hpp兼容)
#define I2S_MUTE_PIN            AUDIO_PIN_MUTE
#define I2S_DATA_PIN            AUDIO_PIN_DATA
#define I2S_BCLK_PIN            AUDIO_PIN_BCLK
#define I2S_LRCLK_PIN           AUDIO_PIN_LRCLK
#define I2S_CLOCK_PIN_BASE      AUDIO_CLOCK_PIN_BASE
#define I2S_DMA_CHANNEL         AUDIO_DMA_CHANNEL
#define I2S_PIO_SM              AUDIO_PIO_SM

// SPI SD卡兼容性定义
#define SPI_SD_PORT             SD_SPI_INST
#define SPI_SD_SCK_PIN          SD_PIN_SCK
#define SPI_SD_MOSI_PIN         SD_PIN_MOSI
#define SPI_SD_MISO_PIN         SD_PIN_MISO
#define SPI_SD_CS_PIN           SD_PIN_CS
#define SPI_SD_CLK_SLOW         SD_SPI_SPEED_SLOW
#define SPI_SD_CLK_FAST         SD_SPI_SPEED_FAST

// =============================================================================
// 硬件配置验证宏
// =============================================================================

// 编译时验证引脚配置的合理性
#if AUDIO_PIN_DATA == SD_PIN_SCK || AUDIO_PIN_DATA == SD_PIN_MOSI
    #warning "I2S DATA pin conflicts with SPI pins"
#endif

#if AUDIO_PIN_BCLK == SD_PIN_SCK || AUDIO_PIN_BCLK == SD_PIN_MOSI
    #warning "I2S BCLK pin conflicts with SPI pins"
#endif

#if AUDIO_PIN_LRCLK == SD_PIN_SCK || AUDIO_PIN_LRCLK == SD_PIN_MOSI
    #warning "I2S LRCLK pin conflicts with SPI pins"
#endif

// 验证音频引脚不冲突
#if AUDIO_PIN_DATA == AUDIO_PIN_BCLK || AUDIO_PIN_DATA == AUDIO_PIN_LRCLK || AUDIO_PIN_BCLK == AUDIO_PIN_LRCLK
    #error "I2S audio pins cannot be the same"
#endif

// =============================================================================
// 辅助宏定义
// =============================================================================

// 获取完整的I2S音频配置
#define AUDIO_GET_I2S_CONFIG() \
    AUDIO_PIN_DATA, AUDIO_CLOCK_PIN_BASE, AUDIO_DMA_CHANNEL, AUDIO_PIO_SM, AUDIO_PIN_MUTE, true

// 获取完整的SPI SD卡配置
#define SD_GET_SPI_CONFIG() \
    SD_SPI_INST, SD_PIN_SCK, SD_PIN_MOSI, SD_PIN_MISO, SD_PIN_CS, SD_SPI_SPEED_SLOW, SD_SPI_SPEED_FAST

// 获取完整的音频格式配置
#define AUDIO_GET_FORMAT_CONFIG() \
    AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, AUDIO_BIT_DEPTH, AUDIO_BUFFER_SIZE

// =============================================================================
// 配置打印函数声明
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// 打印硬件配置信息的函数声明
void print_audio_pin_config(void);
void print_sd_pin_config(void);
void print_status_pin_config(void);
void print_all_pin_config(void);

#ifdef __cplusplus
}
#endif

// =============================================================================
// C++ 命名空间配置 (仅在C++环境下有效)
// =============================================================================

#ifdef __cplusplus
namespace HardwareConfig {

/**
 * @brief I2S音频引脚配置结构体
 */
struct I2SAudioPins {
    static constexpr uint8_t MUTE_PIN = AUDIO_PIN_MUTE;
    static constexpr uint8_t DATA_PIN = AUDIO_PIN_DATA;
    static constexpr uint8_t BCLK_PIN = AUDIO_PIN_BCLK;
    static constexpr uint8_t LRCLK_PIN = AUDIO_PIN_LRCLK;
    static constexpr uint8_t CLOCK_PIN_BASE = AUDIO_CLOCK_PIN_BASE;
    static constexpr uint8_t DMA_CHANNEL = AUDIO_DMA_CHANNEL;
    static constexpr uint8_t PIO_SM = AUDIO_PIO_SM;
};

/**
 * @brief SPI SD卡引脚配置结构体
 */
struct SPISDCardPins {
    static spi_inst_t* get_spi_port() { return SD_SPI_INST; }
    static constexpr uint8_t SCK_PIN = SD_PIN_SCK;
    static constexpr uint8_t MOSI_PIN = SD_PIN_MOSI;
    static constexpr uint8_t MISO_PIN = SD_PIN_MISO;
    static constexpr uint8_t CS_PIN = SD_PIN_CS;
    static constexpr uint32_t CLK_SLOW = SD_SPI_SPEED_SLOW;
    static constexpr uint32_t CLK_FAST = SD_SPI_SPEED_FAST;
    static constexpr bool USE_INTERNAL_PULLUP = SD_USE_INTERNAL_PULLUP;
};

/**
 * @brief 状态引脚配置结构体
 */
struct StatusPins {
    static constexpr uint8_t ONBOARD_LED = STATUS_PIN_ONBOARD_LED;
    static constexpr uint8_t AUDIO_LED = STATUS_PIN_AUDIO_LED;
    static constexpr uint8_t ERROR_LED = STATUS_PIN_ERROR_LED;
};

/**
 * @brief 按键引脚配置结构体
 */
struct ButtonPins {
    static constexpr uint8_t PLAY_PAUSE = BUTTON_PIN_PLAY_PAUSE;
    static constexpr uint8_t VOLUME_UP = BUTTON_PIN_VOLUME_UP;
    static constexpr uint8_t VOLUME_DOWN = BUTTON_PIN_VOLUME_DOWN;
    static constexpr uint8_t MUTE_TOGGLE = BUTTON_PIN_MUTE_TOGGLE;
    static constexpr uint8_t WAVE_CHANGE = BUTTON_PIN_WAVE_CHANGE;
    static constexpr uint32_t DEBOUNCE_MS = BUTTON_DEBOUNCE_MS;
    static constexpr uint32_t LONG_PRESS_MS = BUTTON_LONG_PRESS_MS;
};

/**
 * @brief 获取I2S配置结构体
 */
inline auto getI2SConfig() {
    struct I2SConfig {
        uint8_t data_pin = I2SAudioPins::DATA_PIN;
        uint8_t clock_pin_base = I2SAudioPins::CLOCK_PIN_BASE;
        uint8_t dma_channel = I2SAudioPins::DMA_CHANNEL;
        uint8_t pio_sm = I2SAudioPins::PIO_SM;
        uint8_t mute_pin = I2SAudioPins::MUTE_PIN;
        bool enable_mute_control = true;
    };
    return I2SConfig{};
}

/**
 * @brief 获取SPI配置结构体
 */
inline auto getSPIConfig() {
    struct {
        spi_inst_t* spi_port = SPISDCardPins::get_spi_port();
        uint32_t clk_slow = SPISDCardPins::CLK_SLOW;
        uint32_t clk_fast = SPISDCardPins::CLK_FAST;
        uint pin_miso = SPISDCardPins::MISO_PIN;
        uint pin_cs = SPISDCardPins::CS_PIN;
        uint pin_sck = SPISDCardPins::SCK_PIN;
        uint pin_mosi = SPISDCardPins::MOSI_PIN;
        bool use_internal_pullup = SPISDCardPins::USE_INTERNAL_PULLUP;
    } config;
    return config;
}

} // namespace HardwareConfig
#endif // __cplusplus 