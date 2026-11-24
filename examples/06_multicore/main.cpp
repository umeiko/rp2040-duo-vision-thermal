#include <Arduino.h>
#include <pico/mutex.h>
#include "fusion.hpp"
#include "mlx90640.hpp"

bool core1_separate_stack = true;
auto_init_mutex(thermal_mutex);
static Mlx90640 sensor;
static ThermalFrame captured, published, display_frame;
static bool thermal_ready;
static bool camera_ready;
static uint32_t sequence;

// Core 0 owns the display, camera, USB Serial, and I2C0.
void setup() {
    Serial.begin(115200);
    display::begin();
    display::message("Waiting for MLX90640 on core 1");
    camera_ready = camera::begin();
}

void loop() {
    fusion::serialCommands();
    static uint32_t displayed;
    mutex_enter_blocking(&thermal_mutex);
    const bool fresh = sequence != displayed;
    if (fresh) {
        display_frame = published;
        displayed = sequence;
    }
    mutex_exit(&thermal_mutex);
    if (!fresh) { delay(2); return; }
    camera::Frame visible(camera_ready && fusion::settings.alpha != 0);
    fusion::draw(display_frame, visible);
}

// Core 1 owns the thermal sensor and I2C1.
void setup1() { thermal_ready = sensor.begin(); }

void loop1() {
    if (thermal_ready && sensor.read(captured)) {
        mutex_enter_blocking(&thermal_mutex);
        published = captured;
        ++sequence;
        mutex_exit(&thermal_mutex);
    } else delay(50);
}
