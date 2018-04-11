#include <stdio.h>

int main() {
  double xs[2] = {1.2, 1e-12};
  double ys[2] = {0.5, 1e12};
  double x, y, z;
  for(int i = 0; i < 2; ++i){
    x = xs[i];
    y = ys[i];
    z = ( x + y ) - y;
    printf("%f\n", z);
  }
}
