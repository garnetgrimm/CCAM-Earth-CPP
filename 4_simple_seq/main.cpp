#include "estuary.h"
#include "daisysp.h"

ccam::hw::Estuary hw;

daisysp::Metro clock;
uint8_t step_num = 0;
// HZ = (BPM / 60)
float clk_freq = 240.0f / 60.0f;

size_t ticks_since_high = 0;
float inverse_sample_rate = 0.0f;

std::array<daisysp::Oscillator, 2> vcos;

bool clocking = false;

uint8_t SeqLen() {
    if (hw.switches[0].Read() == daisy::Switch3::POS_CENTER) {
        return 4;
    }
    return 8;
}

void StepSeq() {
    static uint8_t seq_step = 1;
    uint8_t seq_len = SeqLen();

    switch(hw.switches[1].Read() ) {
        case daisy::Switch3::POS_LEFT:
            seq_step = 1;
            break;
        case daisy::Switch3::POS_CENTER:
            if (step_num+1 == seq_len) {
                seq_step = -1;
            } else if (step_num == 0) {
                seq_step = 1;
            }
            break;
        case daisy::Switch3::POS_RIGHT:
            seq_step = -1;
            break;
        default:
            break;
    }

    step_num = (step_num + seq_step) % seq_len;
}

void Process() {
    StepSeq();

    auto write_out = [&](uint8_t channel, uint8_t step) {
        float value = hw.knobs[step]->Value();
        hw.som.WriteCvOut(channel, value*5.0f);
        vcos[channel].SetFreq(daisysp::fmap(value, 44.0f, 440.0f));
    };

    write_out(0, step_num);
    write_out(1, SeqLen() == 4 ? (7 - step_num) : step_num);
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {
    hw.ProcessAllControls();

    for (size_t i = 0; i < size; i++)
    {
        if (hw.som.gate_in_1.Trig()) {
            clk_freq = hw.som.AudioSampleRate() / static_cast<float>(ticks_since_high);
            clock.SetFreq(clk_freq * 2.0f);
            ticks_since_high = 0;
        } else {
            ticks_since_high++;
        }

        if (clock.Process()) {
            if (clocking) {
                Process();
            }
            hw.som.gate_out_1.Write(clocking);
            clocking = !clocking;
        }

        OUT_L[i] = vcos[0].Process();
        OUT_R[i] = vcos[1].Process();
    }

    for (uint8_t i = 0; i < hw.leds.size(); i++) {
        bool enabled = (i == step_num);
        if (SeqLen() == 4) {
            enabled |= (i == (7 - step_num));
        }
        
        hw.leds[i].Set(enabled ? 0.0f : 1.0f);
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    hw.StartAudio(AudioCallback);
    
    hw.som.StartLog(false);

    clock.Init(clk_freq, hw.som.AudioSampleRate());

    for (auto& vco : vcos) {
        vco.Init(hw.som.AudioSampleRate());
        vco.SetAmp(1.0f);
    }

    while(1) {
        hw.som.PrintLine("Step %d tempo %f", step_num+1, clk_freq);
        daisy::System::Delay(100);
    }
}