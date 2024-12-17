#include "estuary.h"
#include "simple_vco.h"

ccam::hw::Estuary hw;

SimpleVco lvco;
SimpleVco rvco;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    for (size_t i = 0; i < size; i++)
    {
        OUT_L[i] = lvco.Process();
        OUT_R[i] = rvco.Process();
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    lvco.Init(hw.som.AudioSampleRate());
    rvco.Init(hw.som.AudioSampleRate());

    hw.som.StartLog(false);
    hw.StartAudio(AudioCallback);

    lvco.freq_knob = hw.knobs[0];
    lvco.tune_knob = hw.knobs[1];
    lvco.type_knob = hw.knobs[2];
    lvco.amp_knob = hw.knobs[3];
    lvco.cv_in = hw.cvins[0];
    lvco.mode = &hw.switches[0];

    rvco.freq_knob = hw.knobs[4];
    rvco.tune_knob = hw.knobs[5];
    rvco.type_knob = hw.knobs[6];
    rvco.amp_knob = hw.knobs[7];
    rvco.cv_in = hw.cvins[1];
    rvco.mode = &hw.switches[1];

    while(1) {
        hw.som.PrintLine(
            "Wave Type: left %s right %s",
            lvco.GetType().c_str(),
            rvco.GetType().c_str()
        );
        daisy::System::Delay(100);
    }
}