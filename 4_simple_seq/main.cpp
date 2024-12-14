#include "estuary.h"
#include "daisysp.h"

ccam::hw::Estuary hw;

daisysp::Metro clock;
uint8_t step_num = 0;
float step_val = 0.0f;
// HZ = (BPM / 60)
float clk_freq = 240.0f / 60.0f;

size_t ticks_since_high = 0;
float inverse_sample_rate = 0.0f;

float vco_freq = 220.0f;
daisysp::Oscillator vco;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    for (size_t i = 0; i < size; i++)
    {
        if (hw.som.gate_in_1.Trig()) {
            clk_freq = hw.som.AudioSampleRate() / static_cast<float>(ticks_since_high);
            clock.SetFreq(clk_freq);
            ticks_since_high = 0;
        } else {
            ticks_since_high++;
        }

        if (clock.Process()) {
            step_num = (step_num + 1) % hw.knobs.size();
            step_val = hw.knobs[step_num]->Value();
            hw.som.WriteCvOut(0, step_val);
        }
        
        vco_freq = daisysp::fmap(step_val, 44.0f, 440.0f);
        vco.SetFreq(vco_freq);
        float voice = vco.Process();
        OUT_L[i] = voice;
        OUT_R[i] = voice;
    }

    for (uint8_t i = 0; i < hw.leds.size(); i++) {
        hw.leds[i].Set(i == step_num ? 0.0f : 1.0f);
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);
    
    hw.som.StartLog(false);

    clock.Init(clk_freq, hw.som.AudioSampleRate());

    vco.Init(hw.som.AudioSampleRate());
    vco.SetAmp(1.0f);
    while(1) {
        hw.som.PrintLine("Step %d freq %f tempo %f", step_num+1, vco_freq, clk_freq);
        daisy::System::Delay(100);
    }
}