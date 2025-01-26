#include "estuary.h"
#include "daisysp.h"
#include "generator.h"
#include "smoothosc.h"

ccam::hw::Estuary hw;

daisysp::Metro clock;
bool clocking = false;

uint8_t step_num = 0;
static uint8_t seq_step = 1;

float max_frequency = 440.0f;

std::array<SmoothOsc, 2> vcos;
std::array<grids::PatternGenerator, 2> gens;
std::array<daisy::GPIO*, 2> gates = {
    &hw.som.gate_out_1,
    &hw.som.gate_out_2
};

using Scale = std::array<uint8_t, 8>;

Scale major_scale = { 0, 2, 4, 5, 7, 9, 11 };
Scale minor_scale = { 0, 2, 3, 5, 7, 8, 10 };

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

// daisysp has a mtof function, but not ftom..
inline float ftom(float freq) {
    return daisysp::fastlog2f((freq)/440.0f)*12.0f + 69.0f;
}

// frequency to 1v/oct helper
inline float ftov(float freq)
{
    // assume 0V = A1
    return daisysp::fastlog2f(freq/55.0f);
}

void quantize(Scale& scale, float& freq) {
    float note = ftom(freq);
    float octave = floorf(note / 12.0f);
    note = fmodf(note, 12.0f);

    uint8_t min_distance = 1.0f;
    uint8_t min_idx = 0;
    for (uint8_t i = 0; i < scale.size(); i++) {
        uint8_t distance = abs(note - scale[i]);
        if (distance < min_distance) {
            min_idx =  i;
            min_distance = distance;
        }
    }
    note = scale[min_idx];
    freq = daisysp::mtof(note + octave*12.0f);
}

void WriteStep(uint8_t channel, float value, bool trig) {
    float freq = daisysp::fmap(value, 88.0f, max_frequency);

    switch(hw.switches[0].Read()) {
        case daisy::Switch3::POS_LEFT:
            break;
        case daisy::Switch3::POS_CENTER:
            quantize(major_scale, freq);
            break;
        case daisy::Switch3::POS_RIGHT:
            quantize(minor_scale, freq);
            break;
    }

    hw.som.WriteCvOut(1 - channel, ftov(freq)); //cv channels switched??
    vcos[channel].SetFreq(freq);
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

                vcos[i].SetWaveshape(hw.knobs[6]->Value());
                max_frequency = daisysp::fmap(hw.knobs[7]->Value(), 110.0f, 880.0f);
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
        if (clock.Process()) {
            if (clocking) {
                Process();
            } else {
                for (daisy::GPIO* gate : gates) {
                    gate->Write(false);
                }
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
    clock.Init(0.0f, hw.som.AudioSampleRate());
    clock.SetFreq(4.0f);

    vcos[0].Init(hw.som.AudioSampleRate());
    vcos[1].Init(hw.som.AudioSampleRate());

    gens[0].SetInstrument(0);
    gens[1].SetInstrument(1);

    hw.StartAudio(AudioCallback);

    while(1) {
        daisy::System::Delay(100);

        if (hw.switches[1].Read() == daisy::Switch3::POS_RIGHT) {
            clock.SetFreq(daisysp::fmap(hw.knobs[3]->Value(), 0.1f, 30.0f));
        }
    }
}