#include "estuary.h"
#include "daisysp.h"

ccam::hw::Estuary hw;
daisysp::Metro clock;
daisysp::HiHat hihat;

daisysp::SyntheticBassDrum bass1;
daisysp::AnalogBassDrum bass2;

static void ConfigureBass(auto& drum) {
    drum.SetFreq(daisysp::fmap(hw.knobs[0]->Value(), 10.0f, 200.0f));
    drum.SetDecay(hw.knobs[1]->Value() + hw.cvins[1]->Value());
    drum.SetAccent(hw.knobs[2]->Value());
    // todo: add my own distortion

    if (hw.som.gate_in_1.Trig()) {
        drum.Trig();
    }
}

static void ConfigureHihat() {
    hihat.SetFreq(hw.knobs[4]->Value() + hw.cvins[2]->Value());
    hihat.SetDecay(hw.knobs[5]->Value() + hw.cvins[3]->Value());
    hihat.SetTone(hw.knobs[6]->Value());
    hihat.SetNoisiness(hw.knobs[7]->Value());

    if (hw.som.gate_in_2.Trig()) {
        hihat.Trig();
    }
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {

    hw.ProcessAllControls();

    bool bass_mode = hw.switches[1].Read() == daisy::Switch3::POS_CENTER;
    
    bass_mode ? ConfigureBass(bass1) : ConfigureBass(bass2);
    ConfigureHihat();

    for (size_t i = 0; i < size; i++) {
        if (clock.Process()) {
            hw.som.gate_out_1.Toggle();
        }

        OUT_L[i] = bass_mode ? bass1.Process() : bass2.Process();
        OUT_R[i] = hihat.Process();
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    bass1.Init(hw.som.AudioSampleRate());
    bass2.Init(hw.som.AudioSampleRate());
    hihat.Init(hw.som.AudioSampleRate());

    clock.Init(4.0f, hw.som.AudioSampleRate());
    
    hw.StartAudio(AudioCallback);
    hw.som.StartLog(false);
    
    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        hw.PostProcess();
        daisy::System::Delay(1000);
    }
}