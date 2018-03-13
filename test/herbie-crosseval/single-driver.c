#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "herbgrind.h"
#include "driver-macros.h"

double f_id(ARGS(double));

int main(int argc, char** argv){
  double args[NARGS];
  double result;

  if (argc < NARGS + 1){
    printf("Hey! You forgot to pass the arguments.\n");
    exit(1);
  }
  // Parse the arguments from the command line.
  for(int i = 0; i < NARGS; ++i){
    args[i] = atof(argv[i+1]);
  }

  HERBGRIND_BEGIN();
  result = INVOKE(f_id, args);
  HERBGRIND_END();

  printf("result: %e\n", result);
}
