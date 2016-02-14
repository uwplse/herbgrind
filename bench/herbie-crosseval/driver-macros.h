#ifndef _DRIVER_MACROS
#define _DRIVER_MACROS

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

#endif
