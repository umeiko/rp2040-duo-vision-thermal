#include <Arduino.h>
#include "display.hpp"

void setup() {
    display::begin();
    auto& tft = display::tft;
    tft.fillRect(20, 45, 80, 120, ST77XX_RED);
    tft.fillRect(100, 45, 80, 120, ST77XX_GREEN);
    tft.fillRect(180, 45, 80, 120, ST77XX_BLUE);
    display::message("ST7789   280 x 240   RGB color bars");
}

void loop() { delay(100); }
