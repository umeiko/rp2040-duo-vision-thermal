#pragma once

#include <Arduino.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <hardware/i2c.h>
#include "board_pins.hpp"
#include "heimann_lut.hpp"
#include "thermal_frame.hpp"

// HTPA32x32d with the 5.0 lookup table. The factory EEPROM is only read.
class Heimann32x32 {
    static constexpr uint8_t sensor_address = 0x1a;
    static constexpr uint8_t eeprom_address = 0x50;
    static constexpr int side = 32, block_rows = 4, blocks = 4;
    static constexpr size_t packet_size = 2 + 2 * block_rows * side;
    static constexpr unsigned offset_interval = 10;
    enum class Conversion : uint8_t { ptat = 0, supply = 4, blind = 2 };

public:
    bool begin() {
        ready_ = false;
        i2c_init(i2c1, 400000);
        gpio_set_function(pins::thermal_sda, GPIO_FUNC_I2C);
        gpio_set_function(pins::thermal_scl, GPIO_FUNC_I2C);
        gpio_pull_up(pins::thermal_sda);
        gpio_pull_up(pins::thermal_scl);
        if (!loadCalibration()) return false;

        i2c_set_baudrate(i2c1, 1000000);
        if (!writeRegister(0x01, 0x01)) return false;
        const uint8_t values[] = {trim_[0], trim_[1], trim_[1], trim_[2], trim_[3], trim_[3], trim_[4]};
        for (unsigned i = 0; i < sizeof(values); ++i) {
            if (!writeRegister(0x03 + i, values[i])) return false;
            delay(5);
        }
        resetCapture();
        ready_ = true;
        return true;
    }

    bool read(ThermalFrame& frame) {
        if (!ready_) return false;
        if (!until_offsets_) {
            if (!captureBlock(Conversion::blind, 0)) return discard();
            until_offsets_ = offset_interval;
        }
        // Prime both auxiliary measurements before the first publication.
        if (!aux_valid_ && !captureImage(Conversion::ptat)) return discard();
        const Conversion mode = next_supply_ ? Conversion::supply : Conversion::ptat;
        if (!captureImage(mode)) return discard();
        next_supply_ = !next_supply_;
        aux_valid_ = true;
        --until_offsets_;
        calculate(frame);
        return true;
    }

private:
    bool ready_ = false, aux_valid_ = false, next_supply_ = true;
    unsigned until_offsets_ = 0;
    uint8_t trim_[5]{}, bad_count_ = 0, bad_mask_[5]{};
    uint16_t bad_address_[5]{}, vdd1_ = 0, ptat1_ = 0, ptat_ = 0, supply_ = 0;
    int8_t global_offset_ = 0;
    float thermal_scale_ = 0, vdd_gradient_scale_ = 0, vdd_offset_scale_ = 0;
    float ptat_gradient_ = 0, ptat_offset_ = 0, supply_slope_ = 0;
    int16_t thermal_gradient_[side][side]{}, thermal_offset_[side][side]{};
    int16_t vdd_gradient_[8][side]{}, vdd_offset_[8][side]{};
    float sensitivity_scale_[side][side]{};
    uint16_t raw_[side][side]{}, electrical_offset_[8][side]{};
    uint8_t packet_[packet_size]{};

    void resetCapture() {
        aux_valid_ = false;
        next_supply_ = true;
        until_offsets_ = 0;
    }

    bool discard() {
        resetCapture();
        return false;
    }

    static uint16_t little16(const uint8_t* p) { return p[0] | (uint16_t(p[1]) << 8); }
    static uint16_t big16(const uint8_t* p) { return (uint16_t(p[0]) << 8) | p[1]; }
    static float littleFloat(const uint8_t* p) {
        const uint32_t word = uint32_t(little16(p)) | (uint32_t(little16(p + 2)) << 16);
        float value;
        static_assert(sizeof(value) == sizeof(word));
        std::memcpy(&value, &word, sizeof(value));
        return value;
    }

    static bool readBytes(uint8_t address, const uint8_t* reg, size_t reg_len, uint8_t* dst, size_t len) {
        return i2c_write_timeout_us(i2c1, address, reg, reg_len, true, 20000) == int(reg_len) &&
               i2c_read_timeout_us(i2c1, address, dst, len, false, 20000) == int(len);
    }

