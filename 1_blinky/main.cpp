#include "estuary.h"
#include "daisysp.h"

ccam::hw::Estuary hw;

static void AudioCallback(daisy::AudioHandle::InputBuffer in, 
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    for(uint8_t i = 0; i < hw.knobs.size(); i++) {
        hw.leds[i].Set(hw.knobs[i]->Value());
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);

    hw.som.StartLog(false);

    while(1) {
        daisy::System::Delay(1000);
    }
}