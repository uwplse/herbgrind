#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  HERBGRIND_BEGIN();
  volatile float x = 3.0;
  volatile float y = (- x);
  volatile float z = 5.0;
  float w = y + z;
  HERBGRIND_END();
  printf("Result: %e\n", w);
}