    static bool readEeprom(uint16_t address, uint8_t* dst, size_t len) {
        const uint8_t reg[] = {uint8_t(address >> 8), uint8_t(address)};
        return readBytes(eeprom_address, reg, sizeof(reg), dst, len);
    }

    static bool readRegister(uint8_t reg, uint8_t* dst, size_t len) {
        return readBytes(sensor_address, &reg, 1, dst, len);
    }

    static bool writeRegister(uint8_t reg, uint8_t value) {
        const uint8_t bytes[] = {reg, value};
        return i2c_write_timeout_us(i2c1, sensor_address, bytes, sizeof(bytes), false, 20000) ==
               int(sizeof(bytes));
    }

    template<class T, size_t Rows>
    static bool loadRows(uint16_t address, T (&out)[Rows][side]) {
        uint8_t bytes[side * 2];
        for (unsigned row = 0; row < Rows; ++row) {
            if (!readEeprom(address + row * sizeof(bytes), bytes, sizeof(bytes))) return false;
            // The lower half is stored from the bottom row towards the center.
            const unsigned target = row < Rows / 2 ? row : Rows + Rows / 2 - 1 - row;
            for (int x = 0; x < side; ++x) {
                const uint16_t word = little16(bytes + x * 2);
                if constexpr (sizeof(T) == 2) out[target][x] = static_cast<int16_t>(word);
                else out[target][x] = word;
            }
        }
        return true;
    }

    bool loadCalibration() {
        uint8_t header[128];
        if (!readEeprom(0, header, sizeof(header))) return false;
        const uint32_t id = little16(header + 0x74) | (uint32_t(little16(header + 0x76)) << 16);
        const uint16_t ptat2 = little16(header + 0x3e);
        ptat1_ = little16(header + 0x3c);
        vdd1_ = little16(header + 0x26);
        const uint16_t vdd2 = little16(header + 0x28);
        const uint16_t table = little16(header + 0x0b);
        if (table != heimann_lut::table_number) {
            Serial.print("Unsupported Heimann EEPROM table: ");
            Serial.println(table);
            return false;
        }
        bad_count_ = header[0x7f];
        if (!id || id == UINT32_MAX || ptat1_ == ptat2 || bad_count_ > 5 ||
            header[0x08] > 31 || header[0x4e] > 31 || header[0x4f] > 31) return false;
        std::memcpy(trim_, header + 0x1a, sizeof(trim_));
        ptat_gradient_ = littleFloat(header + 0x34);
        ptat_offset_ = littleFloat(header + 0x38);
        const float pix_min = littleFloat(header), pix_max = littleFloat(header + 4);
        const float gain = (header[0x0d] / 100.0f) * (little16(header + 0x55) / 10000.0f);
        if (!std::isfinite(ptat_gradient_) || !std::isfinite(ptat_offset_) ||
            !std::isfinite(pix_min) || !std::isfinite(pix_max) ||
            pix_min <= 0 || pix_max < pix_min || gain <= 0 || header[0x0d] > 100) return false;
        thermal_scale_ = std::ldexp(1.0f, -header[0x08]);
        vdd_gradient_scale_ = std::ldexp(1.0f, -header[0x4e]);
        vdd_offset_scale_ = std::ldexp(1.0f, -header[0x4f]);
        supply_slope_ = (int(vdd2) - vdd1_) / float(int(ptat2) - ptat1_);
        global_offset_ = static_cast<int8_t>(header[0x54]);
        if (!loadRows(0x0340, vdd_gradient_) || !loadRows(0x0540, vdd_offset_) ||
            !loadRows(0x0740, thermal_gradient_) || !loadRows(0x0f40, thermal_offset_) ||
            !loadRows(0x1740, sensitivity_scale_)) return false;
        for (auto& row : sensitivity_scale_) for (float& word : row) {
            const float coefficient = (pix_min + (pix_max - pix_min) * (word / 65535.0f)) * gain;
            if (!std::isfinite(coefficient) || coefficient < 1.0f) return false;
            word = 100000000.0f / coefficient;
        }
        if (bad_count_) {
            uint8_t addresses[2 * sizeof(bad_mask_)];
            if (!readEeprom(0x0080, addresses, bad_count_ * 2) ||
                !readEeprom(0x00b0, bad_mask_, bad_count_)) return false;
            for (unsigned i = 0; i < bad_count_; ++i) {
                const uint16_t address = little16(addresses + 2 * i);
                if (address >= side * side) return false;
                const int row = address / side;
                bad_address_[i] = (row < side / 2 ? row : 47 - row) * side + address % side;
            }
        }
        Serial.print("Heimann EEPROM table: ");
        Serial.println(table);
        return true;
    }

