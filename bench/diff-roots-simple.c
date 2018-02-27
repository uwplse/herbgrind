#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  double x,y;
  x = 1e16;
  y = sqrt(x + 1) - sqrt(x);
  printf("%e\n", y);
  return 0;
}
