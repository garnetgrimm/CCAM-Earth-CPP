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
// Pattern generator.
//
// OUTPUT MODE  OUTPUT CLOCK  BIT7  BIT6  BIT5  BIT4  BIT3  BIT2  BIT1  BIT0
// DRUMS        FALSE          RND   CLK  HHAC  SDAC  BDAC    HH    SD    BD
// DRUMS        TRUE           RND   CLK   CLK   BAR   ACC    HH    SD    BD
// EUCLIDEAN    FALSE          RND   CLK  RST3  RST2  RST1  EUC3  EUC2  EUC1
// EUCLIDEAN    TRUE           RND   CLK   CLK  STEP   RST  EUC3  EUC2  EUC1

#ifndef GRIDS_PATTERN_GENERATOR_H_
#define GRIDS_PATTERN_GENERATOR_H_

#include <cstring>

namespace grids {

constexpr uint8_t kGridSize = 5;
constexpr uint8_t kNumParts = 3;
constexpr uint8_t kStepsPerPattern = 32;

enum OutputMode {
  OUTPUT_MODE_EUCLIDEAN,
  OUTPUT_MODE_DRUMS
};

struct Settings {
    float x;
    float y;
    float randomness;
    OutputMode output_mode;
    uint8_t euclidean_length[kNumParts];
    float density[kNumParts];
};

class PatternGenerator {
 public:
  PatternGenerator() { }
  ~PatternGenerator() { }
  
    void Reset() {
        step_ = 0;
        memset(euclidean_step_, 0, sizeof(euclidean_step_));
    }
  
 private:
    void Tick();
    void TickEuclidean();
    void TickDrums();
  
    float ReadDrumMap(uint8_t step, uint8_t instrument, float x, float y);

    uint8_t step_;
    uint8_t euclidean_step_[kNumParts];
  
    uint8_t state_;
    float part_perturbation_[kNumParts];

    static Settings settings_;
};

}  // namespace grids

#endif // GRIDS_PATTERN_GENERATOR_H_