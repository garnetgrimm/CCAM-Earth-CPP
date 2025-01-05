#include "estuary.h"
#include "daisysp.h"

ccam::hw::Estuary hw;

struct SyntheticHihat {
    daisysp::WhiteNoise noise;
    daisysp::Svf filter;
    daisysp::Adsr adsr;

    void Init(float sample_rate) {
        adsr.Init(sample_rate);
        filter.Init(sample_rate);

        adsr.SetAttackTime(0.01);
        adsr.SetDecayTime(0.0f);
        adsr.SetSustainLevel(1.0f);
    }

    float Process() {
        filter.Process(noise.Process());
        return filter.Band() * adsr.Process(true);
    }

    void SetDecay(float timeInS) {
        adsr.SetReleaseTime(timeInS);
    }
    
    void SetFreq(float f) {
        filter.SetFreq(f);
    }

    void Trig() {
        return adsr.Retrigger(true);
    }
};

daisysp::SyntheticBassDrum bass;
SyntheticHihat hihat;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {

    hw.ProcessAllControls();

    if (hw.som.gate_in_1.Trig()) {
        bass.Trig();
    }

    if (hw.som.gate_in_2.Trig()) {
        hihat.Trig();
    }

    bass.SetFreq(hw.knobs[0]->Value());
    bass.SetDecay(hw.knobs[1]->Value() + hw.cvins[1]->Value());
    bass.SetDirtiness(hw.knobs[2]->Value() + hw.cvins[0]->Value());
    bass.SetAccent(hw.knobs[3]->Value());

    hihat.SetFreq(hw.knobs[4]->Value() + hw.cvins[2]->Value());
    hihat.SetDecay(hw.knobs[5]->Value() + hw.cvins[3]->Value());

    for (size_t i = 0; i < size; i++) {
        OUT_L[i] = bass.Process();
        OUT_R[i] = hihat.Process();
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    bass.Init(hw.som.AudioSampleRate());
    hihat.Init(hw.som.AudioSampleRate());
    
    hw.StartAudio(AudioCallback);
    
    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        hw.PostProcess();
        daisy::System::Delay(1000);
    }
}