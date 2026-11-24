# 09 读取电容触摸屏

```sh
pio run -e 09_touch_screen
```

烧录后，用手指划过屏幕。画点应跟随手指，按 KEY 可清屏。

通过 I²C0 探测 CST816 系列（`0x15`）或 FT6x36（`0x38`），读取触点并转换为横屏坐标。

[实验入口](main.cpp) · [display.hpp](../../include/display.hpp) · [board_pins.hpp](../../include/board_pins.hpp)
