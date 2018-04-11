#include <stdio.h>

int main() {
  volatile double x = 1.0;
  double y = x + x;
  printf("%e\n", y);
  return 0;
}
