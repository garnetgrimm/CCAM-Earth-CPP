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
  
static const uint8_t* drum_map[kGridSize][kGridSize] = {
  { node_10, node_8, node_0, node_9, node_11 },
  { node_15, node_7, node_13, node_12, node_6 },
  { node_18, node_14, node_4, node_5, node_3 },
  { node_23, node_16, node_21, node_1, node_2 },
  { node_24, node_19, node_17, node_20, node_22 },
};

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

void PatternGenerator::TickDrums() {
    // At the beginning of a pattern, decide on perturbation levels.
    if (step_ == 0) {
        for (uint8_t i = 0; i < kNumParts; ++i) {
            part_perturbation_[i] = daisy::Random::GetFloat(0.0f, settings_.randomness);
        }
    }
  
    uint8_t instrument_mask = 1;
    for (uint8_t i = 0; i < kNumParts; ++i) {
        float level = ReadDrumMap(step_, i, settings_.x, settings_.y);
        if (level < 1.0f - part_perturbation_[i]) {
            level += part_perturbation_[i];
        } else {
            // The sequencer from Anushri uses a weird clipping rule here. Comment
            // this line to reproduce its behavior.
            level = 1.0f;
        }
        if (level > 1.0f - settings_.density[i]) {
            state_ |= instrument_mask;
        }
        instrument_mask <<= 1;
    }
}

void PatternGenerator::TickEuclidean() {
    // Euclidean pattern generation
    uint8_t instrument_mask = 1;
    uint8_t reset_bits = 0;
    for (uint8_t i = 0; i < kNumParts; ++i) {
        uint8_t length = settings_.euclidean_length[i];
        size_t offset = 0;
        offset += length * kStepsPerPattern;
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

void PatternGenerator::Tick() {
    auto inc_and_wrap = [&](uint8_t& value, uint8_t maxval) {
        value = (value + 1) % maxval;
    };

    state_ = 0;
    if (settings_.output_mode == OUTPUT_MODE_EUCLIDEAN) {
        TickEuclidean();
        for (uint8_t i = 0; i < kNumParts; ++i) {
            inc_and_wrap(euclidean_step_[i], settings_.euclidean_length[i]);
        }
    } else {
        TickDrums();
        inc_and_wrap(step_, kStepsPerPattern);
    }
}

}  // namespace grids

