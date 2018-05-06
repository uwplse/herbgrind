#include <stdio.h>
#include <math.h>

int main() {
  volatile double x1, y1, z1, w;
  x1 = 1 + 1e-15;
  y1 = 1 + 1e-15;
  z1 = 1;
  w = fma(x1, y1, z1);
  printf("%g\n", w);
}
