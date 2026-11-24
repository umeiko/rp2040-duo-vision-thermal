#pragma once

#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "board_pins.hpp"

namespace display {
constexpr int width = 280;
constexpr int height = 240;

inline Adafruit_ST7789 tft(&SPI, pins::screen_cs, pins::screen_dc, pins::screen_reset);

inline void begin() {
    pinMode(pins::screen_backlight, OUTPUT);
    digitalWrite(pins::screen_backlight, LOW);
    SPI.setRX(NOPIN); // The LCD is write-only. GPIO16 is DVP data.
    SPI.setSCK(pins::screen_sck);
    SPI.setTX(pins::screen_mosi);
    tft.init(240, 280);
    tft.setSPISpeed(32000000);
    tft.setRotation(3);
    // ST7789 rotated landscape, BGR subpixel order on this panel.
    uint8_t madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | 0x08;
    tft.sendCommand(ST77XX_MADCTL, &madctl, 1);
    tft.invertDisplay(true);
    tft.fillScreen(ST77XX_BLACK);
    digitalWrite(pins::screen_backlight, HIGH);
}

inline void message(const char* line) {
    tft.fillRect(0, 216, width, 24, ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setCursor(8, 224);
    tft.print(line);
}
}
