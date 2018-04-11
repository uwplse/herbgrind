#include <stdio.h>

double addTwo(double x, double y){
  return x + y;
}

int main() {
  volatile double x, y, z;
  z = 3;
  x = addTwo(z + 2, 4);
  y = addTwo(5, 7);
  printf("%e\n%e\n", x, y);
}
