#include <stdio.h>
#include <math.h>

int main() {
  volatile double x,y;
  x = 3;
  y = sqrt(x + 1);
  printf("%e\n", y);
  return 0;
}
