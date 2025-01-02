#include "estuary.h"
#include "daisysp.h"
#include "generator.h"

ccam::hw::Estuary hw;

daisysp::Metro clock;
float clk_freq = 240.0f / 60.0f;
size_t ticks_since_high = 0;

constexpr size_t NumGenChannels = 2;

grids::PatternGenerator patGens[NumGenChannels];
grids::EuclidianGenerator eucGens[NumGenChannels];

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {

    if (hw.som.gate_in_1.Trig()) {
        clk_freq = hw.som.AudioSampleRate() / static_cast<float>(ticks_since_high);
        clock.SetFreq(clk_freq);
        ticks_since_high = 0;
    } else {
        ticks_since_high++;
    }

    for (size_t i = 0; i < NumGenChannels; i++) {
        if (clock.Process()) {
            patGens[i].Tick();
        }

        patGens[i].x = hw.knobs[1]->Value();
        patGens[i].y = hw.knobs[2]->Value();
        patGens[i].chaos = hw.knobs[3]->Value();

        patGens[i].fill = hw.knobs[4 + i]->Value();
    }

    hw.som.gate_out_1.Write(patGens[0].Triggered());
    hw.som.gate_out_2.Write(patGens[1].Triggered());

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);

    clock.Init(clk_freq, hw.som.AudioSampleRate());

    for (size_t i = 0; i < NumGenChannels; i++) {
        patGens[i].SetInstrument(i);
    }

    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        daisy::System::Delay(1000);
    }
}