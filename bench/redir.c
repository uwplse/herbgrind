#include <stdio.h>
#include <math.h>

int main() {
  volatile double x = 0.0;
  for (int i = 0; i < 2; ++i) {
    x += sqrt(i);
  }
  printf("%e\n", x);
}
