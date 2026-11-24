#pragma once

struct ThermalFrame {
    float celsius[32 * 32]{};
    int height = 0;
    static constexpr int width = 32;
};
