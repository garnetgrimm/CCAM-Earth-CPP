#include <array>

#include "earth.h"

#include "daisysp.h"
#include "Utility/dsp.h"
#include "Effects/reverbsc.h"

json2daisy::DaisyEarth earth;
daisysp::Oscillator vco;
daisysp::Oscillator lfo;
daisysp::ReverbSc verb;

static constexpr float MAX_U16_FLOAT = static_cast<float>(0xFFFF);

static void EarthCallback(daisy::AudioHandle::InputBuffer in, 
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    earth.ProcessAllControls();
    earth.PostProcess();

    float time_cv = daisysp::fmap(earth.knob4.Value(), 0.3f, 0.99f);
    float damp_cv = daisysp::fmap(earth.knob5.Value(), 1000.f, 19000.f, daisysp::Mapping::LOG);

    verb.SetFeedback(time_cv);
    verb.SetLpFreq(damp_cv);

    for (size_t i = 0; i < size; i++)
    {
        verb.Process(IN_L[i], IN_R[i], &OUT_L[i], &OUT_R[i]);

        float voice = vco.Process() * (earth.knob6.Value() - 0.05);
        OUT_L[i] += voice;
        OUT_R[i] += voice;
    }
}

static void CVOutCallback(uint16_t **out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        // the outputs are switched, do not know why.
        out[1][i] = static_cast<uint16_t>(earth.knob1.Value() * MAX_U16_FLOAT) >> 4;
        out[0][i] = static_cast<uint16_t>(earth.knob2.Value() * lfo.Process()) >> 4;
    }
}

int main(void)
{
    earth.Init();
    earth.StartAudio(EarthCallback);
    earth.StartCV(CVOutCallback);

    vco.Init(earth.som.AudioSampleRate());
    vco.SetFreq(440.0f);

    lfo.Init(earth.CvOutSampleRate());    
    lfo.SetFreq(220.0f);
    lfo.SetAmp(MAX_U16_FLOAT);

    verb.Init(earth.som.AudioSampleRate());

    earth.som.StartLog(false);
    earth.som.PrintLine("Hello world");

    while(1) {
        for (unsigned i = 0; i < earth.knobs.size(); i++) {
            earth.leds[i]->Set(earth.knobs[i]->Value() - 0.05);
        }
        earth.led7.Set(earth.cvin1.Value() - 0.05);
        earth.led8.Set(earth.cvin2.Value() - 0.05);

        for (unsigned i = 0; i < earth.leds.size(); i++) {
            if (earth.buttons[i]->Pressed()) {
                earth.leds[i]->Set(0.0f);
            }
        }

        daisy::System::Delay(1);
    }
}