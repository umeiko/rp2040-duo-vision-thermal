#pragma once

#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include "camera.hpp"
#include "render.hpp"

namespace fusion {
struct Settings {
    unsigned alpha = 128;
    float x = 0, y = 0, zoom = 1, angle = 0;
};
inline Settings settings;

// Each valid command fits in 79 characters. Overlong input is discarded to EOL.
inline void serialCommands() {
    static char command[80];
    static size_t size = 0;
    static bool overflow = false;
    while (Serial.available()) {
        const char c = Serial.read();
        if (c == '\r') continue;
        if (c != '\n') {
            if (size + 1 < sizeof(command)) command[size++] = c;
            else overflow = true;
            continue;
        }
        command[size] = '\0';
        size = 0;
        if (overflow) { overflow = false; continue; }
        int alpha;
        float x, y, zoom, angle;
        char extra;
        if (std::sscanf(command, "alpha %d %c", &alpha, &extra) == 1 && alpha >= 0 && alpha <= 255) {
            settings.alpha = alpha;
        } else if (std::sscanf(command, "align %f %f %f %f %c", &x, &y, &zoom, &angle, &extra) == 4 &&
                   std::abs(x) <= 280 && std::abs(y) <= 240 && zoom >= 0.25f && zoom <= 4 && std::abs(angle) <= 180) {
            settings.x = x;
            settings.y = y;
            settings.zoom = zoom;
            settings.angle = angle;
        } else if (std::strcmp(command, "bootloader") == 0) {
            rp2040.rebootToBootloader();
        } else {
            Serial.println("alpha 0..255 | align X Y ZOOM DEGREES | bootloader");
            continue;
        }
        Serial.println("OK");
    }
}

inline void draw(const ThermalFrame& thermal, const camera::Frame& visible) {
    if (!visible.valid() || settings.alpha == 0) {
        render::thermal(thermal);
        return;
    }
    const auto area = raster::fit(thermal.width, thermal.height, display::width, render::image_height);
    const auto range = render::colorize(thermal);
    // The camera covers the thermal view. Crop centrally before applying registration.
    const float factor = std::min(float(visible.width()) / area.width, float(visible.height()) / area.height) /
                         settings.zoom;
    const float radians = settings.angle * 0.01745329252f;
    const float a = std::cos(radians) * factor, b = std::sin(radians) * factor;
    const float cx = (visible.width() - 1) * 0.5f, cy = (visible.height() - 1) * 0.5f;
    const float offset_x = (area.width - 1) * 0.5f + settings.x;
    const float offset_y = (area.height - 1) * 0.5f + settings.y;
    // Keep 12 fractional bits so RGB565 blending needs no floating-point math per output pixel.
    constexpr int unit = 4096;
    const int step_a = std::lround(a * unit), step_b = std::lround(b * unit);
    const int origin_x = std::lround((cx - a * offset_x - b * offset_y + 0.5f) * unit);
    const int origin_y = std::lround((cy + b * offset_x - a * offset_y + 0.5f) * unit);
    const int limit_x = visible.width() * unit, limit_y = visible.height() * unit;
    render::image(area, [&](int x, int y) {
        const uint16_t color = render::thermal_colors[raster::thermalIndex(thermal, x, y, area)];
        const int sx = origin_x + step_a * x + step_b * y;
        const int sy = origin_y - step_b * x + step_a * y;
        if (unsigned(sx) >= unsigned(limit_x) || unsigned(sy) >= unsigned(limit_y)) return color;
        return raster::blend(visible.pixel(sx / unit, sy / unit), color, settings.alpha);
    });
    render::temperatureStatus(range);
}
}
