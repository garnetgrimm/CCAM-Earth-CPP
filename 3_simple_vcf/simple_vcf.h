#include "daisysp.h"

constexpr uint8_t MAX_MODE_IDX = static_cast<uint8_t>(daisysp::LadderFilter::FilterMode::HP12);

class SimpleVcf {
    daisysp::LadderFilter vcf;
    daisysp::LadderFilter::FilterMode type;
public:
    daisy::AnalogControl* freq_knob;
    daisy::AnalogControl* res_knob;
    daisy::AnalogControl* amp_knob;
    daisy::AnalogControl* type_knob;
    daisy::AnalogControl* cv_in;
    daisy::Switch3* mode;

    void Init(float sample_rate) {
        vcf.Init(sample_rate);
    }

    bool UseCV() {
        return mode->Read() == daisy::Switch3::POS_CENTER;
    }

    float Process(float in) {
        vcf.SetInputDrive(amp_knob->Value());
        vcf.SetRes(res_knob->Value());

        type = static_cast<daisysp::LadderFilter::FilterMode>(type_knob->Value() * (MAX_MODE_IDX + 1));
        vcf.SetFilterMode(type);

        daisy::AnalogControl* freq_ctrl = UseCV() ? cv_in : freq_knob;
        float freq = daisysp::fmap(freq_ctrl->Value(), 1.0f, 10000.0f);
        vcf.SetFreq(freq);

        return vcf.Process(in);
    }

    std::string GetType() {
        switch(type) {
            case daisysp::LadderFilter::FilterMode::LP24:
                return "LP24";
            case daisysp::LadderFilter::FilterMode::LP12:
                return "LP12";
            case daisysp::LadderFilter::FilterMode::BP24:
                return "BP24";
            case daisysp::LadderFilter::FilterMode::BP12:
                return "BP12";
            case daisysp::LadderFilter::FilterMode::HP24:
                return "HP24";
            case daisysp::LadderFilter::FilterMode::HP12:
                return "HP12";
            default:
                break;
        }
        return "ERR";
    };
};