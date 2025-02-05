#include <ccam/hw/estuary.h>
#include <ccam/voice/tonedrum.h>
#include <ccam/voice/noisedrum.h>
#include <ccam/utils/lockedEstuaryKnobs.h>
#include <ccam/utils/gubbins.h>
#include "daisysp.h"

bool ledOn = false;

ccam::hw::Estuary hw;
daisysp::Metro clock;

ToneDrum tone_drum;
NoiseDrum noise_drum;

using LockedCvKnob = CvKnob<LockedAnalogControl, daisy::AnalogControl>;

LockedCvKnob tone_decay;
LockedCvKnob tone_freq;
LockedCvKnob fm_amount;
LockedCvKnob fm_decay;

LockedCvKnob noise_decay;
LockedCvKnob noise_freq;

LockedCvKnob tone_amp;
LockedCvKnob noise_amp;

LockedCvKnob tempo;

LockedEstaury knobs_vco;
LockedEstaury knobs_seq;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();
    knobs_vco.Process();
    knobs_seq.Process();

    tone_drum.SetFreq(daisysp::fmap(tone_freq.Value(), 5.0f, 880.0f));
    tone_drum.SetLength(1.0f - tone_decay.Value());
    tone_drum.SetFmAmount(fm_amount.Value() * 0.1);
    tone_drum.SetFmLength(1.0f - fm_decay.Value());

    noise_drum.SetFreq(daisysp::fmap(noise_freq.Value(), 5.0f, 4000.0f));
    noise_drum.SetLength(1.0f - noise_decay.Value());

    tone_drum.SetAmp(tone_amp.Value());
    noise_drum.SetAmp(noise_amp.Value());

    bool tone_trig = hw.som.gate_in_1.Trig();
    bool noise_trig = hw.som.gate_in_2.Trig();

    for (size_t i = 0; i < size; i++)
    {
        if (clock.Process()) {
            ledOn = !ledOn;
            hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        }
        
        OUT_L[i] = tone_drum.Process(tone_trig);
        OUT_R[i] = noise_drum.Process(noise_trig);
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

    knobs_vco.Init(hw, 1, (1 << daisy::Switch3::POS_LEFT) | (1 << daisy::Switch3::POS_CENTER));
    knobs_seq.Init(hw, 1, (1 << daisy::Switch3::POS_RIGHT));

    tone_decay.Init(&knobs_vco.Get(0), hw.cvins[0]);
    tone_freq.Init(&knobs_vco.Get(1), hw.cvins[1]);
    fm_amount.Init(&knobs_vco.Get(2), hw.cvins[2]);
    fm_decay.Init(&knobs_vco.Get(3), hw.cvins[3]);

    noise_decay.Init(&knobs_vco.Get(4), nullptr);
    noise_freq.Init(&knobs_vco.Get(5), nullptr);
    tone_amp.Init(&knobs_vco.Get(6), nullptr);
    noise_amp.Init(&knobs_vco.Get(7), nullptr);

    tempo.Init(&knobs_seq.Get(3), nullptr);

    hw.som.StartLog(false);

    clock.Init(10.0f, hw.som.AudioSampleRate());

    tone_drum.Init(hw.som.AudioSampleRate());
    noise_drum.Init(hw.som.AudioSampleRate());
    
    hw.StartAudio(AudioCallback);

    while(1) {
        daisy::System::Delay(100);
        clock.SetFreq(daisysp::fmap(tempo.Value(), 0.1f, 30.0f));
    }
}