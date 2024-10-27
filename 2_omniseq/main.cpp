#include "daisysp.h"

#include "earth.h"

#include <tuple>
#include <functional>

json2daisy::DaisyEarth earth;

using LedState = std::array<float, 8>;
using NoteSeq = std::array<float, 8>;
using ChordGen = std::function<int(int)>;

int get_rand_major(int root) {
    std::array<uint8_t, 4> offsets = { 0, 4, 7, 12 };
    return root + offsets[daisy::Random::GetValue() % offsets.size()];
}

int get_rand_minor(int root) {
    std::array<uint8_t, 4> offsets = { 0, 3, 7, 12 };
    return root + offsets[daisy::Random::GetValue() % offsets.size()];
}

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

LedState nullState = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

std::array<ChordGen, 2> chords = {
    get_rand_major,
    get_rand_minor
};

template <typename T>
struct StateDelta {
    size_t ticksSinceMove = 0;
    T lastValue;

    void tick(T value) {
        if (value != lastValue) {
            ticksSinceMove = 0;
        } else {
            ticksSinceMove++;
        }
        lastValue = value;
    }

    bool active() {
        return ticksSinceMove < 1000;
    }
};

static std::tuple<bool, uint8_t> updateNoteKnob() {
    static StateDelta<uint8_t> state;

    uint8_t note = daisysp::fmap(earth.knob1.Value(), 0.0f, 12.0f);
    state.tick(note);

    return std::make_tuple(state.active(), note);
}

static std::tuple<bool, uint8_t> updateChordKnob() {
    static StateDelta<uint8_t> state;

    uint8_t chord = daisysp::fmap(earth.knob2.Value(), 0.0f, 2.0f);
    state.tick(chord);

    return std::make_tuple(state.active(), chord);
}

static void EarthCallback(daisy::AudioHandle::InputBuffer, 
            daisy::AudioHandle::OutputBuffer, 
            size_t) {
    earth.ProcessAllControls();

    auto [noteKnobActive, note] = updateNoteKnob();
    auto [chordKnobActive, chordIdx] = updateChordKnob();

    ChordGen chord = chords[chordIdx];

    LedState state;
    if (noteKnobActive) {
        state = pianoLayout[note];
    } else if (chordKnobActive) {
        state = nullState;
        state[chordIdx] = 1.0f;
    } else {
        state = nullState;
    }

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