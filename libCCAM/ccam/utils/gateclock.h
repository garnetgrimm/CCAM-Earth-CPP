#pragma once

#include "daisysp.h"

template <typename Ctrl>
class GateClock {
    size_t last_tick = 0;
    float sample_rate;
    bool changed = false;
    bool is_high = false;
    bool input_active = false;

    daisy::GateIn* clk;
    Ctrl* knob;
    float progress = 0.0f;
    float step = 0.0f;
    float base_freq = 0.0f;
public:
    float timeout = 10.0f;
    size_t delta_tick = 0;

    void Init(daisy::GateIn* clk, Ctrl* knob, float sample_rate) {
        this->clk = clk;
        this->knob = knob;
        this->sample_rate = sample_rate;
    }

    void SetFreq(float freq) {
        step = freq / sample_rate;
    }

    float GetFreq() {
        return step * sample_rate;
    }

    bool RisingEdge() {
        return changed && is_high;
    }

    bool FallingEdge() {
        return changed && !is_high;
    }

    bool IsHigh() {
        return is_high;
    }

    bool InputActive() {
        return input_active;
    }

    bool GetProgress() {
        return progress;
    }

    void Process() {
        progress += step;

        size_t curr_tick = daisy::System::GetTick();
        delta_tick = curr_tick - last_tick;
        float sys_freq = static_cast<float>(daisy::System::GetTickFreq());

        if (clk->Trig()) {
            progress = 0.0f;
            base_freq = sys_freq / delta_tick;
            last_tick = curr_tick;
            input_active = true;
        }
        
        if ((delta_tick / sys_freq) < timeout) {
            input_active = false;
        }

        if (!input_active) {
            this->SetFreq(daisysp::fmap(knob->Value(), 1.0f, 256.0f));
        } else {
            uint8_t multiplier = daisysp::fmap(knob->Value(), 0.0f, 5.0f);
            float power = powf(2.0f, static_cast<float>(multiplier));
            this->SetFreq(base_freq * power);
        }

        if (progress > 1.0f) {
            progress = 0.0f;
        }
        bool is_high = progress < 0.5f; 

        if (is_high != this->is_high) {
            this->is_high = is_high;
            changed = true;
        } else {
            changed = false;
        }
    }
};