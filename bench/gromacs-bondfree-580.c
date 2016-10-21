#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include "herbgrind.h"

double rand_double() {
        long long c0 = rand()&0xffff;
        long long c1 = rand()&0xffff;
        long long c2 = rand()&0xffff;
        long long c3 = rand()&0xffff;
        long long c = ((c3 << 48) | (c2 << 32) | (c1<<16) | c0);
        return *(double*)&c;
}
 
float rand_legit_positive_float() {
  float result;
  do {
    result = rand_double();
  } while (result != result || result * result == INFINITY || result <= 0);
  return result;
}
 
int main(int argc, char** argv){
  double a, b;
  double nrkj;
  for (int i = 0; i < 3; i++){
    a = rand_legit_positive_float();
    b = rand_legit_positive_float();
    HERBGRIND_BEGIN();
    nrkj = sqrt(a + b);
    HERBGRIND_END();
    printf("%e\n", nrkj);
  }
  return 0;
}
