#include <ccam/hw/estuary.h>
#include "tr606.h"

ccam::hw::Estuary hw;
ccam::SamplePlayer player;
ccam::TR606Sampler tr606;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();
    
    player.SetSample(tr606.get_sample());

    for (size_t i = 0; i < size; i++)
    {
        //OUT_L[i] = static_cast<float>(TR606Cymb01_sample[i]) / static_cast<float>(0xFFFF);
    }

    hw.PostProcess();
}


int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);
    player.SetPlaybackSampleRate(hw.som.AudioSampleRate());

    while(1) {
        daisy::System::Delay(100);
    }
}