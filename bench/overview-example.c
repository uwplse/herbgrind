#include <stdio.h>

struct Point { float x; float y; };

float foo(struct Point a, struct Point b) {
  return ((a.x + a.y) - (b.x + b.y)) * a.x;
}

float bar(float x, float y, float z) {
  struct Point p1 = {x, y};
  struct Point p2 = {x, z};
  return foo(p1, p2);
}

int main() {
  float a = bar(2, 3, -7);
  float b = bar(1e12, 1, 0);
  float c = bar(.034, 57.2, 5);
  printf("%f %f %f\n", a, b, c);
}
