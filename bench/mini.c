#include <stdio.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  volatile double x,y;
  x = 3;
  y = sqrt(x + 1);
  printf("%e\n", y);
  return 0;
}
