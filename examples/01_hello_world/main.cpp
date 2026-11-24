#include <Arduino.h>
#include "board_pins.hpp"

void setup() {
    Serial.begin(115200);
    pinMode(pins::button, INPUT_PULLUP);
    analogReadResolution(12);
}

void loop() {
    const int adc = analogRead(pins::battery);
    const float battery = adc * (3.3f / 4095.0f) * (98.0f / 68.0f);
    Serial.print("RP2040 ready | KEY=");
    Serial.print(digitalRead(pins::button) == LOW ? "pressed" : "released");
    Serial.print(" | VBAT=");
    Serial.print(battery, 2);
    Serial.println(" V");
    delay(500);
}
