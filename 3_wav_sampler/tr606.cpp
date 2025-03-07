#include <dev/sdram.h>

#include "samples/cpp/TR606Cymb01.h"
#include "samples/cpp/TR606Cymb02.h"
#include "samples/cpp/TR606Cymb03.h"
#include "samples/cpp/TR606Cymb04.h"
#include "samples/cpp/TR606Cymb05.h"
#include "samples/cpp/TR606Cymb_OD.h"
#include "samples/cpp/TR606Hat_C01.h"
#include "samples/cpp/TR606Hat_C02.h"
#include "samples/cpp/TR606Hat_C03.h"
#include "samples/cpp/TR606Hat_C04.h"
#include "samples/cpp/TR606Hat_C05.h"
#include "samples/cpp/TR606Hat_C_OD.h"
#include "samples/cpp/TR606Hat_O01.h"
#include "samples/cpp/TR606Hat_O02.h"
#include "samples/cpp/TR606Hat_O03.h"
#include "samples/cpp/TR606Hat_O04.h"
#include "samples/cpp/TR606Hat_O05.h"
#include "samples/cpp/TR606Hat_O_OD.h"
#include "samples/cpp/TR606Hat_P01.h"
#include "samples/cpp/TR606Hat_P02.h"
#include "samples/cpp/TR606Hat_P03.h"
#include "samples/cpp/TR606Hat_P04.h"
#include "samples/cpp/TR606Hat_P05.h"
#include "samples/cpp/TR606Hat_P_OD.h"
#include "samples/cpp/TR606Kick01.h"
#include "samples/cpp/TR606Kick02.h"
#include "samples/cpp/TR606Kick03.h"
#include "samples/cpp/TR606Kick04.h"
#include "samples/cpp/TR606Kick05.h"
#include "samples/cpp/TR606Kick_OD.h"
#include "samples/cpp/TR606Snare01.h"
#include "samples/cpp/TR606Snare02.h"
#include "samples/cpp/TR606Snare03.h"
#include "samples/cpp/TR606Snare04.h"
#include "samples/cpp/TR606Snare05.h"
#include "samples/cpp/TR606Snare_OD.h"
#include "samples/cpp/TR606TomHi01.h"
#include "samples/cpp/TR606TomHi02.h"
#include "samples/cpp/TR606TomHi03.h"
#include "samples/cpp/TR606TomHi04.h"
#include "samples/cpp/TR606TomHi05.h"
#include "samples/cpp/TR606TomHi_OD.h"
#include "samples/cpp/TR606TomLo01.h"
#include "samples/cpp/TR606TomLo02.h"
#include "samples/cpp/TR606TomLo03.h"
#include "samples/cpp/TR606TomLo04.h"
#include "samples/cpp/TR606TomLo05.h"
#include "samples/cpp/TR606TomLo_OD.h"

#include "tr606.h"

using namespace ccam;

