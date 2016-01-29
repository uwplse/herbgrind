#include <stdio.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  double x,y;
  HERBGRIND_BEGIN();
  x = 10e10;
  y = sqrt(x + 1) - sqrt(x);
  HERBGRIND_END();
  printf("%e\n", y);
  return 0;
}
