#pragma once

#include <Arduino.h>
#include "render.hpp"

// Sensors expose the same small interface so the renderer does not know the bus.
template<class Sensor>
class ThermalLesson {
public:
    void begin() {
        Serial.begin(115200);
        display::begin();
        ready_ = sensor_.begin();
        if (!ready_) display::message("Thermal probe not detected. Check I2C1.");
    }

    void step() {
        if (!ready_) { delay(50); return; }
        if (!sensor_.read(frame_)) {
            display::message("Thermal frame missed");
            delay(50);
            return;
        }
        render::thermal(frame_);
    }
private:
    Sensor sensor_;
    ThermalFrame frame_;
    bool ready_ = false;
};
