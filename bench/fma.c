#include "herbgrind.h"
#include <stdio.h>
#include <math.h>

int main(int argc, char** argv){
  volatile double x1, y1, z1;
  double x, y, z, w;
  x1 = 5.0;
  y1 = 1.0;
  z1 = 2.0;
  x = x1 + x1;
  y = y1 + y1;
  z = z1 + z1;
  w = fma(x, y, z);
  printf("%f\n", w);
}
