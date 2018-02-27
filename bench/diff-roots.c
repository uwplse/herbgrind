#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "herbgrind.h"

double rand_double() {
        long long c0 = rand()&0xffff;
        long long c1 = rand()&0xffff;
        long long c2 = rand()&0xffff;
        long long c3 = rand()&0xffff;
        long long c = ((c3 << 48) | (c2 << 32) | (c1<<16) | c0);
        return *(double*)&c;
}

int main(int argc, char** argv){
  double x,y;
  for (int i = 0; i < 1000; i++){
    x = fabs(rand_double());
    y = sqrt(x + 1) - sqrt(x);
    printf("%e\n", y);
  }
  return 0;
}
