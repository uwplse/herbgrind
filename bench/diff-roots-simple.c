#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  double x,y;
  x = 10e15;
  HERBGRIND_BEGIN();
  y = sqrt(x + 1) - sqrt(x);
  HERBGRIND_MARK_IMPORTANT(y);
  HERBGRIND_END();
  printf("%e\n", y);
  return 0;
}
