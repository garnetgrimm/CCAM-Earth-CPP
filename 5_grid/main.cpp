#include "estuary.h"
#include "daisysp.h"
#include "generator.h"

ccam::hw::Estuary hw;

daisysp::Metro clock;
float clk_freq = 32.0;
size_t clk_ticks = 0;
size_t ticks_since_high = 0;

constexpr size_t NumGenChannels = 2;
constexpr size_t ClkInTimeout = 8;

grids::PatternGenerator patGens[NumGenChannels];
grids::EuclidianGenerator eucGens[NumGenChannels];

bool clocking = false;

daisysp::SyntheticBassDrum bass;
daisysp::LadderFilter bvcf;
daisysp::SyntheticSnareDrum snare;
daisysp::LadderFilter svcf;

bool IsEuclidian() {
    if (hw.switches[1].Read() == daisy::Switch3::POS_CENTER) {
        return false;
    }
    return true;
}

void SetChannelOut(uint8_t channel, grids::DrumGenerator& drum) {
    if (drum.Triggered()) {
        channel ? snare.Trig() : bass.Trig();
    }
    hw.som.gate_out_1.Write(drum.Triggered());
    hw.leds[channel].Set(drum.GetLevel());
    hw.som.WriteCvOut(channel, drum.GetLevel() * 5.0f);
}

void TickPatGen(bool trig) {
    for (size_t i = 0; i < NumGenChannels; i++) {
        if (trig) {
            patGens[i].Tick();
        }
        patGens[i].x = hw.knobs[0]->Value();
        patGens[i].y = hw.knobs[1]->Value();
        patGens[i].chaos = hw.knobs[2]->Value();
        patGens[i].fill = hw.knobs[i + 4]->Value();
    }
}

void TickEucGen(bool trig) {
    for (size_t i = 0; i < NumGenChannels; i++) {
        if (trig) {
            eucGens[i].Tick();
        }
        eucGens[i].SetLength(static_cast<uint8_t>(hw.knobs[i]->Value() * grids::kStepsPerPattern));
        eucGens[i].fill = hw.knobs[i + 4]->Value();
    }
}

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
            daisy::AudioHandle::OutputBuffer out, 
            size_t size) {

    hw.ProcessAllControls();

    bool trig = false;
    if (clock.Process()) {
        clocking = !clocking;
        trig = clocking;
        hw.leds[3].Set(clocking ? 1.0f : 0.0f);
    }

    if (hw.som.gate_in_1.Trig()) {
        clk_ticks = ticks_since_high;
        clk_freq = hw.som.AudioCallbackRate() / static_cast<float>(clk_ticks);
        clock.SetFreq(2.0f * clk_freq);
        ticks_since_high = 0;
    } else if (ticks_since_high > clk_ticks * ClkInTimeout) {
        clk_ticks = 0;
        clk_freq = daisysp::fmap(hw.knobs[3]->Value(), 0.1f, 100.0f);
        clock.SetFreq(2.0f * clk_freq);
    } else {
        ticks_since_high++;
    }

    bool euclidian = IsEuclidian();
    euclidian ? TickEucGen(trig) : TickPatGen(trig);
    if (clocking) {
        for (size_t i = 0; i < NumGenChannels; i++) {
            if (euclidian) {
                SetChannelOut(i, eucGens[i]);
            } else {
                SetChannelOut(i, patGens[i]);
            }
        }
    } else {
        hw.som.gate_out_1.Write(false);
        hw.som.gate_out_2.Write(false);
    }

    for (size_t i = 0; i < size; i++) {
        OUT_L[i] = bvcf.Process(bass.Process());
        OUT_R[i] = svcf.Process(snare.Process()) * 0.10;
    }

    hw.PostProcess();
}

int main(void)
{
    hw.Init();
    hw.som.StartLog(false);

    bass.Init(hw.som.AudioSampleRate());
    bvcf.Init(hw.som.AudioSampleRate());
    bvcf.SetFreq(200.0);
    bvcf.SetFilterMode(daisysp::LadderFilter::FilterMode::LP24);
    snare.Init(hw.som.AudioSampleRate());
    snare.SetDecay(0.1f);
    svcf.Init(hw.som.AudioSampleRate());
    svcf.SetFreq(1500.0);
    svcf.SetFilterMode(daisysp::LadderFilter::FilterMode::BP12);

    hw.StartAudio(AudioCallback);

    clock.Init(clk_freq, hw.som.AudioCallbackRate());

    patGens[0].SetInstrument(0); // bass drum
    patGens[1].SetInstrument(2); // high hat

    while(1) {
        daisy::System::Delay(1000);
    }
}