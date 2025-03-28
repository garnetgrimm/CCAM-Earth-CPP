#include <array>
#include <cstdint>
#include <math.h>

class Quantizer {
    template <typename T>
    static float apply(T& scale, float note) {
        float octave = floorf(note / 12.0f);
        note = fmodf(note, 12.0f);

        uint8_t min_distance = 12.0f;
        uint8_t min_idx = 0;
        for (uint8_t i = 0; i < scale.size(); i++) {
            uint8_t distance = abs(note - scale[i]);
            if (distance < min_distance) {
                min_idx =  i;
                min_distance = distance;
            }
        }
        return scale[min_idx] + octave*12.0f;
    }
public:
    enum class Scale {
        NONE,
        ALL,
        MAJOR,
        MINOR,
        OCTATONIC_EVEN,
        OCTATONIC_ODD,
        LYDIAN_DOM,
        PHRYGIAN_DOM,
        COUNT,
    };

    static float apply(Scale scale, float note);
};