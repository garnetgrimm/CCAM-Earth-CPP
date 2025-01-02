#include "estuary.h"
#include "daisysp.h"
#include "generator.h"

ccam::hw::Estuary hw;

daisysp::Metro clock;
float clk_freq = 240.0f / 60.0f;
size_t clk_ticks = 0;
size_t ticks_since_high = 0;

constexpr size_t NumGenChannels = 2;
constexpr size_t ClkInTimeout = 8;

grids::PatternGenerator patGens[NumGenChannels];
grids::EuclidianGenerator eucGens[NumGenChannels];

bool IsEuclidian() {
    if (hw.switches[1].Read() == daisy::Switch3::POS_CENTER) {
        return false;
    }
    return true;
}

void TickPatGen() {
    for (size_t i = 0; i < NumGenChannels; i++) {
        if (clock.Process()) {
            patGens[i].Tick();
        }
        patGens[i].x = hw.knobs[1]->Value();
        patGens[i].y = hw.knobs[2]->Value();
        patGens[i].chaos = hw.knobs[3]->Value();
        patGens[i].fill = hw.knobs[i + 4]->Value();
    }

    hw.som.gate_out_1.Write(patGens[0].Triggered());
    hw.som.gate_out_2.Write(patGens[1].Triggered());
}

void TickEucGen() {
    for (size_t i = 0; i < NumGenChannels; i++) {
        if (clock.Process()) {
            eucGens[i].Tick();
        }
        eucGens[i].SetLength(static_cast<uint8_t>(hw.knobs[i]->Value() * grids::kStepsPerPattern));
        eucGens[i].fill = hw.knobs[i + 4]->Value();
    }

    hw.som.gate_out_1.Write(eucGens[0].Triggered());
    hw.som.gate_out_2.Write(eucGens[1].Triggered());
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {

    if (hw.som.gate_in_1.Trig()) {
        clk_ticks = ticks_since_high;
        clk_freq = hw.som.AudioCallbackRate() / static_cast<float>(clk_ticks);
        clock.SetFreq(clk_freq);
        ticks_since_high = 0;
    } else {
        if (ticks_since_high > clk_ticks * ClkInTimeout) {
            clk_ticks = 0;
            clk_freq = daisysp::fmap(hw.knobs[0]->Value(), 0.1f, 10.0f);
            clock.SetFreq(clk_freq);
        } else {
            ticks_since_high++;
        }
    }

    IsEuclidian() ? TickEucGen() : TickPatGen();

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);

    clock.Init(clk_freq, hw.som.AudioSampleRate());

    patGens[0].SetInstrument(0); // bass drum
    patGens[1].SetInstrument(2); // high hat

    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        daisy::System::Delay(1000);
    }
}