#include <stdio.h>
#include "herbgrind.h"

double addTwo(double x, double y){
  return x + y;
}

int main(int argc, char** argv){
  volatile double x, y, z;
  z = 3;
  HERBGRIND_BEGIN();
  x = addTwo(z + 2, 4);
  y = addTwo(5, 7);
  HERBGRIND_END();
}
