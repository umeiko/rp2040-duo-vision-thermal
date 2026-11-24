# 04 MLX90640 测温

```sh
pio run -e 04_thermal_mlx90640
```

安装 MLX90640，烧录后把手移近探头，查看画面热点和底部的最高、最低温。

从 I²C1 读取 EEPROM 与两张互补子页，通过 Melexis API 将像素转换为摄氏度。

[实验入口](main.cpp) · [mlx90640.hpp](../../include/mlx90640.hpp) · [render.hpp](../../include/render.hpp)
