#include "MusicSequencer.hpp"

namespace Audio {

MusicSequencer::MusicSequencer() 
    : state_(PlaybackState::STOPPED),
      current_note_index_(0),
      current_note_samples_(0),
      note_duration_samples_(0),
      pause_duration_samples_(0),
      in_pause_(false),
      loop_(false),
      finished_(false) {
    
    // 创建默认的正弦波生成器（参考simple_audio_test.cpp）
    wave_generator_ = WaveFactory<int16_t>::create(WaveType::SINE);
    
    // 设置简化ADSR包络（模拟simple_audio_test.cpp的简单音频生成）
    ADSREnvelope envelope;
    envelope.attack_samples = 0;          // 无攻击时间，立即达到最大音量
    envelope.decay_samples = 0;           // 无衰减时间
    envelope.sustain_level = 1.0f;        // 维持满音量
    envelope.release_samples = 32000 * 10 / 1000; // 10ms快速释放 @ 32kHz
    wave_generator_->setEnvelope(envelope);
    wave_generator_->setAmplitude(0.3f);  // 适中振幅，与simple_audio_test.cpp兼容
}

MusicSequencer::~MusicSequencer() = default;

void MusicSequencer::setSequence(const MusicSequence& sequence) {
    sequence_ = sequence;
    current_note_index_ = 0;
    current_note_samples_ = 0;
    in_pause_ = false;
    finished_ = false;
    
    if (wave_generator_) {
        wave_generator_->noteOff();
        wave_generator_->resetPhase();
    }
}

void MusicSequencer::addNote(const Note& note) {
    sequence_.push_back(note);
}

void MusicSequencer::clearSequence() {
    sequence_.clear();
    current_note_index_ = 0;
    current_note_samples_ = 0;
    in_pause_ = false;
    finished_ = false;
    
    if (wave_generator_) {
        wave_generator_->noteOff();
        wave_generator_->resetPhase();
    }
}

void MusicSequencer::play() {
    if (sequence_.empty()) return;
    
    state_ = PlaybackState::PLAYING;
    if (finished_ || current_note_index_ >= sequence_.size()) {
        current_note_index_ = 0;
        current_note_samples_ = 0;
        in_pause_ = false;
        finished_ = false;
    }
}

void MusicSequencer::pause() {
    state_ = PlaybackState::PAUSED;
    if (wave_generator_) {
        wave_generator_->noteOff();
    }
}

void MusicSequencer::stop() {
    state_ = PlaybackState::STOPPED;
    current_note_index_ = 0;
    current_note_samples_ = 0;
    in_pause_ = false;
    finished_ = false;
    
    if (wave_generator_) {
        wave_generator_->noteOff();
        wave_generator_->resetPhase();
    }
}

void MusicSequencer::playNote(size_t index) {
    if (index >= sequence_.size()) return;
    
    current_note_index_ = index;
    current_note_samples_ = 0;
    in_pause_ = false;
    finished_ = false;
    state_ = PlaybackState::PLAYING;
}

void MusicSequencer::setWaveType(WaveType wave_type) {
    if (!wave_generator_) return;
    
    // 保存当前状态（简化包络设置）
    ADSREnvelope current_envelope;
    current_envelope.attack_samples = 0;          // 无攻击时间，立即达到最大音量
    current_envelope.decay_samples = 0;           // 无衰减时间
    current_envelope.sustain_level = 1.0f;        // 维持满音量
    current_envelope.release_samples = 32000 * 10 / 1000; // 10ms快速释放 @ 32kHz
    float current_amplitude = 0.3f;  // 适中振幅，与simple_audio_test.cpp兼容
    
    // 创建新的波形生成器
    wave_generator_ = WaveFactory<int16_t>::create(wave_type);
    wave_generator_->setEnvelope(current_envelope);
    wave_generator_->setAmplitude(current_amplitude);
}

PlaybackState MusicSequencer::getState() const {
    return state_;
}

size_t MusicSequencer::getCurrentNoteIndex() const {
    return current_note_index_;
}

size_t MusicSequencer::getNoteCount() const {
    return sequence_.size();
}

void MusicSequencer::generateSamples(int16_t* samples, size_t sample_count, uint32_t sample_rate) {
    if (state_ != PlaybackState::PLAYING || sequence_.empty() || !wave_generator_) {
        // 填充静音
        std::fill(samples, samples + sample_count, int16_t(0));
        return;
    }
    
    if (wave_generator_) {
        wave_generator_->setSampleRate(sample_rate);
    }
    
    for (size_t i = 0; i < sample_count; ++i) {
        // 更新音符状态
        updateNoteState(sample_rate);
        
        // 生成样本
        if (!finished_ && !in_pause_) {
            samples[i] = wave_generator_->generateSample();
        } else {
            samples[i] = 0;
        }
        
        current_note_samples_++;
    }
}

bool MusicSequencer::isFinished() const {
    return finished_;
}

void MusicSequencer::setLoop(bool loop) {
    loop_ = loop;
}

const Note* MusicSequencer::getCurrentNote() const {
    if (current_note_index_ < sequence_.size()) {
        return &sequence_[current_note_index_];
    }
    return nullptr;
}

void MusicSequencer::updateNoteState(uint32_t sample_rate) {
    if (finished_ || current_note_index_ >= sequence_.size()) {
        return;
    }
    
    const Note& note = sequence_[current_note_index_];
    
    if (!in_pause_) {
        // 播放音符阶段
        if (note_duration_samples_ == 0) {
            // 开始新音符
            note_duration_samples_ = msToSamples(note.duration_ms, sample_rate);
            pause_duration_samples_ = msToSamples(note.pause_ms, sample_rate);
            
            if (wave_generator_) {
                wave_generator_->setFrequency(note.frequency);
                wave_generator_->setAmplitude(0.3f * note.volume);  // 适中振幅，与simple_audio_test.cpp兼容
                wave_generator_->noteOn();
            }
        }
        
        if (current_note_samples_ >= note_duration_samples_) {
            // 音符播放完成，进入暂停阶段
            if (wave_generator_) {
                wave_generator_->noteOff();
            }
            
            if (pause_duration_samples_ > 0) {
                in_pause_ = true;
                current_note_samples_ = 0;
            } else {
                // 没有暂停，直接下一个音符
                startNextNote(sample_rate);
            }
        }
    } else {
        // 暂停阶段
        if (current_note_samples_ >= pause_duration_samples_) {
            startNextNote(sample_rate);
        }
    }
}

void MusicSequencer::startNextNote(uint32_t sample_rate) {
    current_note_index_++;
    current_note_samples_ = 0;
    note_duration_samples_ = 0;
    pause_duration_samples_ = 0;
    in_pause_ = false;
    
    if (current_note_index_ >= sequence_.size()) {
        // 序列播放完成
        if (loop_) {
            // 循环播放
            current_note_index_ = 0;
        } else {
            // 播放完成
            finished_ = true;
            state_ = PlaybackState::STOPPED;
        }
    }
}

uint32_t MusicSequencer::msToSamples(uint32_t duration_ms, uint32_t sample_rate) const {
    return (duration_ms * sample_rate) / 1000;
}

} // namespace Audio 