#include <stdio.h>
#include <math.h>

int main() {
  volatile float x = 1e-20;
  volatile float y = (- x);
  volatile float z = 5.0;
  float w = (y + z) - z;
  printf("%e\n", w);
}
