#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  double x,y;
  x = 10e12;
  /* for(int i = 0; i < 2; ++i){ */
  /*   x *= (i + 1) * 10; */
    HERBGRIND_BEGIN();
    y = sqrt(x + 1) - sqrt(x);
    HERBGRIND_END();
  /* } */
  printf("%e\n", y);
  return 0;
}
