// Copyright 2011 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Global clock.

#include "pattern_generator.h"
#include "resources.h"
#include <per/rng.h>

namespace grids {
  
/* static */
Options PatternGenerator::options_;

/* static */
uint8_t PatternGenerator::step_;

/* static */
uint8_t PatternGenerator::euclidean_step_[kNumParts];

/* static */
uint8_t PatternGenerator::state_;

/* static */
float PatternGenerator::part_perturbation_[kNumParts];

/* static */
PatternGeneratorSettings PatternGenerator::settings_;

/* extern */
PatternGenerator pattern_generator;

static const uint8_t* drum_map[kGridSize][kGridSize] = {
  { node_10, node_8, node_0, node_9, node_11 },
  { node_15, node_7, node_13, node_12, node_6 },
  { node_18, node_14, node_4, node_5, node_3 },
  { node_23, node_16, node_21, node_1, node_2 },
  { node_24, node_19, node_17, node_20, node_22 },
};

/* static */
float PatternGenerator::ReadDrumMap(
    uint8_t step,
    uint8_t instrument,
    float x,
    float y) {

    auto parse_data = [&](uint8_t value) {
        return static_cast<float>(value) / static_cast<float>(0xFF);
    };

    auto interp = [&](float v1, float v2, float amt) {
        return v1*amt + v2*(1.0f-amt);
    };
 
    uint8_t i = static_cast<uint8_t>(x * static_cast<float>(kGridSize));
    uint8_t j = static_cast<uint8_t>(y * static_cast<float>(kGridSize));
    const uint8_t* a_map = drum_map[i][j];
    const uint8_t* b_map = drum_map[i + 1][j];
    const uint8_t* c_map = drum_map[i][j + 1];
    const uint8_t* d_map = drum_map[i + 1][j + 1];
    uint8_t offset = (instrument * kStepsPerPattern) + step;
    offset = fminf(offset, kStepsPerPattern * kNumParts);
    float a = parse_data(a_map[offset]);
    float b = parse_data(b_map[offset]);
    float c = parse_data(c_map[offset]);
    float d = parse_data(d_map[offset]);

    return interp(interp(a, b, x), interp(c, d, x), y);
}

/* static */
void PatternGenerator::EvaluateDrums() {
    // At the beginning of a pattern, decide on perturbation levels.
    if (step_ == 0) {
        for (uint8_t i = 0; i < kNumParts; ++i) {
            float randomness = options_.swing ? 0.0f : settings_.options.drums.randomness;
            part_perturbation_[i] = daisy::Random::GetFloat(0.0f, randomness);
        }
    }
  
    uint8_t instrument_mask = 1;
    float x = settings_.options.drums.x;
    float y = settings_.options.drums.y;
    uint8_t accent_bits = 0;
    for (uint8_t i = 0; i < kNumParts; ++i) {
        float level = ReadDrumMap(step_, i, x, y);
        if (level < 1.0f - part_perturbation_[i]) {
            level += part_perturbation_[i];
        } else {
            // The sequencer from Anushri uses a weird clipping rule here. Comment
            // this line to reproduce its behavior.
            level = 1.0f;
        }
        if (level > 1.0f - settings_.density[i]) {
            if (level > 0.75) {
                accent_bits |= instrument_mask;
            }
            state_ |= instrument_mask;
        }
        instrument_mask <<= 1;
    }
    state_ |= accent_bits << 3;
}

/* static */
void PatternGenerator::EvaluateEuclidean() {
    // Euclidean pattern generation
    uint8_t instrument_mask = 1;
    uint8_t reset_bits = 0;
    for (uint8_t i = 0; i < kNumParts; ++i) {
        uint8_t length = (settings_.options.euclidean_length[i] * kStepsPerPattern) + 1;
        size_t offset = 0;
        offset += (length - 1) * kStepsPerPattern;
        offset += settings_.density[i] * kStepsPerPattern;
        while (euclidean_step_[i] >= length) {
            euclidean_step_[i] -= length;
        }
        uint32_t step_mask = 1L << static_cast<uint32_t>(euclidean_step_[i]);
        uint32_t pattern_bits = lut_res_euclidean[offset];
        if (pattern_bits & step_mask) {
            state_ |= instrument_mask;
        }
        if (euclidean_step_[i] == 0) {
            reset_bits |= instrument_mask;
        }
        instrument_mask <<= 1;
    }
  
    state_ |= reset_bits << 3;
}

/* static */
void PatternGenerator::Evaluate(bool do_tick) {
  state_ = 0;
  if (options_.output_mode == OUTPUT_MODE_EUCLIDEAN) {
    EvaluateEuclidean();
    if (do_tick) {
        for (uint8_t i = 0; i < kNumParts; ++i) {
            ++euclidean_step_[i];
        }
    }
  } else {
    EvaluateDrums();
    if (do_tick) {
        step_ = (step_ + 1) % kStepsPerPattern;
    }
  }
}

}  // namespace grids

