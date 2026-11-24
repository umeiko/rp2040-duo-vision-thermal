#pragma once

#include <algorithm>
#include <cmath>
#include <stdint.h>
#include "thermal_frame.hpp"

namespace raster {
struct Rect { int x, y, width, height; };

// Center a source aspect ratio in a destination without stretching.
inline Rect fit(int width, int height, int max_width, int max_height) {
    const bool fit_width = width * max_height >= height * max_width;
    const int w = fit_width ? max_width : width * max_height / height;
    const int h = fit_width ? height * max_width / width : max_height;
    return {(max_width - w) / 2, (max_height - h) / 2, w, h};
}

struct Range { float low, high; };
inline Range temperatureRange(const ThermalFrame& frame) {
    Range range{INFINITY, -INFINITY};
    for (int i = 0; i < frame.width * frame.height; ++i) {
        const float value = frame.celsius[i];
        if (!std::isfinite(value)) continue;
        range.low = std::min(range.low, value);
        range.high = std::max(range.high, value);
    }
    if (!std::isfinite(range.low)) return {0, 1};
    return range;
}

constexpr uint16_t rgb565(int r, int g, int b) {
    return uint16_t(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

inline uint16_t heat(float temperature, Range range) {
    if (!std::isfinite(temperature)) return 0;
    const float span = std::max(1.0f, range.high - range.low);
    const int t = int(std::clamp((temperature - range.low) / span, 0.0f, 1.0f) * 255);
    const int r = std::min(255, t * 3);
    const int g = std::clamp(t * 3 - 255, 0, 255);
    const int b = std::clamp(t * 3 - 510, 0, 255);
    return rgb565(r, g, b);
}

inline int thermalIndex(const ThermalFrame& frame, int x, int y, const Rect& area) {
    const int sx = x * frame.width / area.width;
    const int sy = frame.height - 1 - y * frame.height / area.height;
    return sy * frame.width + sx;
}

inline uint16_t blend(uint16_t camera, uint16_t thermal, unsigned alpha) {
    const unsigned inverse = 255 - alpha;
    const unsigned r = ((camera >> 11) * alpha + (thermal >> 11) * inverse + 127) / 255;
    const unsigned g = ((camera >> 5 & 63) * alpha + (thermal >> 5 & 63) * inverse + 127) / 255;
    const unsigned b = ((camera & 31) * alpha + (thermal & 31) * inverse + 127) / 255;
    return uint16_t((r << 11) | (g << 5) | b);
}
}
