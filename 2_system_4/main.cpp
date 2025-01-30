#include "estuary.h"
#include "daisysp.h"

ccam::hw::Estuary hw;
daisysp::Metro metro;

class SimpleEnv {
    bool trig = false;
    float elapsed = 1.0f;
    float delta = 1.0f;
    float length = 0.5f;
    float amp = 1.0f;
    float slew = 0.95;

public:
    void Init(float sample_rate) {
        delta = 1.0f / sample_rate;
    }

    void SetLength(float length) {
        length = daisysp::fclamp(length, 0.0f, 1.0f);
        length = 1.0f - length;
        length = daisysp::fmap(length, 0.1f, 10.0f);
        this->length = length;
    }

    float Process(bool gate) {
        if (!gate) {
            trig = false;
        }
        
        if (gate && !trig) {
            elapsed = 0.0f;
            trig = true;
        }

        amp = (amp * slew) + ((1.0f - slew) * daisysp::pow10f(-1.0f * length * elapsed));
        elapsed += delta;
        return amp;
    }
};

class ToneDrum {
    daisysp::Oscillator voice;
    SimpleEnv fm_env;
    SimpleEnv amp_env;
    float fm;
    float amp;
    float freq;
    float freq_target;

public:
    void Init(float sample_rate) {
        voice.Init(sample_rate);
        fm_env.Init(sample_rate);
        amp_env.Init(sample_rate);
    }

    void SetFreq(float freq) {
        this->freq_target = freq;
    }

    void SetLength(float len) {
        amp_env.SetLength(len);
    }

    void SetFmAmount(float fm) {
        fm = daisysp::fclamp(fm, 0.0f, 1.0f);
        this->fm = fm * 5000.0f;
    }

    void SetFmLength(float len) {
        fm_env.SetLength(len);
    }

    void SetAmp(float amp) {
        this->amp = amp;
    }

    float Process(bool gate) {
        if (gate) {
            freq = freq_target;
        }
        voice.SetFreq(freq + (fm_env.Process(gate) * fm));
        return amp * voice.Process() * amp_env.Process(gate);
    }
};

class NoiseDrum {
    daisysp::WhiteNoise noise;
    daisysp::LadderFilter filter;
    SimpleEnv env;
    float amp;
    float freq;

public:
    void Init(float sample_rate) {
        noise.Init();
        filter.Init(sample_rate);
        filter.SetFilterMode(daisysp::LadderFilter::FilterMode::BP12);
        env.Init(sample_rate);
    }

    void SetLength(float len){
        env.SetLength(len);
    }

    void SetFreq(float freq) {
        this->freq = freq;
    }

    void SetAmp(float amp) {
        this->amp = amp;
    }

    float Process(bool gate) {
        if (gate) {
            filter.SetFreq(freq);
        }

        return amp * filter.Process(noise.Process()) * env.Process(gate);
    }
};

ToneDrum tone_drum;
NoiseDrum noise_drum;

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    float tone_freq = hw.knobs[0]->Value() + hw.cvins[0]->Value();
    tone_freq = daisysp::fmap(tone_freq, 5.0f, 880.0f);
    float tone_len = hw.knobs[1]->Value() + hw.cvins[1]->Value();

    float noise_freq = hw.knobs[4]->Value() + hw.cvins[2]->Value();
    noise_freq = daisysp::fmap(noise_freq, 5.0f, 4000.0f);
    float noise_len = hw.knobs[5]->Value() + hw.cvins[3]->Value();

    tone_drum.SetFreq(tone_freq);
    tone_drum.SetLength(tone_len);
    tone_drum.SetFmAmount(hw.knobs[2]->Value() * 0.1);
    tone_drum.SetFmLength(hw.knobs[3]->Value());

    noise_drum.SetFreq(noise_freq);
    noise_drum.SetLength(noise_len);

    tone_drum.SetAmp(hw.knobs[6]->Value());
    noise_drum.SetAmp(hw.knobs[7]->Value());

    bool tone_trig = hw.som.gate_in_1.Trig();
    bool noise_trig = hw.som.gate_in_2.Trig();

    for (size_t i = 0; i < size; i++)
    {
        
        OUT_L[i] = tone_drum.Process(tone_trig);
        OUT_R[i] = noise_drum.Process(noise_trig);
    }

    hw.PostProcess();
}


int main(void)
{
    hw.Init();

    hw.som.StartLog(false);

    tone_drum.Init(hw.som.AudioSampleRate());
    noise_drum.Init(hw.som.AudioSampleRate());
    
    hw.StartAudio(AudioCallback);

    bool ledOn = false;
    while(1) {
        ledOn = !ledOn;
        hw.leds[0].Set(ledOn ? 0.0f : 1.0f);
        daisy::System::Delay(1000);
    }
}