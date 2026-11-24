# 08 Heimann 5.0 双光融合

```sh
pio run -e 08_fusion_heimann
```

安装 Heimann 5.0 和相机。烧录后对准有明显边缘的温热目标，调整热图在可见光画面中的叠加位置。

本章沿用第 05 章的相机与融合流程，传入 32×32 热帧。 [透明度与配准命令](../05_fusion_mlx90640)

[实验入口](main.cpp) · [fusion.hpp](../../include/fusion.hpp) · [heimann.hpp](../../include/heimann.hpp)
