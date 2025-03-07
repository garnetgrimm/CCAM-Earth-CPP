#include <ccam/hw/estuary.h>
#include "tr606.h"
#include "samples/cpp/dummy.h"

ccam::hw::Estuary hw;
ccam::SamplePlayer player;
ccam::TR606Sampler tr606;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();
    
    for (size_t i = 0; i < size; i++)
    {
        OUT_L[i] = player.Tick(hw.som.gate_in_1.Trig());
    }

    hw.PostProcess();
}


int main(void)
{
    hw.Init();

    player.SetPlaybackSampleRate(hw.som.AudioSampleRate());
    player.SetSample(
        std::make_pair(&dummy_sample[0], 0xFFFF)
    );

    hw.StartAudio(AudioCallback);

    hw.som.StartLog(false);
    hw.som.PrintLine("Hello world");

    bool ledOn = false;
    while(1) {
        size_t index = player.current_idx;
        index %= player.sample_buffer.second;
        hw.som.PrintLine(
            "current_idx %d sample %d",
            index,
            dummy_sample[index]
        );
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        daisy::System::Delay(100);
    }
}