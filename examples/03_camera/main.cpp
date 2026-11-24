#include <Arduino.h>
#include "camera.hpp"
#include "render.hpp"

static bool ready;

void setup() {
    Serial.begin(115200);
    display::begin();
    ready = camera::begin();
    display::message(ready ? "PicoCamera RGB565" : "Camera not detected. Check FPC and I2C0.");
}

void loop() {
    camera::Frame frame(ready);
    if (!frame.valid()) { delay(50); return; }
    const auto area = raster::fit(frame.width(), frame.height(), display::width, render::image_height);
    render::image(area, [&](int x, int y) {
        return frame.pixel(x * frame.width() / area.width, y * frame.height() / area.height);
    });
}
