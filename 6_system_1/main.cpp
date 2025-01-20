#include "estuary.h"
#include "daisysp.h"
#include "generator.h"

ccam::hw::Estuary hw;

daisysp::Metro clock;
// HZ = (BPM / 60)
float clk_freq = 240.0f / 60.0f;

size_t ticks_since_high = 0;
float inverse_sample_rate = 0.0f;

bool clocking = false;

uint8_t step_num = 0;
static uint8_t seq_step = 1;

std::array<daisysp::Oscillator, 2> vcos;
std::array<grids::PatternGenerator, 2> gens;
std::array<daisy::GPIO*, 2> gates = {
    &hw.som.gate_out_1,
    &hw.som.gate_out_2
};

void Step8x1() {
    step_num = (step_num + 1) % 8;
}

void Step4x2() {
    if (step_num >= 3) {
        seq_step = -1;
    } else if (step_num == 0) {
        seq_step = 1;
    }
    step_num = (step_num + seq_step) % 4;
}

void StepGrid() {
    gens[0].Tick();
    gens[1].Tick();
}

void WriteStep(uint8_t channel, float value, bool trig) {
    hw.som.WriteCvOut(channel, value*5.0f);
    vcos[channel].SetFreq(daisysp::fmap(value, 44.0f, 440.0f));
    gates[channel]->Write(trig);
};

float randf() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void Process() {
    for (uint8_t i = 0; i < hw.leds.size(); i++) {
        hw.leds[i].Set(0.0);
    }

    switch(hw.switches[1].Read()) {
        case daisy::Switch3::POS_LEFT:
            Step8x1();
            WriteStep(0, hw.knobs[step_num]->Value(), true);
            WriteStep(1, randf(), true);
            hw.leds[step_num].Set(1.0f);
            break;
        case daisy::Switch3::POS_CENTER:
            Step4x2();
            WriteStep(0, hw.knobs[step_num]->Value(), true);
            WriteStep(1, hw.knobs[step_num+4]->Value(), true);
            hw.leds[step_num].Set(1.0f);
            hw.leds[step_num+4].Set(1.0f);
            break;
        case daisy::Switch3::POS_RIGHT:
            for (size_t i = 0; i < gens.size(); i++) {
                gens[i].x = hw.knobs[0]->Value();
                gens[i].y = hw.knobs[1]->Value();
                gens[i].chaos = hw.knobs[2]->Value();
                gens[i].fill = hw.knobs[i + 4]->Value();
            }

            for (int i = 0; i < 8; i++) {
                StepGrid();

                if (gens[0].Triggered()) {
                    WriteStep(0, randf(), gens[0].Triggered());
                    hw.leds[0].Set(gens[0].Triggered() ? 1.0f : 0.0f);
                }
                if (gens[1].Triggered()) {
                    WriteStep(1, randf(), gens[1].Triggered());
                    hw.leds[1].Set(gens[1].Triggered() ? 1.0f : 0.0f);
                }
            }
            
            break;
    }
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    for (size_t i = 0; i < size; i++)
    {
        if (hw.som.gate_in_1.Trig()) {
            clk_freq = hw.som.AudioSampleRate() / static_cast<float>(ticks_since_high);
            clock.SetFreq(clk_freq * 2.0f);
            ticks_since_high = 0;
        } else {
            ticks_since_high++;
        }

        if (clock.Process()) {
            if (clocking) {
                Process();
            } else {
                hw.som.gate_out_1.Write(false);
            }
            clocking = !clocking;
        }

        OUT_L[i] = vcos[0].Process();
        OUT_R[i] = vcos[1].Process();
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    
    hw.som.StartLog(false);
    clock.Init(clk_freq, hw.som.AudioSampleRate());

    vcos[0].Init(hw.som.AudioSampleRate());
    vcos[1].Init(hw.som.AudioSampleRate());

    gens[0].SetInstrument(0);
    gens[1].SetInstrument(1);

    hw.StartAudio(AudioCallback);

    while(1) {
        hw.som.PrintLine("Step %d tempo %f", step_num+1, clk_freq);
        daisy::System::Delay(100);
    }
}