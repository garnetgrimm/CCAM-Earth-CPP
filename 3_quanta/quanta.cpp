#include "daisysp.h"

#include <ccam/hw/estuary.h>
#include <ccam/utils/gubbins.h>
#include <ccam/utils/quantizer.h>

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
    -0.0009612f, 1.031f, -0.0009436f
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

    offset = hw.cvins[1]->Value()*5.0f*12.0f;
    inval = hw.cvins[0]->Value();

    raw_note = daisysp::fmap(inval, 0.0f, MAX_MIDI_NOTE) + 33.0f + offset;

    note = Quantizer::apply(
        Quantizer::Scale::MAJOR,
        raw_note
    );

    freq = daisysp::mtof(note);
    voltage = ftov(freq);
    voltage = adjust_voltage(voltage);

    hw.som.WriteCvOut(0, voltage);

    for (size_t i = 0; i < size; i++) {}

    hw.PostProcess();
}

int main(void)
{
    hw.Init();    
    hw.som.StartLog(false);
    hw.StartAudio(AudioCallback);

    while(1) {
        hw.som.PrintLine("input: %f output %f", raw_note, offset);
        daisy::System::Delay(100);
    }
}