#pragma once

#include "sampler.h"

namespace ccam {

class TR606Sampler {
    enum class SampleType
    {
        CYMBOL,
        HAT_CLOSED,
        HAT_OPENED,
        HAT_PEDAL,
        KICK,
        SNARE,
        TOM_HIGH,
        TOM_LOW,
    };

    SampleBuffer get_cymbol();
    SampleBuffer get_hat_closed();
    SampleBuffer get_hat_opened();
    SampleBuffer get_hat_pedal();
    SampleBuffer get_kick();
    SampleBuffer get_snare();
    SampleBuffer get_tom_high();
    SampleBuffer get_tom_low();

    uint8_t sample_option = 0;
    SampleType sample_type = SampleType::KICK;
public:
    SampleBuffer get_sample();

    void SetSampleType(SampleType sample_type) {
        this->sample_type = sample_type;
    }

    void SetSampleOption(uint8_t sample_option) {
        this->sample_option = sample_option;
    }
};

}
