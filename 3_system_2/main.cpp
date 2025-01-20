#include "estuary.h"
#include "daisysp.h"

ccam::hw::Estuary hw;

daisysp::Metro metro;
daisysp::Adsr adsr;
daisysp::LadderFilter vcf1;
daisysp::LadderFilter vcf2;

daisysp::LadderFilter::FilterMode filter_mode_1() {
    switch(hw.switches[0].Read()) {
        case daisy::Switch3::POS_LEFT:
            return daisysp::LadderFilter::FilterMode::LP12;
        case daisy::Switch3::POS_CENTER:
            return daisysp::LadderFilter::FilterMode::BP12;
        case daisy::Switch3::POS_RIGHT:
            return daisysp::LadderFilter::FilterMode::HP12;
    }
    return daisysp::LadderFilter::FilterMode::LP12;
}

daisysp::LadderFilter::FilterMode filter_mode_2() {
    switch(hw.switches[1].Read()) {
        case daisy::Switch3::POS_LEFT:
            return daisysp::LadderFilter::FilterMode::LP24;
        case daisy::Switch3::POS_CENTER:
            return daisysp::LadderFilter::FilterMode::BP24;
        case daisy::Switch3::POS_RIGHT:
            return daisysp::LadderFilter::FilterMode::HP24;
    }
    return daisysp::LadderFilter::FilterMode::LP24;
}

float knob_cv_combo(uint8_t kidx, uint8_t cidx) {
    return hw.knobs[kidx]->Value() + hw.cvins[cidx]->Value();
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    vcf1.SetFilterMode(filter_mode_1());
    vcf2.SetFilterMode(filter_mode_2());

    adsr.SetAttackTime(hw.knobs[0]->Value());
    adsr.SetDecayTime(hw.knobs[1]->Value());
    adsr.SetSustainLevel(hw.knobs[2]->Value());
    adsr.SetReleaseTime(hw.knobs[3]->Value());

    vcf1.SetFreq(daisysp::fmap(knob_cv_combo(4, 0), 8.0f, 8000.0f));
    vcf1.SetRes(knob_cv_combo(5, 1));

    vcf2.SetFreq(daisysp::fmap(knob_cv_combo(6, 2), 8.0f, 8000.0f));
    vcf2.SetRes(knob_cv_combo(7, 3));

    if (hw.som.gate_in_2.Trig()) {
        adsr.Retrigger(false);
    }
    hw.som.WriteCvOut(0, adsr.Process(hw.som.gate_in_1.State()) * 5.0f);

    for (size_t i = 0; i < size; i++)
    {
        
        OUT_L[i] = vcf1.Process(IN_L[i]);
        OUT_R[i] = vcf2.Process(IN_R[i]);
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    vcf1.Init(hw.som.AudioSampleRate());
    vcf2.Init(hw.som.AudioSampleRate());
    adsr.Init(hw.som.AudioCallbackRate());
    hw.som.StartLog(false);
    hw.StartAudio(AudioCallback);
    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        hw.som.PrintLine("CV0 %f", fabs(hw.cvins[0]->Value()));
        daisy::System::Delay(250);
    }
}