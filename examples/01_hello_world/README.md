# 01 USB 串口、按键与电池电压

```sh
pio run -e 01_hello_world
```

打开 115200 波特率串口。每 500 ms 输出一行。按下 KEY 可看到 `pressed`，接电池后对照万用表查看 `VBAT`。

GPIO28 的 12 位 ADC 读数先换成电压，再乘上 30 kΩ 与 68 kΩ 分压网络的校正系数。

[实验入口](main.cpp) · [board_pins.hpp](../../include/board_pins.hpp)
