#pragma once

// WaveGenerator模板类实现文件
namespace Audio {

// ==================== WaveGenerator 基类实现 ====================

template<typename SampleType>
void WaveGenerator<SampleType>::setSampleRate(uint32_t sample_rate) {
    sample_rate_ = sample_rate;
    updatePhaseStep();
}

template<typename SampleType>
void WaveGenerator<SampleType>::setFrequency(float frequency) {
    frequency_ = frequency;
    updatePhaseStep();
}

template<typename SampleType>
void WaveGenerator<SampleType>::setAmplitude(float amplitude) {
    amplitude_ = amplitude;
}

template<typename SampleType>
void WaveGenerator<SampleType>::resetPhase() {
    phase_ = 0;
    envelope_position_ = 0;
}

template<typename SampleType>
void WaveGenerator<SampleType>::generateSamples(SampleType* samples, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        samples[i] = generateSample();
    }
}

template<typename SampleType>
void WaveGenerator<SampleType>::setEnvelope(const ADSREnvelope& envelope) {
    envelope_ = envelope;
}

template<typename SampleType>
void WaveGenerator<SampleType>::noteOn() {
    envelope_position_ = 0;
    note_on_ = true;
}

template<typename SampleType>
void WaveGenerator<SampleType>::noteOff() {
    note_on_ = false;
    release_start_position_ = envelope_position_;
}

template<typename SampleType>
void WaveGenerator<SampleType>::updatePhaseStep() {
    phase_step_ = static_cast<uint32_t>((frequency_ * 4294967296.0) / sample_rate_);
}

template<typename SampleType>
float WaveGenerator<SampleType>::calculateEnvelope() {
    if (!note_on_ && envelope_position_ == 0) {
        return 0.0f; // 音符未开始
    }

    uint32_t pos = envelope_position_;
    
    if (note_on_) {
        // 音符按下状态
        if (pos < envelope_.attack_samples) {
            // Attack阶段
            return static_cast<float>(pos) / envelope_.attack_samples;
        } else if (pos < envelope_.attack_samples + envelope_.decay_samples) {
            // Decay阶段
            uint32_t decay_pos = pos - envelope_.attack_samples;
            float decay_ratio = static_cast<float>(decay_pos) / envelope_.decay_samples;
            return 1.0f - decay_ratio * (1.0f - envelope_.sustain_level);
        } else {
            // Sustain阶段
            return envelope_.sustain_level;
        }
    } else {
        // 音符释放状态
        uint32_t release_pos = pos - release_start_position_;
        if (release_pos >= envelope_.release_samples) {
            return 0.0f; // Release完成
        }
        float release_ratio = static_cast<float>(release_pos) / envelope_.release_samples;
        return envelope_.sustain_level * (1.0f - release_ratio);
    }
}

template<typename SampleType>
void WaveGenerator<SampleType>::updateEnvelope() {
    if (note_on_ || envelope_position_ > release_start_position_) {
        envelope_position_++;
    }
}

// ==================== SineWaveGenerator 实现 ====================

// 静态成员定义
template<typename SampleType>
std::array<float, SineWaveGenerator<SampleType>::TABLE_SIZE> SineWaveGenerator<SampleType>::sine_table_;

template<typename SampleType>
bool SineWaveGenerator<SampleType>::table_initialized_ = false;

template<typename SampleType>
SineWaveGenerator<SampleType>::SineWaveGenerator() {
    initializeSineTable();
}

template<typename SampleType>
void SineWaveGenerator<SampleType>::initializeSineTable() {
    if (!table_initialized_) {
        for (size_t i = 0; i < TABLE_SIZE; ++i) {
            sine_table_[i] = std::sin(2.0 * M_PI * i / TABLE_SIZE);
        }
        table_initialized_ = true;
    }
}

template<typename SampleType>
SampleType SineWaveGenerator<SampleType>::generateSample() {
    // 获取正弦波值
    uint32_t table_index = (this->phase_ >> 20) % TABLE_SIZE; // 取高位作为表索引
    float wave_value = sine_table_[table_index];
    
    // 应用包络和振幅
    float envelope = this->calculateEnvelope();
    float sample = wave_value * envelope * this->amplitude_;
    
    // 更新相位和包络
    this->phase_ += this->phase_step_;
    this->updateEnvelope();
    
    // 转换为目标类型
    if constexpr (std::is_same_v<SampleType, int16_t>) {
        return static_cast<int16_t>(sample * 32767.0f);
    } else if constexpr (std::is_same_v<SampleType, float>) {
        return sample;
    } else {
        return static_cast<SampleType>(sample);
    }
}

// ==================== SquareWaveGenerator 实现 ====================

template<typename SampleType>
SampleType SquareWaveGenerator<SampleType>::generateSample() {
    // 方波：前半周期为正，后半周期为负
    float wave_value = (this->phase_ < 0x80000000) ? 1.0f : -1.0f;
    
    // 应用包络和振幅
    float envelope = this->calculateEnvelope();
    float sample = wave_value * envelope * this->amplitude_;
    
    // 更新相位和包络
    this->phase_ += this->phase_step_;
    this->updateEnvelope();
    
    // 转换为目标类型
    if constexpr (std::is_same_v<SampleType, int16_t>) {
        return static_cast<int16_t>(sample * 32767.0f);
    } else if constexpr (std::is_same_v<SampleType, float>) {
        return sample;
    } else {
        return static_cast<SampleType>(sample);
    }
}

// ==================== TriangleWaveGenerator 实现 ====================

