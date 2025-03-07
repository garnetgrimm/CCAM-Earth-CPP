#include <ccam/hw/estuary.h>
#include "samples/cpp/directory.h"

ccam::hw::Estuary hw;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    auto [cymbol_data, cymbol_len] = get_cymbol(0);
   
    for (size_t i = 0; i < size; i++)
    {
        OUT_L[i] = static_cast<float>(TR606Cymb01_sample[i]) / static_cast<float>(0xFFFF);
    }

    hw.PostProcess();
}


int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);

    while(1) {
        daisy::System::Delay(100);
    }
}