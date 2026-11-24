# 06 双核热帧采集

```sh
pio run -e 06_multicore
```

安装 MLX90640 和相机。烧录后观察融合画面，也可用与第 05 章相同的串口命令调整透明度和配准。

核心 1 操作热探头。核心 0 操作 USB 串口、相机和显示屏。两个核心用互斥锁交接完整热帧，采集和渲染期间都不占用这把锁。 [第 05 章命令说明](../05_fusion_mlx90640)

[实验入口](main.cpp) · [thermal_frame.hpp](../../include/thermal_frame.hpp)
