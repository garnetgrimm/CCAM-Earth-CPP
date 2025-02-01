#include "estuary.h"
#include "daisysp.h"
#include "generator.h"
#include "smoothosc.h"
#include "quantizer.h"

ccam::hw::Estuary hw;

daisysp::Metro clock;
bool clocking = false;

uint8_t step_num = 0;
static uint8_t seq_step = 1;

float midi_start = 10.0f;
float midi_range = 36.0f;
uint8_t scale = 0;
float chance = 1.0;

std::array<SmoothOsc, 2> vcos;
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

// frequency to 1v/oct helper
inline float ftov(float freq)
{
    // assume 0V = A1
    return daisysp::fastlog2f(freq/55.0f);
}

float ChannelNoteOffset(uint8_t channel) {
    if (channel == 0) {
        return 0;
    }
    switch (hw.switches[0].Read()) {
        case daisy::Switch3::POS_CENTER:
            return 7.0f;
        case daisy::Switch3::POS_RIGHT:
            return 12.0f;
        default:
            return 0;
    }
}

void WriteStep(uint8_t channel, float value, bool trig) {
    float note = daisysp::fmap(value, midi_start, midi_start + midi_range);
    note += ChannelNoteOffset(channel);
    note = Quantizer::apply(static_cast<Quantizer::Scale>(scale), note);
    float freq = daisysp::mtof(note);
    hw.som.WriteCvOut(1 - channel, ftov(freq)); //cv channels switched??
    vcos[channel].SetFreq(freq);
    gates[channel]->Write(trig);
};

float randf() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void Process() {
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
            for (size_t i = 0; i < vcos.size(); i++) {
                vcos[i].SetWaveshape(hw.knobs[4 + i]->Value());
            }
            chance = hw.knobs[0]->Value();
            midi_start = daisysp::fmap(hw.knobs[6]->Value(), 1.0f, 100.f);
            midi_range = daisysp::fmap(hw.knobs[7]->Value(), 1.0f, 36.0f);
            WriteStep(0, randf(), randf() < chance);
            WriteStep(1, randf(), randf() < chance);
            break;
    }
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    for (uint8_t i = 0; i < hw.leds.size(); i++) {
        hw.leds[i].Set(0.0);
    }

    if (hw.switches[1].Read() == daisy::Switch3::POS_RIGHT) {
        scale = hw.knobs[1]->Value() * 8.0f;
        hw.leds[scale].Set(1.0f);
    }

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

    for (SmoothOsc& vco : vcos) {
        vco.Init(hw.som.AudioSampleRate());
    }

    hw.StartAudio(AudioCallback);

    while(1) {
        daisy::System::Delay(100);

        if (hw.switches[1].Read() == daisy::Switch3::POS_RIGHT) {
            clock.SetFreq(daisysp::fmap(hw.knobs[3]->Value(), 0.1f, 30.0f));
        }
    }
}