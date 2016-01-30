#include "pub_tool_clreq.h"
#include "herbgrind.h"
#include "hg_mathreplace_funcs.h"
#include "pub_tool_redir.h"

#define LIBM libmZdsoZa

// This file instructs valgrind to capture calls to the math functions
// listed in hg_mathreplace_funcs.h, and redirect them to the
// appropriate funciton here instead. From here, we pass them through
// the client call mechanism to the valgrind tool code, which captures
// each operation and executes them directly in MPFR, instead of using
// whatever special bit twiddling function the standard math library
// provides. This means we can shadow the function properly, with the
// mpfr equivalent, instead of shadowing the arithmetic in libm that's
// only accurate to 64 bits.


/*----------------------------
====== Unary Ops =============
----------------------------*/

#define HG_WRAP_OP_1(fnname, opname)                    \
  double VG_WRAP_FUNCTION_ZU(LIBM, fnname)(double x);   \
  double VG_WRAP_FUNCTION_ZU(LIBM, fnname)(double x){   \
    double result;                                      \
    double args[1];                                     \
    args[0] = x;                                        \
    HERBGRIND_PERFORM_OP(opname, &result, args);        \
    return result;                                      \
  }

HG_WRAP_OP_1(sqrt, OP_SQRT);
HG_WRAP_OP_1(exp, OP_EXP);
HG_WRAP_OP_1(log, OP_LOG);
HG_WRAP_OP_1(cos, OP_COS);
HG_WRAP_OP_1(sin, OP_SIN);
HG_WRAP_OP_1(tan, OP_TAN);
HG_WRAP_OP_1(asin, OP_ASIN);
HG_WRAP_OP_1(atan, OP_ATAN);
HG_WRAP_OP_1(sinh, OP_SINH);
HG_WRAP_OP_1(cosh, OP_COSH);
HG_WRAP_OP_1(tanh, OP_TANH);
HG_WRAP_OP_1(fabs, OP_ABS);
HG_WRAP_OP_1(expm1, OP_EXPM1);
HG_WRAP_OP_1(log1p, OP_LOG1P);

/*----------------------------
====== Binary Ops ============
----------------------------*/

#define HG_WRAP_OP_2(fnname, opname)                             \
  double VG_WRAP_FUNCTION_ZU(LIBM, fnname)(double x, double y);  \
  double VG_WRAP_FUNCTION_ZU(LIBM, fnname)(double x, double y){  \
    double result;                                               \
    double args[2];                                              \
    args[0] = x;                                                 \
    args[1] = y;                                                 \
    HERBGRIND_PERFORM_OP(opname, &result, args);                 \
    return result;                                               \
  }

HG_WRAP_OP_2(fmod, OP_MOD);
HG_WRAP_OP_2(pow, OP_POW);
HG_WRAP_OP_2(atan2, OP_ATAN2);
HG_WRAP_OP_2(hypot, OP_HYPOT);
