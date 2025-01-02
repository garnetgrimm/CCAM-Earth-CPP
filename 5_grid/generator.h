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

namespace grids {

constexpr uint8_t kGridSize = 5;
constexpr uint8_t kNumParts = 3;
constexpr uint8_t kStepsPerPattern = 32;

class DrumGenerator {
public:
    virtual void Reset() { step_ = 0; }
    virtual void Tick();
    virtual bool Triggered();
protected:
    uint8_t step_;
};

class PatternGenerator : public DrumGenerator {
public:
    float x = 0.0f;
    float y = 0.0f;
    float chaos = 0.0f;
    float fill = 0.0f;

    void Tick();

    void SetInstrument(uint8_t inst) {
        if (inst > 3) { 
            inst = 3;
        }
        instrument = inst;
    }

    bool Triggered();
private:
    float ReadDrumMap();
    float part_perturbation_;
    float level_;

    uint8_t instrument = 0;
};

class EuclidianGenerator : public DrumGenerator {
public:
    float fill = 0.0f;
    uint8_t length;

    void Tick();
    bool Triggered();
};

}  // namespace grids

#endif // GRIDS_PATTERN_GENERATOR_H_