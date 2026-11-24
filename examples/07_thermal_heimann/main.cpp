#include "heimann.hpp"
#include "thermal_lesson.hpp"

static ThermalLesson<Heimann32x32> lesson;

void setup() { lesson.begin(); }
void loop() { lesson.step(); }
