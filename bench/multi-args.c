#include "herbgrind.h"
#include <stdio.h>
#include <math.h>

int main(int argc, char** argv){
  double xs[2] = {1.2, 4.5};
  double ys[2] = {0.5, 0.7};
  double x, y, z;
  for(int i = 0; i < 2; ++i){
    x = xs[i];
    y = ys[i];
    z = ( x + y ) - y;
    printf("%f\n", z);
  }
}
