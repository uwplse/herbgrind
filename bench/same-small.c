#include <stdio.h>

int main(int argc, char** argv){
  volatile double x = 1.0;
  double y = x + x;
  printf("%e\n", y);
  return 0;
}
