#include <stdio.h>
#include <math.h>
#include "herbgrind.h"

void calcY(double* y, double x);

int main(int argc, char** argv){
  double x,y;
  x = 1e10;
  calcY(&y, x);
  calcY(&y, x);
  printf("%e\n", y);
  return 0;
}

void calcY(double* y, double x){
  *y = sqrt(x + 1) - sqrt(x);
}
