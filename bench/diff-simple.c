#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  double x,y;
  x = 10e12;
  HERBGRIND_BEGIN();
  y = x + 1 - x;
  HERBGRIND_END();
  printf("%e\n", y);
  return 0;
}
