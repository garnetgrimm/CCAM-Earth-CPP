#pragma once

namespace ccam {

using SampleBuffer = std::pair<uint16_t*, size_t>;

constexpr uint16_t U16_DIRECTION_MASK = static_cast<uint16_t>(1<<15);
constexpr uint16_t U16_MAGNITUDE_MASK = static_cast<uint16_t>(~U16_DIRECTION_MASK);

class SamplePlayer {
    float playback_sample_rate = 440000.0f;
    float recorded_sample_rate = 440000.0f;

    float current_idx = 0.0f;
    float playback_step = 1.0f;

    SampleBuffer sample_buffer;

    bool playing = false;

    void UpdatePlayBackStep() {
        playback_step = recorded_sample_rate / playback_sample_rate;
    }

    float u16_to_float(uint16_t sample) {
        float direction = (sample & U16_DIRECTION_MASK) ? 1.0f : -1.0f;
        float magnitude = static_cast<float>(sample & U16_MAGNITUDE_MASK);
        magnitude /= static_cast<float>(U16_MAGNITUDE_MASK);
        return direction * magnitude;
    }

    float get_sample(size_t index) {
        return sample_buffer.first[index % sample_buffer.second];
    }

    float get_sample(float index) {
        float samp_start = get_sample(
            static_cast<size_t>(std::floorf(index))
        );
        float samp_end = get_sample(
            static_cast<size_t>(std::ceil(index))
        );
        return (samp_start + samp_end) / 2.0f;
    }

public:
    float Tick(bool trigger) {
        if (trigger && !playing) {
            current_idx = 0.0f;
            playing = true;
        }

        if (current_idx >= sample_buffer.second) {
            playing = false;
        }

        if (playing) {
            current_idx += playback_step;
        }

        return get_sample(current_idx);
    }

    void SetPlaybackSampleRate(float sr) {
        playback_sample_rate = sr;
        UpdatePlayBackStep();
    }

    void SetRecordedSampleRate(float sr) {
        recorded_sample_rate = sr;
        UpdatePlayBackStep();
    }

    void SetSample(SampleBuffer sb) {
        sample_buffer = sb;
    }


    
};

}
