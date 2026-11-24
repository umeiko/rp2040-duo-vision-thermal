#pragma once

#include "fusion.hpp"

template<class Sensor>
class FusionLesson {
public:
    void begin() {
        Serial.begin(115200);
        display::begin();
        sensor_ready_ = sensor_.begin();
        if (sensor_ready_) camera_ready_ = camera::begin();
        else display::message("Thermal probe not detected. Check I2C1.");
    }

    void step() {
        fusion::serialCommands();
        if (!sensor_ready_) { delay(50); return; }
        if (!sensor_.read(thermal_)) {
            display::message("Thermal frame missed");
            delay(25);
            return;
        }
        camera::Frame visible(camera_ready_ && fusion::settings.alpha != 0);
        fusion::draw(thermal_, visible);
    }
private:
    Sensor sensor_;
    ThermalFrame thermal_;
    bool sensor_ready_ = false;
    bool camera_ready_ = false;
};
