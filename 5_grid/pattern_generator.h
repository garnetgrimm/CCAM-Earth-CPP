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

const uint8_t kNumParts = 3;
const uint8_t kPulsesPerStep = 3;  // 24 ppqn ; 8 steps per quarter note.
const uint8_t kStepsPerPattern = 32;
const uint8_t kPulseDuration = 8;  // 8 ticks of the main clock.

struct DrumsSettings {
  float x;
  float y;
  float randomness;
};

struct PatternGeneratorSettings {
  union Options {
    DrumsSettings drums;
    float euclidean_length[kNumParts];
  } options;
  float density[kNumParts];
};

enum OutputMode {
  OUTPUT_MODE_EUCLIDEAN,
  OUTPUT_MODE_DRUMS
};

enum ClockResolution {
  CLOCK_RESOLUTION_4_PPQN,
  CLOCK_RESOLUTION_8_PPQN,
  CLOCK_RESOLUTION_24_PPQN,
  CLOCK_RESOLUTION_LAST
};

enum OutputBits {
  OUTPUT_BIT_COMMON = 0x08,
  OUTPUT_BIT_CLOCK = 0x10,
  OUTPUT_BIT_RESET = 0x20
};

enum LedState {
    LED_BD,
    LED_SD,
    LED_HH    
};

struct Options {
  ClockResolution clock_resolution;
  OutputMode output_mode;
  bool output_clock;
  bool tap_tempo;
  bool gate_mode;
  bool swing;
};

class PatternGenerator {
 public:
  PatternGenerator() { }
  ~PatternGenerator() { }
  
  static inline void Init() {
    step_ = 0;
    memset(euclidean_step_, 0, sizeof(euclidean_step_));
  }
  
  static inline void Retrigger() {
    Evaluate(false);
  }
  
  static inline void TickClock() {
    Evaluate(true);
  }
  
  static inline PatternGeneratorSettings* mutable_settings() {
    return &settings_;
  }
  
  static inline uint8_t led_pattern() {
    uint8_t result = 0;
    if (state_ & 1) {
      result |= LED_BD;
    }
    if (state_ & 2) {
      result |= LED_SD;
    }
    if (state_ & 4) {
      result |= LED_HH;
    }
    return result;
  }
  
 private:
  static void Evaluate(bool do_tick);
  static void EvaluateEuclidean();
  static void EvaluateDrums();
  
  static float ReadDrumMap(
      uint8_t step,
      uint8_t instrument,
      float x,
      float y);

  static Options options_;
  
  static uint8_t pulse_;
  static uint8_t step_;
  static uint8_t euclidean_step_[kNumParts];
  
  static uint8_t state_;
  static float part_perturbation_[kNumParts];

  static uint8_t pulse_duration_counter_;
  
  static PatternGeneratorSettings settings_;
};

extern PatternGenerator pattern_generator;

}  // namespace grids

#endif // GRIDS_PATTERN_GENERATOR_H_
