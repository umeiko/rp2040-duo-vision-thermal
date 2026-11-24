#pragma once

#include <Arduino.h>
#include <PicoCamera.h>
#include <Wire.h>
#include "board_pins.hpp"

namespace camera {
inline bool rotate = false;

inline bool begin() {
    Wire.setSDA(pins::shared_sda);
    Wire.setSCL(pins::shared_scl);
    Wire.begin();
    Wire.setClock(100000);

    camera_config_t config{};
    config.pin_pwdn = -1;
    config.pin_reset = pins::camera_reset;
    config.pin_xclk = pins::camera_xclk;
    config.pin_sccb_sda = -1; // Borrow the Wire bus shared with the touch controller.
    config.pin_sccb_scl = -1;
    config.sccb_i2c_port = 0;
    config.pin_d0 = pins::camera_data0;
    config.pin_d1 = pins::camera_data0 + 1;
    config.pin_d2 = pins::camera_data0 + 2;
    config.pin_d3 = pins::camera_data0 + 3;
    config.pin_d4 = pins::camera_data0 + 4;
    config.pin_d5 = pins::camera_data0 + 5;
    config.pin_d6 = pins::camera_data0 + 6;
    config.pin_d7 = pins::camera_data0 + 7;
    config.pin_vsync = pins::camera_vsync;
    config.pin_href = pins::camera_href;
    config.pin_pclk = pins::camera_pclk;
    config.xclk_freq_hz = 10000000;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_QQVGA;
    config.fb_count = 1;
    config.fb_location = PICO_CAMERA_FB_AUTO;
    const auto error = pico_camera_init(&config);
    if (error != PICO_CAMERA_OK) {
        Serial.print("Camera init: ");
        Serial.println(pico_camera_err_str(error));
        return false;
    }
    const auto* sensor = pico_camera_sensor_info_get();
    rotate = sensor && sensor->pid == 0x76; // OV7670's mounting on this board.
    if (sensor) {
        Serial.print("Camera: ");
        Serial.println(sensor->name);
    }
    return true;
}

class Frame {
public:
    explicit Frame(bool ready) : fb_(ready ? pico_camera_fb_get() : nullptr) {}
    ~Frame() { if (fb_) pico_camera_fb_return(fb_); }
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

    bool valid() const {
        return fb_ && fb_->buf && fb_->format == PIXFORMAT_RGB565 && fb_->width && fb_->height &&
               fb_->len >= fb_->width * fb_->height * 2;
    }
    int width() const { return rotate ? fb_->height : fb_->width; }
    int height() const { return rotate ? fb_->width : fb_->height; }

    uint16_t pixel(int x, int y) const {
        const int sx = rotate ? int(fb_->width) - 1 - y : x;
        const int sy = rotate ? x : y;
        const uint8_t* bytes = fb_->buf + (sy * fb_->width + sx) * 2;
        return (uint16_t(bytes[0]) << 8) | bytes[1];
    }
private:
    camera_fb_t* fb_;
};
}
