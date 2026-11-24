# 07 Heimann HTPA32×32 5.0 测温

```sh
pio run -e 07_thermal_heimann
```

安装 Heimann 5.0 探头。烧录后把手移近探头，观察热图与摄氏度读数是否随目标变化。

从 EEPROM 读取标定值，再按块读取 32×32 热帧，结合电偏置、PTAT 和 VDD 补偿查表求温。

[实验入口](main.cpp) · [heimann.hpp](../../include/heimann.hpp)
