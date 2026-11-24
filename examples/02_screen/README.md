# 02 点亮 ST7789

```sh
pio run -e 02_screen
```

烧录后，屏幕从左到右显示红、绿、蓝三块色条。观察横屏方向及颜色顺序。

硬件 SPI 驱动 ST7789，初始化按 240×280 面板设置，随后调整到 280×240 横屏。

[实验入口](main.cpp) · [display.hpp](../../include/display.hpp)
