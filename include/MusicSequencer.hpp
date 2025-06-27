#pragma once

#include "WaveGenerator.hpp"
#include "Notes.hpp"
#include <vector>
#include <memory>
#include <cstdint>
#include <string>

namespace Audio {

/**
 * @brief 音符结构
 */
struct Note {
    float frequency;
    uint32_t duration_ms;
    uint32_t pause_ms;
    float volume;
    std::string name;
    
    Note(float freq, uint32_t dur, uint32_t pause = 0, float vol = 1.0f, const std::string& note_name = "")
        : frequency(freq), duration_ms(dur), pause_ms(pause), volume(vol), name(note_name) {}
};

/**
 * @brief 音乐序列类型
 */
using MusicSequence = std::vector<Note>;

/**
 * @brief 播放状态枚举
 */
enum class PlaybackState {
    STOPPED,
    PLAYING,
    PAUSED
};



/**
 * @brief 音乐音序器类
 * 负责管理音符序列的播放和控制
 */
class MusicSequencer {
public:
    MusicSequencer();
    ~MusicSequencer();

    /**
     * @brief 设置音符序列
     * @param sequence 音符序列
     */
    void setSequence(const MusicSequence& sequence);

    /**
     * @brief 添加单个音符
     * @param note 音符
     */
    void addNote(const Note& note);

    /**
     * @brief 清空序列
     */
    void clearSequence();

    /**
     * @brief 开始播放
     */
    void play();

    /**
     * @brief 暂停播放
     */
    void pause();

    /**
     * @brief 停止播放
     */
    void stop();

    /**
     * @brief 播放指定索引的音符
     * @param index 音符索引
     */
    void playNote(size_t index);

    /**
     * @brief 设置波形类型
     * @param wave_type 波形类型
     */
    void setWaveType(WaveType wave_type);

    /**
     * @brief 获取当前播放状态
     * @return 播放状态
     */
    PlaybackState getState() const;

    /**
     * @brief 获取当前音符索引
     * @return 当前音符索引
     */
    size_t getCurrentNoteIndex() const;

    /**
     * @brief 获取音符总数
     * @return 音符总数
     */
    size_t getNoteCount() const;

    /**
     * @brief 生成音频样本
     * @param samples 样本缓冲区
     * @param sample_count 样本数量
     * @param sample_rate 采样率
     */
    void generateSamples(int16_t* samples, size_t sample_count, uint32_t sample_rate);

    /**
     * @brief 检查是否播放完成
     * @return 是否播放完成
     */
    bool isFinished() const;

    /**
     * @brief 设置循环播放
     * @param loop 是否循环
     */
    void setLoop(bool loop);

    /**
     * @brief 获取当前播放的音符
     * @return 当前音符的指针，如果没有则返回nullptr
     */
    const Note* getCurrentNote() const;

private:
    MusicSequence sequence_;
    std::unique_ptr<WaveGenerator<int16_t>> wave_generator_;
    
    PlaybackState state_;
    size_t current_note_index_;
    uint32_t current_note_samples_;
    uint32_t note_duration_samples_;
    uint32_t pause_duration_samples_;
    bool in_pause_;
    bool loop_;
    bool finished_;

    /**
     * @brief 更新当前音符状态
     * @param sample_rate 采样率
     */
    void updateNoteState(uint32_t sample_rate);

    /**
     * @brief 开始播放下一个音符
     * @param sample_rate 采样率
     */
    void startNextNote(uint32_t sample_rate);

    /**
     * @brief 计算音符持续时间的采样数
     * @param duration_ms 持续时间（毫秒）
     * @param sample_rate 采样率
     * @return 采样数
     */
    uint32_t msToSamples(uint32_t duration_ms, uint32_t sample_rate) const;
};

} // namespace Audio 