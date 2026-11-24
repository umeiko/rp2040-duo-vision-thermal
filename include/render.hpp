#pragma once

#include "display.hpp"
#include "raster.hpp"

namespace render {
constexpr int image_height = 216; // Leave a small status line below the image.
inline uint16_t line[display::width];
inline uint16_t thermal_colors[ThermalFrame::width * 32];

inline raster::Range colorize(const ThermalFrame& frame) {
    const auto range = raster::temperatureRange(frame);
    for (int i = 0; i < frame.width * frame.height; ++i) {
        thermal_colors[i] = raster::heat(frame.celsius[i], range);
    }
    return range;
}

template<class Pixel>
void image(raster::Rect area, Pixel pixel) {
    display::tft.startWrite();
    display::tft.setAddrWindow(area.x, area.y, area.width, area.height);
    for (int y = 0; y < area.height; ++y) {
        for (int x = 0; x < area.width; ++x) line[x] = pixel(x, y);
        display::tft.writePixels(line, area.width, true, false);
    }
    display::tft.endWrite();
}

inline void temperatureStatus(raster::Range range) {
    display::tft.fillRect(0, image_height, display::width, 24, ST77XX_BLACK);
    display::tft.setTextSize(1);
    display::tft.setTextColor(ST77XX_WHITE);
    display::tft.setCursor(8, 224);
    display::tft.print("Low ");
    display::tft.print(range.low, 1);
    display::tft.print(" C    High ");
    display::tft.print(range.high, 1);
    display::tft.print(" C");
}

inline void thermal(const ThermalFrame& frame) {
    const auto area = raster::fit(frame.width, frame.height, display::width, image_height);
    const auto range = colorize(frame);
    image(area, [&](int x, int y) {
        return thermal_colors[raster::thermalIndex(frame, x, y, area)];
    });
    temperatureStatus(range);
}
}
