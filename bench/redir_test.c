#include <stdio.h>
#include <math.h>

int main(int argc, char** argv){
  volatile double x = 0.0;
  for (int i = 0; i < 2; ++i) {
    x += sqrt(i);
  }
  printf("%e\n", x);
}
