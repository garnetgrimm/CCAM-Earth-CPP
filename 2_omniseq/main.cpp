#include "daisysp.h"

#include "earth.h"

json2daisy::DaisyEarth earth;

using LedState = std::array<float, 8>;

std::array<LedState, 12> pianoLayout = {
    LedState{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    LedState{0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    LedState{0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    LedState{0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    LedState{0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    LedState{0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    LedState{0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f},
    LedState{0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
    LedState{0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f},
    LedState{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
    LedState{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f},
    LedState{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}
};

static void EarthCallback(daisy::AudioHandle::InputBuffer, 
            daisy::AudioHandle::OutputBuffer, 
            size_t) {
    earth.ProcessAllControls();

    uint8_t note = daisysp::fmap(earth.knob1.Value(), 0.0f, 12.0f);

    LedState state = pianoLayout[note];

    for (unsigned i = 0; i < earth.leds.size(); i++) {
        earth.leds[i]->Set(state[i]);
    }

    earth.PostProcess();
}

int main(void)
{
    earth.Init();
    earth.StartAudio(EarthCallback);

    while(1) {
        daisy::System::Delay(1000);
    }
}