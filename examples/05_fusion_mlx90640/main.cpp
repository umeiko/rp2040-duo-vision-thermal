#include "mlx90640.hpp"
#include "fusion_lesson.hpp"

static FusionLesson<Mlx90640> lesson;

void setup() { lesson.begin(); }
void loop() { lesson.step(); }
