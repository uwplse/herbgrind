#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "herbgrind.h"
#include "driver-macros.h"

#ifndef NPTS
#define NPTS 8000
#endif

double f_id(ARGS(double));

extern double pts[NPTS][NARGS];

int main(int argc, char** argv){
  double result;
  double min_input, max_input;

  // For each sample
  for (int i = 0; i < NPTS; ++i){
    for (int j = 0; j < NARGS; j++){
      if (pts[i][j] > max_input) max_input = pts[i][j];
      if (pts[i][j] < min_input) min_input = pts[i][j];
    }

    // Run the program on the args
    HERBGRIND_BEGIN();
    result = INVOKE(f_id, pts[i]);
    HERBGRIND_END();

    /* printf(" --> %e\n", result); */
  }
  printf("Min legal input was %e, max was %e\n", min_input, max_input);
}

