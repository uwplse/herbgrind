#include <stdio.h>
#include <math.h>
#include "herbgrind.h"

double __attribute__ ((noinline)) add(double x, double y){
  return x + y;
}
int main(int argc, char** argv){
  volatile double x, y, z1, z2, z3;
  HERBGRIND_BEGIN();
  z1 = 5;
  z2 = 6;
  z3 = z1 + z2;
  x = add(4, 5);
  y = add(6, z3);
  HERBGRIND_END();
}
