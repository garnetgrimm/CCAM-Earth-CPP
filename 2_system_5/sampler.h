#pragma once

namespace ccam {

using SampleBuffer = std::pair<uint16_t*, size_t>;

class SamplePlayer {
    float playback_sample_rate = 440000.0f;
    float recorded_sample_rate = 440000.0f;

    float current_idx = 0.0f;
    float playback_step = 1.0f;

    SampleBuffer sample_buffer;

    void UpdatePlayBackStep() {
        playback_step = recorded_sample_rate / playback_sample_rate;
    }

public:
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
