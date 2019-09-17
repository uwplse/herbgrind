#include <stdio.h>
#include <math.h>

int main() {
  volatile double x,y;
  x = 1e16;
  y = exp2l(x);
  printf("%e\n", y);
  return 0;
}
