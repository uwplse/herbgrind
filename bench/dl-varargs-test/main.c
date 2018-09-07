
#include <stdio.h>
#include "dl.h"

int main(int argc, char** argv){
  volatile double x = 1.0;
  volatile double y = 2.0;
  volatile double z = x + y;
  double res = sum(2, z, 2.0);
  printf("Res is: %f\n", res);
  return 0;
}
