#include <ccam/hw/estuary.h>
#include <ccam/voice/tonedrum.h>
#include <ccam/voice/noisedrum.h>
#include "daisysp.h"

ccam::hw::Estuary hw;
daisysp::Metro metro;

ToneDrum tone_drum;
NoiseDrum noise_drum;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    float tone_freq = hw.knobs[0]->Value() + hw.cvins[0]->Value();
    tone_freq = daisysp::fmap(tone_freq, 5.0f, 880.0f);
    float tone_len = hw.knobs[1]->Value() + hw.cvins[1]->Value();

    float noise_freq = hw.knobs[4]->Value() + hw.cvins[2]->Value();
    noise_freq = daisysp::fmap(noise_freq, 5.0f, 4000.0f);
    float noise_len = hw.knobs[5]->Value() + hw.cvins[3]->Value();

    tone_drum.SetFreq(tone_freq);
    tone_drum.SetLength(tone_len);
    tone_drum.SetFmAmount(hw.knobs[2]->Value() * 0.1);
    tone_drum.SetFmLength(hw.knobs[3]->Value());

    noise_drum.SetFreq(noise_freq);
    noise_drum.SetLength(noise_len);

    tone_drum.SetAmp(hw.knobs[6]->Value());
    noise_drum.SetAmp(hw.knobs[7]->Value());

    bool tone_trig = hw.som.gate_in_1.Trig();
    bool noise_trig = hw.som.gate_in_2.Trig();

    for (size_t i = 0; i < size; i++)
    {
        
        OUT_L[i] = tone_drum.Process(tone_trig);
        OUT_R[i] = noise_drum.Process(noise_trig);
    }

    hw.PostProcess();
}


int main(void)
{
    hw.Init();

    hw.som.StartLog(false);

    tone_drum.Init(hw.som.AudioSampleRate());
    noise_drum.Init(hw.som.AudioSampleRate());
    
    hw.StartAudio(AudioCallback);

    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        daisy::System::Delay(1000);
    }
}