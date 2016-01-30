#include <stdio.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  double x, y;
  HERBGRIND_BEGIN();
  x = 5;
  y = tan(x) - (sin(x)/cos(x));
  HERBGRIND_END();
  printf("%e\n", y);
  return 0;
}
