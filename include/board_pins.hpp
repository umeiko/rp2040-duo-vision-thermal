#pragma once

namespace pins {
constexpr int button = 10;
constexpr int battery = 28;
constexpr int screen_sck = 6, screen_mosi = 7, screen_dc = 8, screen_cs = 9;
constexpr int screen_backlight = 11, screen_reset = -1;
constexpr int camera_vsync = 2, camera_href = 3, camera_pclk = 4, camera_xclk = 5;
constexpr int camera_data0 = 14, camera_reset = -1;
constexpr int shared_sda = 12, shared_scl = 13;
constexpr int thermal_sda = 26, thermal_scl = 27;
}
