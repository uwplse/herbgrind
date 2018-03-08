#include <stdio.h>

int main() {
  volatile double x;
  int cmp;
  x = 0.0;
  cmp = (x == 0.0);
  printf("%d\n", cmp);
}
