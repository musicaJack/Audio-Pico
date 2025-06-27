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
// 注意：避免与hardware/spi.h中的定义冲突
// Pico SDK已经提供了spi0和spi1的定义，这里不再重复定义
// extern spi_inst_t spi0_inst;
// extern spi_inst_t spi1_inst;
// #define spi0 (&spi0_inst)
// #define spi1 (&spi1_inst)

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
#define AUDIO_DMA_CHANNEL       1           // DMA通道（避免与显示系统冲突）
#define AUDIO_PIO_SM            1           // PIO状态机（避免与显示系统冲突）
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
// Joystick 手柄 I2C 配置
// =============================================================================

// I2C 接口配置
#define JOYSTICK_I2C_INST       i2c1        // I2C接口实例
#define JOYSTICK_I2C_ADDR       0x63        // I2C设备地址
#define JOYSTICK_I2C_SPEED      100000      // I2C速度（100kHz）

// I2C 信号引脚
#define JOYSTICK_PIN_SDA        6           // I2C数据引脚
#define JOYSTICK_PIN_SCL        7           // I2C时钟引脚

// Joystick 操作参数配置
#define JOYSTICK_THRESHOLD      1800        // 操作检测阈值
#define JOYSTICK_LOOP_DELAY_MS  20          // 循环延迟时间（毫秒）

// Joystick LED 颜色定义
#define JOYSTICK_LED_OFF        0x000000    // 黑色（关闭）
#define JOYSTICK_LED_RED        0xFF0000    // 红色
#define JOYSTICK_LED_GREEN      0x00FF00    // 绿色
#define JOYSTICK_LED_BLUE       0x0000FF    // 蓝色

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

// ILI9488 兼容性定义（仅在使用时启用）
#define PIN_DC                  ILI9488_PIN_DC
#define PIN_RST                 ILI9488_PIN_RST
#define PIN_CS                  ILI9488_PIN_CS
#define PIN_SCK                 ILI9488_PIN_SCK
#define PIN_MOSI                ILI9488_PIN_MOSI
#define PIN_BL                  ILI9488_PIN_BL

// Joystick 兼容性定义
#define JOYSTICK_I2C_PORT       JOYSTICK_I2C_INST
#define JOYSTICK_I2C_SDA_PIN    JOYSTICK_PIN_SDA
#define JOYSTICK_I2C_SCL_PIN    JOYSTICK_PIN_SCL

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

// 验证ILI9488引脚不冲突于音频引脚
#if ILI9488_PIN_SCK == AUDIO_PIN_DATA || ILI9488_PIN_SCK == AUDIO_PIN_BCLK || ILI9488_PIN_SCK == AUDIO_PIN_LRCLK
    #warning "ILI9488 SPI SCK pin conflicts with I2S audio pins"
#endif

#if ILI9488_PIN_MOSI == AUDIO_PIN_DATA || ILI9488_PIN_MOSI == AUDIO_PIN_BCLK || ILI9488_PIN_MOSI == AUDIO_PIN_LRCLK
    #warning "ILI9488 SPI MOSI pin conflicts with I2S audio pins"
#endif

// 验证Joystick I2C引脚不冲突
#if JOYSTICK_PIN_SDA == AUDIO_PIN_DATA || JOYSTICK_PIN_SDA == AUDIO_PIN_BCLK || JOYSTICK_PIN_SDA == AUDIO_PIN_LRCLK
    #warning "Joystick I2C SDA pin conflicts with I2S audio pins"
#endif

#if JOYSTICK_PIN_SCL == AUDIO_PIN_DATA || JOYSTICK_PIN_SCL == AUDIO_PIN_BCLK || JOYSTICK_PIN_SCL == AUDIO_PIN_LRCLK
    #warning "Joystick I2C SCL pin conflicts with I2S audio pins"
#endif



// =============================================================================
// ILI9488 TFT LCD 显示屏引脚配置（预留，具体引脚根据实际硬件配置）
// =============================================================================

// 注意：以下引脚配置仅为示例，请根据实际硬件连接进行配置
// SPI 接口配置
#define ILI9488_SPI_INST        spi0        // SPI接口实例
#define ILI9488_SPI_SPEED_HZ    40000000    // SPI速度（40MHz）

// SPI 信号引脚（请根据实际连接修改）
#define ILI9488_PIN_SCK         18          // SPI时钟引脚
#define ILI9488_PIN_MOSI        19          // SPI数据输出引脚
#define ILI9488_PIN_MISO        255         // SPI数据输入引脚（未使用，设为255）
// 注意：ILI9488是只写显示屏，不需要MISO引脚

// 控制信号引脚（请根据实际连接修改）
#define ILI9488_PIN_CS          17          // 片选引脚
#define ILI9488_PIN_DC          20          // 数据/命令选择引脚
#define ILI9488_PIN_RST         15          // 复位引脚
#define ILI9488_PIN_BL          16          // 背光控制引脚（PWM控制）

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

// 获取完整的ILI9488 SPI配置
#define ILI9488_GET_SPI_CONFIG() \
    ILI9488_SPI_INST, ILI9488_PIN_DC, ILI9488_PIN_RST, ILI9488_PIN_CS, ILI9488_PIN_SCK, ILI9488_PIN_MOSI, ILI9488_PIN_BL, ILI9488_SPI_SPEED_HZ

// 获取完整的Joystick I2C配置
#define JOYSTICK_GET_I2C_CONFIG() \
    JOYSTICK_I2C_INST, JOYSTICK_I2C_ADDR, JOYSTICK_PIN_SDA, JOYSTICK_PIN_SCL, JOYSTICK_I2C_SPEED

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
 * @brief Joystick手柄引脚配置结构体
 */
struct JoystickPins {
    static constexpr uint8_t I2C_ADDR = JOYSTICK_I2C_ADDR;
    static constexpr uint32_t I2C_SPEED = JOYSTICK_I2C_SPEED;
    static constexpr uint8_t SDA_PIN = JOYSTICK_PIN_SDA;
    static constexpr uint8_t SCL_PIN = JOYSTICK_PIN_SCL;
    static constexpr uint16_t THRESHOLD = JOYSTICK_THRESHOLD;
    static constexpr uint32_t LOOP_DELAY_MS = JOYSTICK_LOOP_DELAY_MS;
    static constexpr uint32_t LED_OFF = JOYSTICK_LED_OFF;
    static constexpr uint32_t LED_RED = JOYSTICK_LED_RED;
    static constexpr uint32_t LED_GREEN = JOYSTICK_LED_GREEN;
    static constexpr uint32_t LED_BLUE = JOYSTICK_LED_BLUE;
};

/**
 * @brief ILI9488显示屏引脚配置结构体（预留，根据实际硬件配置）
 */
struct ILI9488Pins {
    static spi_inst_t* get_spi_port() { return ILI9488_SPI_INST; }
    static constexpr uint32_t SPI_SPEED_HZ = ILI9488_SPI_SPEED_HZ;
    static constexpr uint8_t SCK_PIN = ILI9488_PIN_SCK;
    static constexpr uint8_t MOSI_PIN = ILI9488_PIN_MOSI;
    static constexpr uint8_t CS_PIN = ILI9488_PIN_CS;
    static constexpr uint8_t DC_PIN = ILI9488_PIN_DC;
    static constexpr uint8_t RST_PIN = ILI9488_PIN_RST;
    static constexpr uint8_t BL_PIN = ILI9488_PIN_BL;
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