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

class PatternGenerator {
public:
    float x = 0.0f;
    float y = 0.0f;
    float randomness = 0.0f;
    float density = 0.0f;

    // 0 to 3 inclusive
    uint8_t instrument = 0;

    void Reset() { step_ = 0; }
    bool Tick();

private:
    float ReadDrumMap();

    uint8_t step_;
    float part_perturbation_;
};

class EuclidianGenerator {
public:
    float density = 0.0f;
    uint8_t length;

    void Reset() { step_ = 0; }
    bool Tick();
private:
    uint8_t step_;
};

}  // namespace grids

#endif // GRIDS_PATTERN_GENERATOR_H_