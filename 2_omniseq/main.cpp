#include "daisysp.h"

#include "earth.h"

#include <tuple>
#include <functional>

json2daisy::DaisyEarth earth;

using LedState = std::array<float, 8>;
using NoteSeq = std::array<uint8_t, 8>;
using ChordGen = std::function<int(int)>;

const uint8_t MIDDLE_C = 60;

static uint8_t major_chord(uint8_t root) {
    std::array<uint8_t, 4> offsets = { 0, 4, 7, 12 };
    return root + offsets[daisy::Random::GetValue() % offsets.size()];
}

static uint8_t minor_chord(uint8_t root) {
    std::array<uint8_t, 4> offsets = { 0, 3, 7, 12 };
    return root + offsets[daisy::Random::GetValue() % offsets.size()];
}

static uint8_t dimin_chord(uint8_t root) {
    std::array<uint8_t, 4> offsets = { 0, 3, 6, 12 };
    return root + offsets[daisy::Random::GetValue() % offsets.size()];
}

static uint8_t major_7th_chord(uint8_t root) {
    std::array<uint8_t, 4> offsets = { 0, 4, 7, 11 };
    return root + offsets[daisy::Random::GetValue() % offsets.size()]; 
}

static uint8_t minor_7th_chord(uint8_t root) {
    std::array<uint8_t, 4> offsets = { 0, 3, 7, 10 };
    return root + offsets[daisy::Random::GetValue() % offsets.size()]; 
}

static uint8_t dimin_7th_chord(uint8_t root) {
    std::array<uint8_t, 4> offsets = { 0, 3, 6, 9 };
    return root + offsets[daisy::Random::GetValue() % offsets.size()];
}

static void get_rand_seq(ChordGen gen, NoteSeq& seq, uint8_t root) {
    for (uint8_t i = 0; i < seq.size(); i++) {
        seq[i] = gen(root);
    }
}

const std::array<LedState, 12> pianoLayout = {
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

const LedState nullState = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

const std::array<ChordGen, 6> chordOptions = {
    major_chord,
    minor_chord,
    dimin_chord,
    major_7th_chord,
    minor_7th_chord,
    dimin_7th_chord
};

std::array<NoteSeq, 8> patterns;

enum class Focus {
    None,
    KeyKnob,
    OctKnob,
    SclKnob,
    DegKnob,
};

static Focus masterFocus;

template <Focus focus>
struct StateDelta {
    static const size_t timeoutTicks = 1000;

    size_t ticksSinceMove = 0;
    uint8_t lastValue;

    void tick(uint8_t value) {
        if (value != lastValue) {
            ticksSinceMove = 0;
            lastValue = value;
            masterFocus = focus;
        }
                
        if (masterFocus != focus) {
            return;
        }

        ticksSinceMove++;

        if (ticksSinceMove == timeoutTicks) {
            masterFocus = Focus::None;
        }
    }
};

static StateDelta<Focus::KeyKnob> keyKnob;
static StateDelta<Focus::OctKnob> octKnob;
static StateDelta<Focus::SclKnob> sclKnob;
static StateDelta<Focus::DegKnob> degKnob;

static uint8_t updatePatternSelect() {
    static uint8_t activePattern = 0;

    for(uint8_t i = 0; i < earth.buttons.size(); i++) {
        if(earth.buttons[i]->Pressed()) {
            masterFocus = Focus::None; 
            activePattern = i;
        }
    }

    return activePattern;
}

static void patternToLedState(const NoteSeq& pattern, LedState& state) {
    uint8_t maxVal = 0;
    uint8_t minVal = -1;
    for(uint8_t i = 0; i < pattern.size(); i++) {
        maxVal = std::max(maxVal, pattern[i]);
        minVal = std::min(minVal, pattern[i]);
    }

    float amp = static_cast<float>(maxVal - minVal);
    for(uint8_t i = 0; i < pattern.size(); i++) {
        state[i] = static_cast<float>(pattern[i] - minVal) / amp;
    }
}

static void EarthCallback(daisy::AudioHandle::InputBuffer, 
            daisy::AudioHandle::OutputBuffer, 
            size_t) {
    earth.ProcessAllControls();

    uint8_t deg = daisysp::fmap(earth.knob4.Value(), 0.0f, 8.0f);
    degKnob.tick(deg);

    uint8_t scale = daisysp::fmap(earth.knob3.Value(), 0.0f, 2.0f);
    sclKnob.tick(scale);

    uint8_t oct = daisysp::fmap(earth.knob2.Value(), 0.0f, 4.0f);
    octKnob.tick(oct);

    uint8_t key = daisysp::fmap(earth.knob1.Value(), 0.0f, 12.0f);
    keyKnob.tick(key);

    uint8_t pidx = updatePatternSelect();

    //ChordGen chord = chordOptions[deg];

    LedState state = nullState;

    switch(masterFocus) {
        case Focus::KeyKnob:
            state = pianoLayout[key];
            break;
        case Focus::OctKnob:
            state[oct] = 1.0f;
            break;
        case Focus::SclKnob:
            state[scale] = 1.0f;
            break;
        case Focus::DegKnob:
            state[deg] = 1.0f;
            break;
        default:
            patternToLedState(patterns[pidx], state);
            break;
    }

    for (unsigned i = 0; i < earth.leds.size(); i++) {
        earth.leds[i]->Set(state[i]);
    }

    earth.PostProcess();
}

int main(void)
{
    earth.Init();
    earth.som.StartLog(false);

    using Chord = std::tuple<ChordGen, uint8_t>;
    std::array<Chord, 8> major_scale = {
        Chord{major_chord, 0},
        Chord{minor_chord, 2},
        Chord{minor_chord, 4},
        Chord{major_chord, 5},
        Chord{major_chord, 7},
        Chord{minor_chord, 9},
        Chord{dimin_chord, 11},
        Chord{major_chord, 12}
    };

    for(uint8_t i = 0; i < patterns.size(); i++) {
        auto [chord, root] = major_scale[i];
        get_rand_seq(chord, patterns[i], root);
    }

    earth.StartAudio(EarthCallback);

    while(1) {
        daisy::System::Delay(1000);
    }
}