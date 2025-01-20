#include "estuary.h"
#include "daisysp.h"

#include "shaper.h"
#include <Effects/decimator.h>
#include <Effects/chorus.h>

#include <Utility/delayline.h>
#include <Effects/reverbsc.h>

ccam::hw::Estuary hw;

Shaper shape;
daisysp::Decimator decimate;
daisysp::ReverbSc verb;

float interpf(float v1, float v2, float amt) {
    return v1*(1.0f-amt) + v2*amt;
};

float process_shape(float in) {
    return decimate.Process(shape.Process(in));
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    shape.SetTresh(hw.knobs[0]->Value() + hw.cvins[0]->Value());
    shape.SetShape(hw.knobs[1]->Value() + hw.cvins[1]->Value());
    decimate.SetDownsampleFactor(hw.knobs[2]->Value() + hw.cvins[2]->Value());
    decimate.SetBitcrushFactor(hw.knobs[3]->Value() + hw.cvins[3]->Value());

    verb.SetFeedback(hw.knobs[4]->Value());
    verb.SetLpFreq(daisysp::fmap(hw.knobs[5]->Value(), 10.0f, 10000.0f));

    float wet = hw.knobs[6]->Value();

    for (size_t i = 0; i < size; i++)
    {
        float left_in, left_out;
        float right_in, right_out;
        
        static float null_in = 0.0f;
        static float null_out = 0.0f;

        switch(hw.switches[0].Read()) {
            case daisy::Switch3::POS_LEFT:
                left_in = process_shape(IN_L[i]);
                right_in = IN_R[i];
                break;
            case daisy::Switch3::POS_RIGHT:
                left_in = IN_L[i];
                right_in = process_shape(IN_R[i]);
                break;
            case daisy::Switch3::POS_CENTER:
                left_in = process_shape(IN_L[i]);
                right_in = process_shape(IN_R[i]);
                break;
        }

        switch(hw.switches[1].Read()) {
            case daisy::Switch3::POS_LEFT:
                verb.Process(left_in, null_in, &left_out, &null_out);
                OUT_L[i] = interpf(left_in, left_out, wet);
                OUT_R[i] = right_in;
                break;
            case daisy::Switch3::POS_RIGHT:
                verb.Process(null_in, right_in, &null_out, &right_out);
                OUT_L[i] = left_in;
                OUT_R[i] = interpf(right_in, right_out, wet);
                break;
            case daisy::Switch3::POS_CENTER:
                verb.Process(left_in, right_in, &left_out, &right_out);
                OUT_L[i] = interpf(left_in, left_out, wet);
                OUT_R[i] = interpf(right_in, right_out, wet);
                break;
        }

    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    decimate.Init();
    decimate.SetSmoothCrushing(true);
    verb.Init(hw.som.AudioSampleRate());
    hw.StartAudio(AudioCallback);

    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        daisy::System::Delay(250);
    }
}