    uint32_t auxiliary_sum_ = 0;

    bool captureBlock(Conversion mode, int block) {
        if (!writeRegister(0x01, 0x09 | uint8_t(mode) | uint8_t(block << 4))) return false;
        const uint32_t start = millis();
        for (;;) {
            uint8_t status;
            if (!readRegister(0x02, &status, 1)) return false;
            if (status & 1) break;
            if (millis() - start >= 100) return false;
            delay(1);
        }
        for (int half = 0; half < 2; ++half) {
            if (!readRegister(0x0a + half, packet_, sizeof(packet_))) return false;
            auxiliary_sum_ += big16(packet_);
            for (int row = 0; row < block_rows; ++row) {
                const int index = block * block_rows + row;
                uint16_t* target = mode == Conversion::blind
                    ? electrical_offset_[half ? 7 - row : row]
                    : raw_[half ? side - 1 - index : index];
                const uint8_t* p = packet_ + 2 + row * side * 2;
                for (int x = 0; x < side; ++x) target[x] = big16(p + 2 * x);
            }
        }
        return true;
    }

    bool captureImage(Conversion mode) {
        auxiliary_sum_ = 0;
        for (int block = 0; block < blocks; ++block)
            if (!captureBlock(mode, block)) return false;
        uint16_t& measurement = mode == Conversion::ptat ? ptat_ : supply_;
        measurement = auxiliary_sum_ / (2 * blocks);
        return true;
    }

    static float lookup(float signal, float ambient) {
        if (!std::isfinite(signal) || !std::isfinite(ambient)) return NAN;
        using namespace heimann_lut;
        constexpr int last_row = sizeof(temperature) / sizeof(temperature[0]) - 1;
        const float ad = std::clamp((signal + adc_offset) / adc_step, 0.0f, float(last_row));
        const float ta = std::clamp((ambient - ambient_first) / ambient_step, 0.0f, 6.0f);
        const int row = std::min(int(ad), last_row - 1), col = std::min(int(ta), 5);
        const float a = temperature[row][col] + (temperature[row][col + 1] - temperature[row][col]) * (ta - col);
        const float b = temperature[row + 1][col] + (temperature[row + 1][col + 1] - temperature[row + 1][col]) * (ta - col);
        return a + (b - a) * (ad - row);
    }

    void calculate(ThermalFrame& frame) const {
        const float ambient = ptat_ * ptat_gradient_ + ptat_offset_;
        const float supply_delta = supply_ - vdd1_ - supply_slope_ * (int(ptat_) - ptat1_);
        for (int y = 0; y < side; ++y) {
            const int er = y % block_rows + (y >= side / 2 ? block_rows : 0);
            for (int x = 0; x < side; ++x) {
                const float drift = thermal_gradient_[y][x] * ptat_ * thermal_scale_ + thermal_offset_[y][x];
                const float voltage = (vdd_gradient_[er][x] * ptat_ * vdd_gradient_scale_ + vdd_offset_[er][x]) *
                                      supply_delta * vdd_offset_scale_;
                const float signal = (raw_[y][x] - drift - electrical_offset_[er][x] - voltage) * sensitivity_scale_[y][x];
                const float decikelvin = lookup(signal, ambient) + global_offset_;
                frame.celsius[y * side + x] = decikelvin * 0.1f - 273.15f;
            }
        }
        repairPixels(frame);
        frame.height = side;
    }

    void repairPixels(ThermalFrame& frame) const {
        constexpr int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
        constexpr int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
        for (unsigned i = 0; i < bad_count_; ++i) {
            const int x = bad_address_[i] % side, y = bad_address_[i] / side;
            float sum = 0;
            unsigned count = 0;
            for (unsigned bit = 0; bit < 8; ++bit) {
                const int sx = x + dx[bit], sy = y + (y < side / 2 ? dy[bit] : -dy[bit]);
                if ((bad_mask_[i] & (1u << bit)) && unsigned(sx) < side && unsigned(sy) < side) {
                    sum += frame.celsius[sy * side + sx];
                    ++count;
                }
            }
            if (count) frame.celsius[y * side + x] = sum / count;
        }
    }
};
