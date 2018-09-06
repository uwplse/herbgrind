#include <stdio.h>
#include <math.h>

int main() {
  volatile float x, y;
  x = 4.0f;
  y = sqrtf(x);
  printf("%e\n", y);
}
