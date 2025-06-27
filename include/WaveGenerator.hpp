#pragma once

#include <cstdint>
#include <cmath>
#include <vector>
#include <memory>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Audio {

/**
 * @brief 波形类型枚举
 */
enum class WaveType {
    SINE,           // 正弦波
    SQUARE,         // 方波
    TRIANGLE,       // 三角波
    SAWTOOTH,       // 锯齿波
    PIANO           // 钢琴音色（多谐波合成）
};

/**
 * @brief ADSR包络参数
 */
struct ADSREnvelope {
    uint32_t attack_samples = 0;   // 攻击时间（采样数）
    uint32_t decay_samples = 0;    // 衰减时间（采样数）
    float sustain_level = 1.0f;    // 持续电平（0.0-1.0）
    uint32_t release_samples = 0;  // 释放时间（采样数）
};

/**
 * @brief 波形生成器基类模板
 * @tparam SampleType 采样数据类型
 */
template<typename SampleType = int16_t>
class WaveGenerator {
public:
    virtual ~WaveGenerator() = default;

    /**
     * @brief 设置采样率
     * @param sample_rate 采样率
     */
    virtual void setSampleRate(uint32_t sample_rate);

    /**
     * @brief 设置频率
     * @param frequency 频率（Hz）
     */
    virtual void setFrequency(float frequency);

    /**
     * @brief 设置振幅
     * @param amplitude 振幅（0.0-1.0）
     */
    virtual void setAmplitude(float amplitude);

    /**
     * @brief 重置相位
     */
    virtual void resetPhase();

    /**
     * @brief 生成下一个采样
     * @return 生成的采样值
     */
    virtual SampleType generateSample() = 0;

    /**
     * @brief 批量生成采样
     * @param samples 输出缓冲区
     * @param count 采样数量
     */
    virtual void generateSamples(SampleType* samples, size_t count);

    /**
     * @brief 设置ADSR包络
     * @param envelope ADSR包络参数
     */
    virtual void setEnvelope(const ADSREnvelope& envelope);

    /**
     * @brief 触发音符开始
     */
    virtual void noteOn();

    /**
     * @brief 触发音符结束
     */
    virtual void noteOff();

protected:
    uint32_t sample_rate_ = 44100;
    float frequency_ = 440.0f;
    float amplitude_ = 0.5f;
    uint32_t phase_ = 0;
    uint32_t phase_step_ = 0;
    
    // ADSR 包络相关
    ADSREnvelope envelope_;
    uint32_t envelope_position_ = 0;
    uint32_t release_start_position_ = 0;
    bool note_on_ = false;

    /**
     * @brief 更新相位步进值
     */
    virtual void updatePhaseStep();

    /**
     * @brief 计算当前包络值
     * @return 包络值（0.0-1.0）
     */
    virtual float calculateEnvelope();

    /**
     * @brief 更新包络位置
     */
    virtual void updateEnvelope();
};

/**
 * @brief 正弦波生成器
 */
template<typename SampleType = int16_t>
class SineWaveGenerator : public WaveGenerator<SampleType> {
public:
    SineWaveGenerator();
    SampleType generateSample() override;

private:
    static constexpr size_t TABLE_SIZE = 2048;
    static std::array<float, TABLE_SIZE> sine_table_;
    static bool table_initialized_;

    static void initializeSineTable();
};

/**
 * @brief 方波生成器
 */
template<typename SampleType = int16_t>
class SquareWaveGenerator : public WaveGenerator<SampleType> {
public:
    SampleType generateSample() override;
};

/**
 * @brief 三角波生成器
 */
template<typename SampleType = int16_t>
class TriangleWaveGenerator : public WaveGenerator<SampleType> {
public:
    SampleType generateSample() override;
};

/**
 * @brief 锯齿波生成器
 */
template<typename SampleType = int16_t>
class SawtoothWaveGenerator : public WaveGenerator<SampleType> {
public:
    SampleType generateSample() override;
};

/**
 * @brief 钢琴音色生成器（多谐波合成）
 */
template<typename SampleType = int16_t>
class PianoWaveGenerator : public WaveGenerator<SampleType> {
public:
    PianoWaveGenerator();
    SampleType generateSample() override;

private:
    static constexpr size_t TABLE_SIZE = 2048;
    static std::array<float, TABLE_SIZE> sine_table_;
    static bool table_initialized_;

    static void initializeSineTable();
};

/**
 * @brief 波形生成器工厂类
 */
template<typename SampleType = int16_t>
class WaveFactory {
public:
    /**
     * @brief 创建指定类型的波形生成器
     * @param type 波形类型
     * @return 波形生成器实例
     */
    static std::unique_ptr<WaveGenerator<SampleType>> create(WaveType type);
};

} // namespace Audio

// 由于模板类需要在头文件中包含实现，我们在这里包含实现文件
#include "WaveGenerator.inl" 