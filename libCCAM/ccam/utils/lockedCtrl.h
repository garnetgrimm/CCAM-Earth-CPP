#pragma once
#include <src/hid/ctrl.h>
#include <array>

class LockedAnalogControl {
    std::array<float, 3> values;
    daisy::AnalogControl* knob;
    daisy::Switch3* sw;
    uint8_t last_pos = -1;
    float last_val = 0.0f;
    bool awake = true;
public:
    float tolerance = 0.10f; 

    void Init(daisy::AnalogControl* knob, daisy::Switch3* sw) {
        this->knob = knob;
        this->sw = sw;
        for (size_t i = 0; i < values.size(); i++) {
            values[i] = knob->Value();
        }
    }

    void Process() {
        uint8_t curr_pos = sw->Read();
        float curr_val = knob->Value();
        if (curr_pos != last_pos) {
            last_val = curr_val;
            awake = false;
        }
        if (fabs(curr_val - last_val) > tolerance) {
            awake = true;
        }
        values[curr_pos] = awake ? curr_val : values[curr_pos];
        last_pos = curr_pos;
    }

    float Value(uint8_t switch_pos) {
        return values[switch_pos];
    }
};