#include <math.h>
 
#include <stdio.h>
#include <stdlib.h>
#include "herbgrind.h"
 
static inline float iprod(float* a,float* b)
{
  return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]);
}
 
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
  float r_kj[3];
  float nrkj2, nrkj;
  for (int i = 0; i < 10; i++){
    r_kj[0] = rand_legit_positive_float();
    r_kj[1] = rand_legit_positive_float();
    r_kj[2] = rand_legit_positive_float();
    HERBGRIND_BEGIN();
    nrkj2 = iprod(r_kj, r_kj);
    nrkj = sqrt(nrkj2);
    HERBGRIND_END();
    printf("%e\n", nrkj);
  }
  return 0;
}
