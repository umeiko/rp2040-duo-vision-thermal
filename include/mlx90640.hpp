#pragma once

#include <Arduino.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "board_pins.hpp"
#include "thermal_frame.hpp"

extern "C" {
#include <MLX90640_I2C_Driver.h>
}

class Mlx90640 {
public:
    bool begin() {
        i2c_init(i2c1, 400000);
        gpio_set_function(pins::thermal_sda, GPIO_FUNC_I2C);
        gpio_set_function(pins::thermal_scl, GPIO_FUNC_I2C);
        gpio_pull_up(pins::thermal_sda);
        gpio_pull_up(pins::thermal_scl);
        // EEPROM calibration and raw frames never need the same buffer concurrently.
        return MLX90640_DumpEE(address, raw_) == 0 &&
               MLX90640_ExtractParameters(raw_, &calibration_) == 0 &&
               MLX90640_SetChessMode(address) == 0 &&
               MLX90640_SetRefreshRate(address, 4) == 0; // Register value 4 is 8 Hz.
    }

    bool read(ThermalFrame& frame) {
        unsigned seen = 0;
        for (int attempt = 0; attempt < 4 && seen != 3; ++attempt) {
            if (!waitForSubpage()) return false;
            const int subpage = MLX90640_GetFrameData(address, raw_);
            if (subpage < 0 || subpage > 1) return false;
            const float reflected = MLX90640_GetTa(raw_, &calibration_) - 8.0f;
            MLX90640_CalculateTo(raw_, &calibration_, 0.95f, reflected, frame.celsius);
            seen |= 1u << subpage;
        }
        if (seen != 3) return false;
        MLX90640_BadPixelsCorrection(calibration_.brokenPixels, frame.celsius, 1, &calibration_);
        MLX90640_BadPixelsCorrection(calibration_.outlierPixels, frame.celsius, 1, &calibration_);
        frame.height = 24;
        return true;
    }

private:
    bool waitForSubpage() {
        const uint32_t start = millis();
        do {
            uint16_t status;
            if (MLX90640_I2CRead(address, MLX90640_STATUS_REG, 1, &status) != 0) return false;
            if (MLX90640_GET_DATA_READY(status)) return true;
            delay(2);
        } while (millis() - start < 400);
        return false;
    }

    static constexpr uint8_t address = 0x33;
    paramsMLX90640 calibration_{};
    uint16_t raw_[834]{};
};
