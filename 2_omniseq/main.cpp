#include "daisysp.h"

#include "earth.h"

#include <tuple>
#include <functional>

json2daisy::DaisyEarth earth;

static constexpr float MAX_U16_FLOAT = static_cast<float>(0xFFFF);

using LedState = std::array<float, 8>;
using NoteSeq = std::array<float, 8>;

const std::array<std::string, 12> note_names = {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B"
};

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

struct Pattern {
    NoteSeq seq;
    uint8_t deg;
    uint8_t oct;
};

std::array<Pattern, 8> patterns;

enum class Focus {
    None,
    KeyKnob,
    SclKnob,
    TmpKnob,
    DegKnob,
    OctKnob
};

static Focus masterFocus;

template <Focus focus>
struct StateDelta {
    static const size_t timeoutSeconds = 2;

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

        if (ticksSinceMove == timeoutSeconds * earth.CvOutCallbackRate()) {
            masterFocus = Focus::None;
        }
    }
};

static StateDelta<Focus::KeyKnob> keyKnob;
static StateDelta<Focus::SclKnob> sclKnob;
static StateDelta<Focus::TmpKnob> tmpKnob;

static StateDelta<Focus::OctKnob> octKnob;
static StateDelta<Focus::DegKnob> degKnob;

void get_rand_seq(NoteSeq& seq) {
    for (uint8_t i = 0; i < seq.size(); i++) {
        seq[i] = daisy::Random::GetFloat();
    }
}

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

static uint8_t activeNote = 0;
static size_t activeNoteTicks = 0;

static void CVOutCallback(uint16_t **out, size_t size) {
    earth.ProcessAllControls();

    uint8_t deg = daisysp::fmap(earth.knob5.Value(), 0.0f, 8.0f);
    degKnob.tick(deg);

    uint8_t oct = daisysp::fmap(earth.knob4.Value(), 0.0f, 4.0f);
    octKnob.tick(oct);

    uint8_t tempo = daisysp::fmap(earth.knob3.Value(), 60.0f, 240.0f);
    tmpKnob.tick(tempo);

    uint8_t scale = daisysp::fmap(earth.knob2.Value(), 0.0f, 2.0f);
    sclKnob.tick(scale);

    uint8_t key = daisysp::fmap(earth.knob1.Value(), 0.0f, 12.0f);
    keyKnob.tick(key);

    uint8_t pidx = updatePatternSelect();

    size_t nextNoteTicks = (earth.CvOutCallbackRate()*60)/tempo;
    activeNoteTicks++;
    if (activeNoteTicks >= nextNoteTicks) {
        activeNoteTicks = 0;
        activeNote++;
        activeNote %= 8;
    }

    LedState state = nullState;

    switch(masterFocus) {
        case Focus::KeyKnob:
            state = pianoLayout[key];
            break;
        case Focus::SclKnob:
            state[scale] = 1.0f;
            break;
        case Focus::TmpKnob: {
            float tmpPerc = static_cast<float>(tempo - 60) / static_cast<float>(240 - 60 - 10);
            int maxLed = std::min((int)(tmpPerc * 8.0f), 8);
            for (int i = 0; i < maxLed; i++) {
                state[i] = 1.0f;
            }
            break;
        } case Focus::OctKnob:
            state[oct] = 1.0f;
            break;
        case Focus::DegKnob:
            state[deg] = 1.0f;
            break;
        default:
            state = patterns[pidx].seq;
            state[activeNote] = 1.0f;
            break;
    }

    for (unsigned i = 0; i < earth.leds.size(); i++) {
        earth.leds[i]->Set(state[i]);
    }


    for(size_t i = 0; i < size; i++)
    {
        out[1][i] = static_cast<uint16_t>(earth.knob1.Value() * MAX_U16_FLOAT) >> 4;
    }

    earth.PostProcess();
}

int main(void)
{
    earth.Init();
    earth.som.StartLog(false);

    for(uint8_t i = 0; i < patterns.size(); i++) {
        get_rand_seq(patterns[i].seq);
    }

    earth.StartCV(CVOutCallback);
    earth.SetAudioSampleRate(earth.CvOutSampleRate());

    while(1) {
        daisy::System::Delay(1000);
    }
}