template<typename SampleType>
SampleType TriangleWaveGenerator<SampleType>::generateSample() {
    // 三角波：线性上升和下降
    float normalized_phase = static_cast<float>(this->phase_) / 4294967296.0f;
    float wave_value;
    
    if (normalized_phase < 0.5f) {
        // 上升段：0到1
        wave_value = 4.0f * normalized_phase - 1.0f;
    } else {
        // 下降段：1到-1
        wave_value = 3.0f - 4.0f * normalized_phase;
    }
    
    // 应用包络和振幅
    float envelope = this->calculateEnvelope();
    float sample = wave_value * envelope * this->amplitude_;
    
    // 更新相位和包络
    this->phase_ += this->phase_step_;
    this->updateEnvelope();
    
    // 转换为目标类型
    if constexpr (std::is_same_v<SampleType, int16_t>) {
        return static_cast<int16_t>(sample * 32767.0f);
    } else if constexpr (std::is_same_v<SampleType, float>) {
        return sample;
    } else {
        return static_cast<SampleType>(sample);
    }
}

// ==================== SawtoothWaveGenerator 实现 ====================

template<typename SampleType>
SampleType SawtoothWaveGenerator<SampleType>::generateSample() {
    // 锯齿波：线性上升
    float normalized_phase = static_cast<float>(this->phase_) / 4294967296.0f;
    float wave_value = 2.0f * normalized_phase - 1.0f;
    
    // 应用包络和振幅
    float envelope = this->calculateEnvelope();
    float sample = wave_value * envelope * this->amplitude_;
    
    // 更新相位和包络
    this->phase_ += this->phase_step_;
    this->updateEnvelope();
    
    // 转换为目标类型
    if constexpr (std::is_same_v<SampleType, int16_t>) {
        return static_cast<int16_t>(sample * 32767.0f);
    } else if constexpr (std::is_same_v<SampleType, float>) {
        return sample;
    } else {
        return static_cast<SampleType>(sample);
    }
}

// ==================== PianoWaveGenerator 实现 ====================

// 静态成员定义
template<typename SampleType>
std::array<float, PianoWaveGenerator<SampleType>::TABLE_SIZE> PianoWaveGenerator<SampleType>::sine_table_;

template<typename SampleType>
bool PianoWaveGenerator<SampleType>::table_initialized_ = false;

template<typename SampleType>
PianoWaveGenerator<SampleType>::PianoWaveGenerator() {
    initializeSineTable();
}

template<typename SampleType>
void PianoWaveGenerator<SampleType>::initializeSineTable() {
    if (!table_initialized_) {
        for (size_t i = 0; i < TABLE_SIZE; ++i) {
            sine_table_[i] = std::sin(2.0 * M_PI * i / TABLE_SIZE);
        }
        table_initialized_ = true;
    }
}

template<typename SampleType>
SampleType PianoWaveGenerator<SampleType>::generateSample() {
    static constexpr size_t NUM_HARMONICS = 6;
    // 谐波强度
    static const std::array<float, NUM_HARMONICS> harmonic_amplitudes = {
        1.0f, 0.4f, 0.2f, 0.1f, 0.05f, 0.03f
    };
    
    float sample = 0.0f;
    float envelope = this->calculateEnvelope();

    // 合成多个谐波
    for (size_t h = 0; h < NUM_HARMONICS; ++h) {
        uint32_t harmonic_phase = (this->phase_ * (h + 1)) & 0xFFFFFFFF;
        uint32_t table_index = (harmonic_phase >> 20) % TABLE_SIZE;
        float harmonic_wave = sine_table_[table_index];
        
        // 应用谐波强度和包络衰减
        float harmonic_amplitude = harmonic_amplitudes[h];
        if (h > 0) {
            // 高次谐波随包络衰减更快
            harmonic_amplitude *= std::pow(envelope, static_cast<float>(h) * 0.5f + 1.0f);
        }
        
        sample += harmonic_wave * harmonic_amplitude;
    }
    
    // 应用总包络和振幅
    sample *= envelope * this->amplitude_;
    
    // 更新相位和包络
    this->phase_ += this->phase_step_;
    this->updateEnvelope();
    
    // 转换为目标类型
    if constexpr (std::is_same_v<SampleType, int16_t>) {
        float scaled = sample * 32767.0f;
        if (scaled > 32767.0f) scaled = 32767.0f;
        if (scaled < -32767.0f) scaled = -32767.0f;
        return static_cast<int16_t>(scaled);
    } else if constexpr (std::is_same_v<SampleType, float>) {
        return sample;
    } else {
        return static_cast<SampleType>(sample);
    }
}

// ==================== WaveFactory 实现 ====================

template<typename SampleType>
std::unique_ptr<WaveGenerator<SampleType>> WaveFactory<SampleType>::create(WaveType type) {
    switch (type) {
        case WaveType::SINE:
            return std::make_unique<SineWaveGenerator<SampleType>>();
        case WaveType::SQUARE:
            return std::make_unique<SquareWaveGenerator<SampleType>>();
        case WaveType::TRIANGLE:
            return std::make_unique<TriangleWaveGenerator<SampleType>>();
        case WaveType::SAWTOOTH:
            return std::make_unique<SawtoothWaveGenerator<SampleType>>();
        case WaveType::PIANO:
            return std::make_unique<PianoWaveGenerator<SampleType>>();
        default:
            return std::make_unique<SineWaveGenerator<SampleType>>();
    }
}

} // namespace Audio 