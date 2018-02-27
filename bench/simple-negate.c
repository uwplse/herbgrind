#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  volatile float x = 1e-20;
  volatile float y = (- x);
  volatile float z = 5.0;
  float w = (y + z) - z;
  printf("%e\n", w);
}