SampleBuffer TR606Sampler::get_cymbol() {
    switch (sample_option) {
        case 0:
            return std::make_pair(
                TR606Cymb01_sample.data(),
                TR606Cymb01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606Cymb02_sample.data(),
                TR606Cymb02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606Cymb03_sample.data(),
                TR606Cymb03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606Cymb04_sample.data(),
                TR606Cymb04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606Cymb05_sample.data(),
                TR606Cymb05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606Cymb_OD_sample.data(),
                TR606Cymb_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }
}


SampleBuffer TR606Sampler::get_hat_closed() {
    switch (sample_option) {
        case 0:
            return std::make_pair(
                TR606Hat_C01_sample.data(),
                TR606Hat_C01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606Hat_C02_sample.data(),
                TR606Hat_C02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606Hat_C03_sample.data(),
                TR606Hat_C03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606Hat_C04_sample.data(),
                TR606Hat_C04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606Hat_C05_sample.data(),
                TR606Hat_C05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606Hat_C_OD_sample.data(),
                TR606Hat_C_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }
}


SampleBuffer TR606Sampler::get_hat_opened() {
    switch (sample_option) {
        case 0:
            return std::make_pair(
                TR606Hat_O01_sample.data(),
                TR606Hat_O01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606Hat_O02_sample.data(),
                TR606Hat_O02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606Hat_O03_sample.data(),
                TR606Hat_O03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606Hat_O04_sample.data(),
                TR606Hat_O04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606Hat_O05_sample.data(),
                TR606Hat_O05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606Hat_O_OD_sample.data(),
                TR606Hat_O_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }
}


SampleBuffer TR606Sampler::get_hat_pedal() {
    switch (sample_option) {
        case 0:
            return std::make_pair(
                TR606Hat_P01_sample.data(),
                TR606Hat_P01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606Hat_P02_sample.data(),
                TR606Hat_P02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606Hat_P03_sample.data(),
                TR606Hat_P03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606Hat_P04_sample.data(),
                TR606Hat_P04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606Hat_P05_sample.data(),
                TR606Hat_P05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606Hat_P_OD_sample.data(),
                TR606Hat_P_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }
}


SampleBuffer TR606Sampler::get_kick() {
    switch (sample_option) {
        case 0:
            return std::make_pair(
                TR606Kick01_sample.data(),
                TR606Kick01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606Kick02_sample.data(),
                TR606Kick02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606Kick03_sample.data(),
                TR606Kick03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606Kick04_sample.data(),
                TR606Kick04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606Kick05_sample.data(),
                TR606Kick05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606Kick_OD_sample.data(),
                TR606Kick_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }
}


SampleBuffer TR606Sampler::get_snare() {
    switch (sample_option) {
        case 0:
            return std::make_pair(
                TR606Snare01_sample.data(),
                TR606Snare01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606Snare02_sample.data(),
                TR606Snare02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606Snare03_sample.data(),
                TR606Snare03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606Snare04_sample.data(),
                TR606Snare04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606Snare05_sample.data(),
                TR606Snare05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606Snare_OD_sample.data(),
                TR606Snare_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }
}


SampleBuffer TR606Sampler::get_tom_high() {
    switch (sample_option) {
        case 0:
            return std::make_pair(
                TR606TomHi01_sample.data(),
                TR606TomHi01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606TomHi02_sample.data(),
                TR606TomHi02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606TomHi03_sample.data(),
                TR606TomHi03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606TomHi04_sample.data(),
                TR606TomHi04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606TomHi05_sample.data(),
                TR606TomHi05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606TomHi_OD_sample.data(),
                TR606TomHi_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }
}


SampleBuffer TR606Sampler::get_tom_low() {
    switch (sample_option) {
        case 0:
            return std::make_pair(
                TR606TomLo01_sample.data(),
                TR606TomLo01_sample.size()
            );
            break;
        case 1:
            return std::make_pair(
                TR606TomLo02_sample.data(),
                TR606TomLo02_sample.size()
            );
            break;
        case 2:
            return std::make_pair(
                TR606TomLo03_sample.data(),
                TR606TomLo03_sample.size()
            );
            break;
        case 3:
            return std::make_pair(
                TR606TomLo04_sample.data(),
                TR606TomLo04_sample.size()
            );
            break;
        case 4:
            return std::make_pair(
                TR606TomLo05_sample.data(),
                TR606TomLo05_sample.size()
            );
            break;
        case 5:
            return std::make_pair(
                TR606TomLo_OD_sample.data(),
                TR606TomLo_OD_sample.size()
            );
            break;
        default:
            return std::make_pair(nullptr, 0);
            break;
    }
}

SampleBuffer TR606Sampler::get_sample() {
    switch (sample_type) {
        case SampleType::CYMBOL:
            return get_cymbol();
        case SampleType::HAT_CLOSED:
            return get_hat_closed();
        case SampleType::HAT_OPENED:
            return get_hat_opened();
        case SampleType::HAT_PEDAL:
            return get_hat_pedal();
        case SampleType::KICK:
            return get_kick();
        case SampleType::SNARE:
            return get_snare();
        case SampleType::TOM_HIGH:
            return get_tom_high();
        case SampleType::TOM_LOW:
            return get_tom_low();
        default:
            return std::make_pair(nullptr, 0);
    }
};
