#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "herbgrind.h"
#include "driver-macros.h"

double f_id(ARGS(double));
double f_im(ARGS(double));
void setup_mpfr_f_im();

double rand_double() {
        long long c0 = rand()&0xffff;
        long long c1 = rand()&0xffff;
        long long c2 = rand()&0xffff;
        long long c3 = rand()&0xffff;
        long long c = ((c3 << 48) | (c2 << 32) | (c1<<16) | c0);
        return *(double*)&c;
}
char ordinaryd(double x) {
        return 1 / x != 0 && x == x;
}

int main(int argc, char** argv){
  double args[NARGS];
  double result;
  bool ordinaryArgs;
  unsigned int num_samples = 8000;
  double min_input, max_input;
  // Parse a num_samples argument, if it's there.
  if (argc > 1) {
    num_samples = atoi(argv[1]);
  }

  setup_mpfr_f_im();

  // For each sample
  for (int i = 0; i < num_samples; ++i){
    // Get some random arguments. If they end up including nans or
    // infs, just keep getting them until they dont. If the result
    // computed with floats would be nan or inf, do the same.
    bool valid_args = false;
    do {
      RANDARGS(args);
      valid_args = true;
      for (int j = 0; j < NARGS; ++j){
        if (!ordinaryd(args[j])) valid_args = false;
        else if (!ordinaryd(INVOKE(f_im, args))) valid_args = false;
      }
      if (valid_args){
        for (int j = 0; j < NARGS; ++j){
          if (args[j] > max_input) max_input = args[j];
          if (args[j] < min_input) min_input = args[j];
        }
      }
    } while (!valid_args);

    // Run the program on the args
    HERBGRIND_BEGIN();
    result = INVOKE(f_id, args);
    HERBGRIND_END();

    /* printf(" --> %e\n", result); */
  }
  printf("Min legal input was %e, max was %e\n", min_input, max_input);
}

