#include "pub_tool_clreq.h"
#include "pub_tool_redir.h"

#include "include/herbgrind.h"
#include "include/hg_mathreplace_funcs.h"

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

#define WRAP_UNARY(fnname, opname)                    \
  double VG_REPLACE_FUNCTION_ZU(LIBM, fnname)(double x);   \
  double VG_REPLACE_FUNCTION_ZU(LIBM, fnname)(double x){   \
    double result;                                      \
    double args[1];                                     \
    args[0] = x;                                        \
    HERBGRIND_PERFORM_OP(opname, &result, args);        \
    return result;                                      \
  }

// This macro is defined in include/hg_mathreplace_funcs.h, and
// invokes the above macro for each unary operation that needs to be
// wrapped.
WRAP_UNARY_OPS

/*----------------------------
====== Binary Ops ============
----------------------------*/

#define WRAP_BINARY(fnname, opname)                             \
  double VG_REPLACE_FUNCTION_ZU(LIBM, fnname)(double x, double y);  \
  double VG_REPLACE_FUNCTION_ZU(LIBM, fnname)(double x, double y){  \
    double result;                                               \
    double args[2];                                              \
    args[0] = x;                                                 \
    args[1] = y;                                                 \
    HERBGRIND_PERFORM_OP(opname, &result, args);                 \
    return result;                                               \
  }

// This macro is defined in include/hg_mathreplace_funcs.h, and
// invokes the above macro for each binary operation that needs to be
// wrapped.
WRAP_BINARY_OPS

/*----------------------------
====== Ternary Ops ===========
----------------------------*/

#define WRAP_TERNARY(fnname, opname)                                    \
  double VG_REPLACE_FUNCTION_ZU(LIBM, fnname)(double x, double y, double z); \
  double VG_REPLACE_FUNCTION_ZU(LIBM, fnname)(double x, double y, double z){ \
    double result;                                                      \
    double args[3];                                                     \
    args[0] = x;                                                        \
    args[1] = y;                                                        \
    args[2] = z;
    HERBGRIND_PERFORM_OP(opname, &result, args);                        \
    return result;                                                      \
  }

// This macro is defined in include/hg_mathreplace_funcs.h, and
// invokes the above macro for each ternary operation that needs to be
// wrapped.
WRAP_TERNARY_OPS
