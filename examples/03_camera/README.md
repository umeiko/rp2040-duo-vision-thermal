# 03 PicoCamera 可见光预览

```sh
pio run -e 03_camera
```

烧录后打开串口，确认出现相机名称。镜头对着文字或不对称图案，检查画面可读且方向正常。

PicoCamera 在 PIO 和 DMA 上采集 160×120 RGB565。OV7670 按模组安装方向旋转 90°。释放当前帧后，驱动可重用同一块缓冲。 [PicoCamera API](https://arduino-pico.readthedocs.io/en/latest/camera.html)

[实验入口](main.cpp) · [camera.hpp](../../include/camera.hpp)
