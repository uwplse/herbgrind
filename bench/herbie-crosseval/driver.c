#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "herbgrind.h"
#include "driver-macros.h"

#ifndef NPTS
#define NPTS 8000
#endif

double f(ARGS(double));

extern double pts[NPTS][NARGS];

int main(int argc, char** argv){
  double result;

  // For each sample
  for (int i = 0; i < NPTS; ++i){
    // Run the program on the args
    HERBGRIND_BEGIN();
    result = INVOKE(f, pts[i]);
    HERBGRIND_END();
    HERBGRIND_MARK_IMPORTANT(result);

    /* printf(" --> %e\n", result); */
  }
}

