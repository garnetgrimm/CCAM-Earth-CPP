#include "daisysp.h"

constexpr uint8_t MAX_WAVE_IDX = static_cast<uint8_t>(daisysp::Oscillator::WAVE_SQUARE);

class SimpleVco {
    float freq = 220.0f;
    float amp = 1.0;
    uint8_t type = 0;
    daisysp::Oscillator osc;
public:
    daisy::AnalogControl* freq_knob;
    daisy::AnalogControl* tune_knob;
    daisy::AnalogControl* amp_knob;
    daisy::AnalogControl* type_knob;
    daisy::AnalogControl* cv_in;
    daisy::Switch3* mode;

    void Init(float sample_rate) {
        osc.Init(sample_rate);
    }

    bool UseCV() {
        return mode->Read() == daisy::Switch3::POS_CENTER;
    }

    float Process() {
        type = type_knob->Value() * (MAX_WAVE_IDX + 1);

        if (UseCV()) {
            freq = daisysp::fmap(cv_in->Value(), 44.0f, 440.0f);
        } else {
            freq = daisysp::fmap(freq_knob->Value(), 44.0f, 440.0f);
            freq += daisysp::fmap(tune_knob->Value(), -5.0f, 5.0f);
        }

        amp = amp_knob->Value();

        osc.SetFreq(freq);
        osc.SetAmp(amp);
        osc.SetWaveform(type);

        return osc.Process();
    }

    std::string GetType() {
        switch(type) {
            case daisysp::Oscillator::WAVE_SIN:
                return "SIN";
            case daisysp::Oscillator::WAVE_TRI:
                return "TRI";
            case daisysp::Oscillator::WAVE_SAW:
                return "SAW";
            case daisysp::Oscillator::WAVE_RAMP:
                return "RMP";
            case daisysp::Oscillator::WAVE_SQUARE:
                return "SQR";
            default:
                break;
        }
        return "ERR";
    };
};