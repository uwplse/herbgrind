#include <stdio.h>

double foo(double x);
double foo(double x){
  return ((x * 5) + 1) - (x * 5);
}

double bar(double x, double y);
double bar(double x, double y){
  return ((x * 5) + (1 + y)) - ((x * 5) + y);
}

int main(int argc, char** argv){
  volatile double x, y, z, t;
  x = 1.0;
  y = 1.0e16;
  z = foo(x);
  t = foo(y);
  printf("z = %e\n", z);
  printf("t = %e\n", t);
  volatile double a, b;
  a = bar(y, 8);
  b = bar(x, 9);
  printf("a = %e\n", a);
  printf("b = %e\n", b);
  return 0;
}
