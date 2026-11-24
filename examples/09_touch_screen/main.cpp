#include <Arduino.h>
#include <Wire.h>
#include "board_pins.hpp"
#include "display.hpp"

static uint8_t touch_address;

static bool readTouch(uint8_t (&data)[5]) {
    if (!touch_address) return false;
    Wire.beginTransmission(touch_address);
    Wire.write(uint8_t(0x02));
    if (Wire.endTransmission(false) != 0 || Wire.requestFrom(touch_address, sizeof(data)) != sizeof(data)) return false;
    for (uint8_t& value : data) value = Wire.read();
    return (data[0] & 0x0f) != 0;
}

void setup() {
    Serial.begin(115200);
    display::begin();
    Wire.setSDA(pins::shared_sda);
    Wire.setSCL(pins::shared_scl);
    Wire.begin();
    Wire.setClock(100000);
    Wire.setTimeout(25, true);
    const uint8_t addresses[] = {0x15, 0x38}; // CST816 family and FT6x36.
    for (uint8_t address : addresses) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) { touch_address = address; break; }
    }
    display::message(touch_address ? "Touch to draw. Press KEY to clear." : "Touch controller not detected.");
    pinMode(pins::button, INPUT_PULLUP);
}

void loop() {
    static bool button_pressed;
    const bool pressed = digitalRead(pins::button) == LOW;
    if (pressed && !button_pressed) display::tft.fillScreen(ST77XX_BLACK);
    button_pressed = pressed;
    uint8_t sample[5];
    if (readTouch(sample)) {
        const int raw_x = ((sample[1] & 0x0f) << 8) | sample[2];
        const int raw_y = ((sample[3] & 0x0f) << 8) | sample[4];
        const int x = raw_y;
        const int y = display::height - 1 - raw_x;
        if (unsigned(x) < display::width && unsigned(y) < display::height) {
            display::tft.fillCircle(x, y, 2, ST77XX_CYAN);
        }
    }
    delay(12);
}
