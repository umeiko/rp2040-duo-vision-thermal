#include "mlx90640.hpp"
#include "thermal_lesson.hpp"

static ThermalLesson<Mlx90640> lesson;

void setup() { lesson.begin(); }
void loop() { lesson.step(); }
