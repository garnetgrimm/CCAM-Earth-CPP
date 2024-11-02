#include "estuary.h"
#include "daisysp.h"

json2daisy::DaisyEstuary hw;

static void AudioCallback(daisy::AudioHandle::InputBuffer in, 
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();
    hw.PostProcess();

    for(unsigned i = 0; i < hw.knobs.size(); i++) {
        hw.leds[i]->Set(hw.knobs[i]->Value());
        hw.leds[i]->Update();
    }
}

int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);

    while(1) {
        daisy::System::Delay(1000);
    }
}