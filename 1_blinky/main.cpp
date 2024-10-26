#include "earth.h"

json2daisy::DaisyEarth earth;

int main(void)
{
    earth.Init();
    bool led_on = false;

    while(1) {
        for (unsigned i = 0; i < earth.leds.size(); i++) {
            earth.leds[i]->Set(led_on ? 1.0f : 0.0f);
            earth.leds[i]->Update();
        }
        led_on = !led_on;
        daisy::System::Delay(500);
    }
}