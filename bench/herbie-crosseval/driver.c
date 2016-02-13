#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "herbgrind.h"

#ifndef NARGS
#define NARGS 1
#endif

#if NARGS == 1
#define ARGS(t) t
#define INVOKE(f,args) f(args[0])
#define RANDARGS(argsName) argsName[0] = rand_double()
#define PRINTARGS(argsName) printf("%e", argsName[0]);
#elif NARGS == 2
#define ARGS(t) t, t
#define INVOKE(f,args) f(args[0], args[1])
#define RANDARGS(argsName) \
  argsName[0] = rand_double();\
  argsName[1] = rand_double()
#define PRINTARGS(argsName) printf("%e, %e", \
                                   argsName[0], \
                                   argsName[1]);
#elif NARGS == 3
#define ARGS(t) t, t, t
#define INVOKE(f,args) f(args[0], args[1], args[2])
#define RANDARGS(argsName) \
  argsName[0] = rand_double();\
  argsName[1] = rand_double();\
  argsName[2] = rand_double()
#define PRINTARGS(argsName) printf("%e, %e, %e", \
                                   argsName[0], \
                                   argsName[1], \
                                   argsName[2]);
#elif NARGS == 4
#define ARGS(t) t, t, t, t
#define INVOKE(f,args) f(args[0], args[1], args[2], args[3])
#define RANDARGS(argsName) \
  argsName[0] = rand_double();\
  argsName[1] = rand_double();\
  argsName[2] = rand_double();\
  argsName[3] = rand_double()
#define PRINTARGS(argsName) printf("%e, %e, %e, %e", \
                                   argsName[0], \
                                   argsName[1], \
                                   argsName[2]);
#elif NARGS == 5
#define ARGS(t) t, t, t, t, t
#define INVOKE(f,args) f(args[0], args[1], args[2], args[3], args[4])
#define RANDARGS() \
  argsName[0] = rand_double();\
  argsName[1] = rand_double();\
  argsName[2] = rand_double();\
  argsName[3] = rand_double();\
  argsName[4] = rand_double()
#define PRINTARGS(argsName) printf("%e, %e, %e, %e, %e", \
                                   argsName[0], \
                                   argsName[1], \
                                   argsName[2], \
                                   argsName[3]);
#elif NARGS == 6
#define ARGS(t) t, t, t, t, t, t
#define INVOKE(f,args) f(args[0], args[1], args[2], args[3], args[4], args[5])
#define RANDARGS() \
  argsName[0] = rand_double();\
  argsName[1] = rand_double();\
  argsName[2] = rand_double();\
  argsName[3] = rand_double();\
  argsName[4] = rand_double();\
  argsName[5] = rand_double()
#define PRINTARGS(argsName) printf("%e, %e, %e, %e, %e, %e", \
                                   argsName[0], \
                                   argsName[1], \
                                   argsName[2], \
                                   argsName[3], \
                                   argsName[4]);
#else
#define ARGS(t) abort()
#define INVOKE(f,args) abort()
#define RANDARGS() abort()
#endif

double f_id(ARGS(double));

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

  // For each sample
  for (int i = 0; i < num_samples; ++i){
    // Get some random arguments. If they end up including nans or
    // infs, just keep getting them until they dont. If the result
    // computed with floats would be nan or inf, do the same.
    /* bool valid_args = false; */
    /* do { */
    /*   RANDARGS(args); */
    /*   valid_args = true; */
    /*   for (int j = 0; j < NARGS; ++j){ */
    /*     if (!ordinaryd(args[j])) valid_args = false; */
    /*     if (!ordinaryd(INVOKE(f_id, args))) valid_args = false; */
    /*   } */
    /*   if (valid_args){ */
    /*     for (int j = 0; j < NARGS; ++j){ */
    /*       if (args[j] > max_input) max_input = args[j]; */
    /*       if (args[j] < min_input) min_input = args[j]; */
    /*     } */
    /*   } */
    /* } while (!valid_args); */
    RANDARGS(args);
    for (int j = 0; j < NARGS; ++j){
      if (args[j] < 0) args[j] = -args[j];
      if (args[j] > max_input) max_input = args[j];
      if (args[j] < min_input) min_input = args[j];
    }

    /* PRINTARGS(args) */

    // Run the program on the args
    /* HERBGRIND_END(); */
    /* INVOKE(f_id, args); */
    HERBGRIND_BEGIN();
    result = INVOKE(f_id, args);
    HERBGRIND_END();

    /* printf(" --> %e\n", result); */
  }
  printf("Min legal input was %e, max was %e\n", min_input, max_input);
}

