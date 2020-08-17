#include <stdio.h>
#include <math.h>
#include <herbgrind.h>

int main() {
  volatile double x, y, z;
  x = 123.456;
  y = modf(x, (double*)&z);
  HERBGRIND_MARK_IMPORTANT(z);
  HERBGRIND_MARK_IMPORTANT(y);
  printf("%e %e\n", z, y);
  return 0;
}
