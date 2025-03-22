#include "daisysp.h"

#include <ccam/hw/estuary.h>
#include <ccam/utils/gubbins.h>
#include <ccam/utils/quantizer.h>

#define CALIBRATION_MODE_OFF 0
#define CALIBRATION_MODE_RAW 1
#define CALIBRATION_MODE_COF 2
#define CALIBRATION_MODE_VCO 3

#define QUANTA_CALIBRATION_MODE CALIBRATION_MODE_OFF

ccam::hw::Estuary hw;

constexpr float MAX_MIDI_NOTE = 12.0f * 5.0f;
float raw_note = 0.0f;
float note = 0.0f;
float inval = 0.0f;
float outval = 0.0f;
float freq = 0.0f;
float voltage = 0.0f;
float offset = 0.0f;

// coefficents from python calibration
constexpr std::array<float, 3> coeffs = {
    -0.002163f, 1.037f, -0.01769f
};

float adjust_voltage(float target) {
    float result = 0.0f;
    result += coeffs[0] * target * target;
    result += coeffs[1] * target;
    result += coeffs[2];
    return result;
}


static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

#if QUANTA_CALIBRATION_MODE == CALIBRATION_MODE_RAW

    // determine tuning coefficents (see python script)
    voltage = std::floorf(hw.knobs[0]->Value()*60.0f)/10.0f;
    hw.som.WriteCvOut(0, voltage);

#elif QUANTA_CALIBRATION_MODE == CALIBRATION_MODE_COF

    // validate tuning cofficents or re-measure
    voltage = std::floorf(hw.knobs[0]->Value()*6.0f);
    hw.som.WriteCvOut(0, adjust_voltage(voltage));

#elif QUANTA_CALIBRATION_MODE == CALIBRATION_MODE_VCO

    // use a vco to confirm that all the notes are correct
    raw_note = daisysp::fmap(hw.knobs[0]->Value(), 0.0f, MAX_MIDI_NOTE) + 33.0f;
    note = Quantizer::apply(Quantizer::Scale::MAJOR, raw_note);
    freq = daisysp::mtof(note);
    voltage = ftov(freq);
    hw.som.WriteCvOut(0, adjust_voltage(voltage));

#else

    // regular operation
    auto max_scale = static_cast<float>(Quantizer::Scale::COUNT);
    auto scale = static_cast<Quantizer::Scale>(hw.knobs[0]->Value() * max_scale);

    for (uint8_t i = 0; i < hw.leds.size(); i++) {
        hw.leds[i].Set(0.0);
    }

    hw.leds[static_cast<uint8_t>(scale)].Set(1.0f);

    raw_note = daisysp::fmap(hw.cvins[0]->Value(), 0.0f, MAX_MIDI_NOTE) + 33.0f;
    note = Quantizer::apply(scale, raw_note);
    freq = daisysp::mtof(note);
    voltage = ftov(freq);
    hw.som.WriteCvOut(0, adjust_voltage(voltage));

#endif

    for (size_t i = 0; i < size; i++) {}

    hw.PostProcess();
}

int main(void)
{
    hw.Init();    
    hw.som.StartLog(false);
    hw.StartAudio(AudioCallback);

    while(1) {
        if (QUANTA_CALIBRATION_MODE != CALIBRATION_MODE_OFF) {
            hw.som.PrintLine("voltage: %f note: %f", voltage, note);
        }
        daisy::System::Delay(100);
    }
}