#include "estuary.h"
#include "simple_vcf.h"

ccam::hw::Estuary hw;

SimpleVcf lvcf;
SimpleVcf rvcf;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    for (size_t i = 0; i < size; i++)
    {
        OUT_L[i] = lvcf.Process(IN_L[i]);
        OUT_R[i] = rvcf.Process(IN_R[i]);
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    lvcf.Init(hw.som.AudioSampleRate());
    rvcf.Init(hw.som.AudioSampleRate());

    hw.som.StartLog(false);
    hw.StartAudio(AudioCallback);

    lvcf.freq_knob = hw.knobs[0];
    lvcf.res_knob = hw.knobs[1];
    lvcf.type_knob = hw.knobs[2];
    lvcf.amp_knob = hw.knobs[3];
    lvcf.cv_in = hw.cvins[0];
    lvcf.mode = &hw.switches[0];

    rvcf.freq_knob = hw.knobs[4];
    rvcf.res_knob = hw.knobs[5];
    rvcf.type_knob = hw.knobs[6];
    rvcf.amp_knob = hw.knobs[7];
    rvcf.cv_in = hw.cvins[1];
    rvcf.mode = &hw.switches[1];

    while(1) {
        hw.som.PrintLine(
            "Filter Type: left %s right %s",
            lvcf.GetType().c_str(),
            rvcf.GetType().c_str()
        );
        daisy::System::Delay(100);
    }

}