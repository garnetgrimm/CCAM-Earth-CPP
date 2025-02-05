#include "daisysp.h"

#include <ccam/hw/estuary.h>
#include <ccam/voice/smoothosc.h>
#include <ccam/utils/quantizer.h>
#include <ccam/utils/delayenv.h>
#include <ccam/utils/lockedEstuaryKnobs.h>

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
std::array<DelayEnv, 2> envs;
std::array<bool, 2> gates;

LockedEstaury locked_knobs;

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
    gates[channel] = trig;
};

float randf() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void Process() {
    switch(hw.switches[1].Read()) {
        case daisy::Switch3::POS_LEFT:
            Step8x1();
            WriteStep(0, locked_knobs.Value(step_num, daisy::Switch3::POS_LEFT), true);
            WriteStep(1, randf(), true);
            hw.leds[step_num].Set(1.0f);
            break;
        case daisy::Switch3::POS_CENTER:
            Step4x2();
            WriteStep(0, locked_knobs.Value(step_num, daisy::Switch3::POS_CENTER), true);
            WriteStep(1, locked_knobs.Value(step_num+4, daisy::Switch3::POS_CENTER), true);
            hw.leds[step_num].Set(1.0f);
            hw.leds[step_num+4].Set(1.0f);
            break;
        case daisy::Switch3::POS_RIGHT:
            for (size_t i = 0; i < vcos.size(); i++) {
                vcos[i].SetWaveshape(locked_knobs.Value(4 + i, daisy::Switch3::POS_RIGHT));
            }
            for (DelayEnv& env : envs) {
                env.SetLength(locked_knobs.Value(2, daisy::Switch3::POS_RIGHT));
            }
            chance = hw.knobs[0]->Value();
            midi_start = daisysp::fmap(locked_knobs.Value(6, daisy::Switch3::POS_RIGHT), 1.0f, 100.f);
            midi_range = daisysp::fmap(locked_knobs.Value(7, daisy::Switch3::POS_RIGHT), 1.0f, 36.0f);
            if (randf() < chance) {
                WriteStep(0, randf(), true);
            }
            if (randf() < chance) {
                WriteStep(1, randf(), true);
            }
            hw.leds[0].Set(gates[0] ? 1.0f : 0.0f);
            hw.leds[1].Set(gates[1] ? 1.0f : 0.0f);
            break;
    }
    hw.som.gate_out_1.Write(gates[0]);
    hw.som.gate_out_2.Write(gates[1]);
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();
    locked_knobs.Process();

    for (uint8_t i = 0; i < hw.leds.size(); i++) {
        hw.leds[i].Set(0.0);
    }

    scale = locked_knobs.Value(1, daisy::Switch3::POS_RIGHT) * 8.0f;
    if (hw.switches[1].Read() == daisy::Switch3::POS_RIGHT) {
        hw.leds[scale].Set(1.0f);
    }

    for (size_t i = 0; i < size; i++)
    {
        if (clock.Process()) {
            if (clocking) {
                Process();
            } else {
                gates[0] = false;
                gates[1] = false;
                hw.som.gate_out_1.Write(false);
                hw.som.gate_out_2.Write(false);
            }
            clocking = !clocking;
        }

        OUT_L[i] = vcos[0].Process() * envs[0].Process(gates[0]);
        OUT_R[i] = vcos[1].Process() * envs[1].Process(gates[1]);
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();

    // give it a few cycles to initialize the knob values
    // otherwise everything is silent or 100%
    for (size_t i = 0; i < 10; i++) {
        daisy::System::Delay(1);
        hw.ProcessAllControls();
    }

    locked_knobs.Init(hw, 1);
    
    hw.som.StartLog(false);
    clock.Init(0.0f, hw.som.AudioSampleRate());
    clock.SetFreq(4.0f);

    for (SmoothOsc& vco : vcos) {
        vco.Init(hw.som.AudioSampleRate());
    }

    for (DelayEnv& env : envs) {
        env.Init(hw.som.AudioSampleRate());
    }

    hw.StartAudio(AudioCallback);

    while(1) {
        daisy::System::Delay(100);
        clock.SetFreq(daisysp::fmap(locked_knobs.Value(3, daisy::Switch3::POS_RIGHT), 0.1f, 30.0f));
    }
}