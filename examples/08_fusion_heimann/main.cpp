#include "heimann.hpp"
#include "fusion_lesson.hpp"

static FusionLesson<Heimann32x32> lesson;

void setup() { lesson.begin(); }
void loop() { lesson.step(); }
