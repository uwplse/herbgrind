#include <stdio.h>
#include <math.h>

int main(int argc, char** argv){
  double x,y;
  x = 1e16;
  y = (x + 1) - x;
  printf("%e\n", y);
  return 0;
}
