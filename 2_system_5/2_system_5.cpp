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
        OUT_L[i] = player.Tick(hw.som.gate_in_1.Trig());
    }

    hw.PostProcess();
}


int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);
    player.SetPlaybackSampleRate(hw.som.AudioSampleRate());

    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        daisy::System::Delay(100);
    }
}