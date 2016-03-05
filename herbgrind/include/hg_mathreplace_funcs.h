#ifndef _HG_MATHREPLACE_FUNCS
#define _HG_MATHREPLACE_FUNCS

// This monstrosity of a file sets up a lot of the machinery to
// replace libm functions called by client programs with a call into
// tool code which handles them properly. Basically, I wanted to make
// it very easy to add new libm functions without having to change
// code in like four different places. So I took every place you would
// have to change code to add new functions, and abstraced them away
// into macros, which all live in this file. This means the rest of
// the project is agnostic to which functions from libm we support,
// but it also means there are mysterious macros scattered throughout
// that code, and this file is a giant mess.

// These operations are based on the ones that Herbie supports natively.

// A list of all the unary operations, for the enum definition farther
// down in the file.
#define UNARY_OPS_LIST                                          \
  OP_SQRT,                                                      \
    OP_CBRT,                                                    \
    OP_CBRTF,                                                   \
    OP_EXP,                                                     \
    OP_LOG,                                                     \
    OP_ABS,                                                     \
    OP_EXPM1,                                                   \
    OP_LOG1P,                                                   \
    OP_CEIL,                                                    \
    OP_CEILF,                                                   \
                                                                \
    OP_ERF,                                                     \
    OP_ERFF,                                                    \
    OP_ERFC,                                                    \
    OP_ERFCF,                                                   \
                                                                \
    OP_COS,                                                     \
    OP_COSF,                                                    \
    OP_SIN,                                                     \
    OP_SINF,                                                    \
    OP_TAN,                                                     \
    OP_TANF,                                                    \
    /* OP_COTAN, */ /* Looks like libm doesn't have this one.*/ \
    OP_ASIN,                                                    \
    OP_ASINF,                                                   \
    OP_ACOS,                                                    \
    OP_ACOSF,                                                   \
    OP_ATAN,                                                    \
    OP_ATANF,                                                   \
                                                                \
    OP_SINH,                                                    \
    OP_SINHF,                                                   \
    OP_COSH,                                                    \
    OP_COSHF,                                                   \
    OP_TANH,                                                    \
    OP_TANHF,                                                   \
    OP_ASINH,                                                   \
    OP_ASINHF,                                                  \
    OP_ACOSH,                                                   \
    OP_ACOSHF,                                                  \
    OP_ATANH,                                                   \
    OP_ATANHF

// A list of all the binary ops comma seperated, for the enum
// definition farther down in the file.
#define BINARY_OPS_LIST                         \
  OP_MOD,                                       \
    OP_POW,                                     \
    OP_ATAN2,                                   \
    OP_ATAN2F,                                  \
    OP_HYPOT,                                   \
                                                \
    OP_COPYSIGN,                                \
    OP_COPYSIGNF

// A bunch of case statements for each unary op whose mpfr function
// doesn't need a rounding mode, used in runtime/hg_mathreplace.c
#define UNARY_OPS_ROUND_CASES                   \
  case OP_SQRT:                                 \
 case OP_CBRT:                                  \
 case OP_CBRTF:                                 \
 case OP_EXP:                                   \
 case OP_LOG:                                   \
 case OP_ABS:                                   \
 case OP_EXPM1:                                 \
 case OP_LOG1P:                                 \
                                                \
 case OP_ERF:                                   \
 case OP_ERFF:                                  \
 case OP_ERFC:                                  \
 case OP_ERFCF:                                 \
                                                \
 case OP_COS:                                   \
 case OP_COSF:                                  \
 case OP_SIN:                                   \
 case OP_SINF:                                  \
 case OP_TAN:                                   \
 case OP_TANF:                                  \
 case OP_ASIN:                                  \
 case OP_ASINF:                                 \
 case OP_ACOS:                                  \
 case OP_ACOSF:                                 \
 case OP_ATAN:                                  \
 case OP_ATANF:                                 \
                                                \
 case OP_SINH:                                  \
 case OP_SINHF:                                 \
 case OP_COSH:                                  \
 case OP_COSHF:                                 \
 case OP_TANH:                                  \
 case OP_TANHF:                                 \
 case OP_ASINH:                                 \
 case OP_ASINHF:                                \
 case OP_ACOSH:                                 \
 case OP_ACOSHF:                                \
 case OP_ATANH:                                 \
 case OP_ATANHF:

// Same as above, but for those that don't need a rounding mode.
#define UNARY_OPS_NOROUND_CASES                 \
  case OP_CEIL:                                 \
 case OP_CEILF:

// For places where we don't care about the rounding mode.
#define UNARY_OPS_CASES                         \
  UNARY_OPS_ROUND_CASES                         \
  UNARY_OPS_NOROUND_CASES

// The binary operation cases, used in runtime/hg_mathreplace.c
#define BINARY_OPS_CASES \
  case OP_MOD:           \
 case OP_POW:            \
 case OP_ATAN2:          \
 case OP_ATAN2F:         \
 case OP_HYPOT:          \
 case OP_COPYSIGN:       \
 case OP_COPYSIGNF:

// A switch statement to populate the plain_opname, op_symbol, and
// mpfr_func for all the unary ops that need a rounding mode.
#define GET_UNARY_OPS_ROUND_INFO(op)                    \
  switch(op){                                           \
  case OP_SQRT:                                         \
  plain_opname = "square root";                         \
  op_symbol = "sqrt";                                   \
  mpfr_func = mpfr_sqrt;                                \
  break;                                                \
 case OP_CBRT:                                          \
 plain_opname = "cube root";                            \
 op_symbol = "cbrt";                                    \
 mpfr_func = mpfr_cbrt;                                 \
 break;                                                 \
 case OP_CBRTF:                                         \
 plain_opname = "cube root (float)";                    \
 op_symbol = "cbrtf";                                   \
 mpfr_func = mpfr_cbrt;                                 \
 break;                                                 \
 case OP_EXP:                                           \
 plain_opname = "exponentiate";                         \
 op_symbol = "exp";                                     \
 mpfr_func = mpfr_exp;                                  \
 break;                                                 \
 case OP_LOG:                                           \
 plain_opname = "log";                                  \
 op_symbol = "log";                                     \
 mpfr_func = mpfr_log;                                  \
 break;                                                 \
 case OP_ABS:                                           \
 plain_opname = "absolute";                             \
 op_symbol = "abs";                                     \
 mpfr_func = mpfr_abs;                                  \
 break;                                                 \
 case OP_EXPM1:                                         \
 plain_opname = "exponentiate minus one";               \
 op_symbol = "expm1";                                   \
 mpfr_func = mpfr_expm1;                                \
 break;                                                 \
 case OP_LOG1P:                                         \
 plain_opname = "plus 1 log";                           \
 op_symbol = "log1p";                                   \
 mpfr_func = mpfr_log1p;                                \
 break;                                                 \
                                                        \
 case OP_ERF:                                           \
 plain_opname = "error function";                       \
 op_symbol = "erf";                                     \
 mpfr_func = mpfr_erf;                                  \
 break;                                                 \
 case OP_ERFF:                                          \
 plain_opname = "error function (float)";               \
 op_symbol = "erff";                                    \
 mpfr_func = mpfr_erf;                                  \
 break;                                                 \
 case OP_ERFC:                                          \
 plain_opname = "complementary error function";         \
 op_symbol = "erfc";                                    \
 mpfr_func = mpfr_erfc;                                 \
 break;                                                 \
 case OP_ERFCF:                                         \
 plain_opname = "complementary error function (float)"; \
 op_symbol = "erfcf";                                   \
 mpfr_func = mpfr_erfc;                                 \
 break;                                                 \
                                                        \
 case OP_COS:                                           \
 plain_opname = "cosine";                               \
 op_symbol = "cos";                                     \
 mpfr_func = mpfr_cos;                                  \
 break;                                                 \
 case OP_COSF:                                          \
 plain_opname = "cosine (float)";                       \
 op_symbol = "cosf";                                    \
 mpfr_func = mpfr_cos;                                  \
 break;                                                 \
 case OP_SIN:                                           \
 plain_opname = "sine";                                 \
 op_symbol = "sin";                                     \
 mpfr_func = mpfr_sin;                                  \
 break;                                                 \
 case OP_SINF:                                          \
 plain_opname = "sine (float)";                         \
 op_symbol = "sinf";                                    \
 mpfr_func = mpfr_sin;                                  \
 break;                                                 \
 case OP_TAN:                                           \
 plain_opname = "tangent";                              \
 op_symbol = "tan";                                     \
 mpfr_func = mpfr_tan;                                  \
 break;                                                 \
 case OP_TANF:                                          \
 plain_opname = "tangent (float)";                      \
 op_symbol = "tanf";                                    \
 mpfr_func = mpfr_tan;                                  \
 break;                                                 \
 case OP_ASIN:                                          \
 plain_opname = "arcsine";                              \
 op_symbol = "asin";                                    \
 mpfr_func = mpfr_asin;                                 \
 break;                                                 \
 case OP_ASINF:                                         \
 plain_opname = "arcsine (float)";                      \
 op_symbol = "asinf";                                   \
 mpfr_func = mpfr_asin;                                 \
 break;                                                 \
 case OP_ACOS:                                          \
 plain_opname = "arccosine";                            \
 op_symbol = "acos";                                    \
 mpfr_func = mpfr_acos;                                 \
 break;                                                 \
 case OP_ACOSF:                                         \
 plain_opname = "arccosine (float)";                    \
 op_symbol = "acosf";                                   \
 mpfr_func = mpfr_acos;                                 \
 break;                                                 \
 case OP_ATAN:                                          \
 plain_opname = "arctangent";                           \
 op_symbol = "atan";                                    \
 mpfr_func = mpfr_atan;                                 \
 break;                                                 \
 case OP_ATANF:                                         \
 plain_opname = "arctangent (float)";                   \
 op_symbol = "atanf";                                   \
 mpfr_func = mpfr_atan;                                 \
 break;                                                 \
 case OP_SINH:                                          \
 plain_opname = "hyperbolic sine";                      \
 op_symbol = "sinh";                                    \
 mpfr_func = mpfr_sinh;                                 \
 break;                                                 \
 case OP_SINHF:                                         \
 plain_opname = "hyperbolic sine (float)";              \
 op_symbol = "sinhf";                                   \
 mpfr_func = mpfr_sinh;                                 \
 break;                                                 \
 case OP_COSH:                                          \
 plain_opname = "hyperbolic cosine";                    \
 op_symbol = "cosh";                                    \
 mpfr_func = mpfr_cosh;                                 \
 break;                                                 \
 case OP_COSHF:                                         \
 plain_opname = "hyperbolic cosine (float)";            \
 op_symbol = "coshf";                                   \
 mpfr_func = mpfr_cosh;                                 \
 break;                                                 \
 case OP_TANH:                                          \
 plain_opname = "hyperbolic tangent";                   \
 op_symbol = "tanh";                                    \
 mpfr_func = mpfr_tanh;                                 \
 break;                                                 \
 case OP_TANHF:                                         \
 plain_opname = "hyperbolic tangent (float)";           \
 op_symbol = "tanhf";                                   \
 mpfr_func = mpfr_tanh;                                 \
 break;                                                 \
 case OP_ASINH:                                         \
 plain_opname = "hyperbolic arc-sine (float)";          \
 op_symbol = "asinhf";                                  \
 mpfr_func = mpfr_asinh;                                \
 break;                                                 \
 case OP_ASINHF:                                        \
 plain_opname = "hyperbolic arc-sine";                  \
 op_symbol = "asinh";                                   \
 mpfr_func = mpfr_asinh;                                \
 break;                                                 \
 case OP_ACOSH:                                         \
 plain_opname = "hyperbolic arc-cosine";                \
 op_symbol = "acosh";                                   \
 mpfr_func = mpfr_acosh;                                \
 break;                                                 \
 case OP_ACOSHF:                                        \
 plain_opname = "hyperbolic arc-cosine (float)";        \
 op_symbol = "acoshf";                                  \
 mpfr_func = mpfr_acosh;                                \
 break;                                                 \
 case OP_ATANH:                                         \
 plain_opname = "hyperbolic arc-tangent";               \
 op_symbol = "atanh";                                   \
 mpfr_func = mpfr_atanh;                                \
 break;                                                 \
 case OP_ATANHF:                                        \
 plain_opname = "hyperbolic arc-tangent (float)";       \
 op_symbol = "atanhf";                                  \
 mpfr_func = mpfr_atanh;                                \
 break;                                                 \
  default:                                              \
    return;                                             \
  }

// Same as above, but for those that don't need a rounding mode.
#define GET_UNARY_OPS_NOROUND_INFO(op)          \
  switch(op){                                   \
  case OP_CEIL:                                 \
    plain_opname = "ceiling";                   \
    op_symbol = "ceilf";                        \
    mpfr_func = mpfr_ceil;                      \
    break;                                      \
  case OP_CEILF:                                \
    plain_opname = "ceiling (float)";           \
    op_symbol = "ceilf";                        \
    mpfr_func = mpfr_ceil;                      \
    break;                                      \
  default:                                      \
    return;                                     \
  }                                             \

// Same as above, but binary ops.
#define GET_BINARY_OPS_INFO(op)                 \
  switch(op){                                           \
  case OP_MOD:                                          \
    plain_opname = "modulus";                           \
    op_symbol = "mod";                                  \
    mpfr_func = mpfr_fmod;                              \
    break;                                              \
  case OP_POW:                                          \
    plain_opname = "power";                             \
    op_symbol = "pow";                                  \
    mpfr_func = mpfr_pow;                               \
    break;                                              \
  case OP_ATAN2:                                        \
    plain_opname = "arctangent (two arguments)";        \
    op_symbol = "atan2";                                \
    mpfr_func = mpfr_atan2;                             \
    break;                                              \
  case OP_ATAN2F:                                       \
    plain_opname = "arctangent (two arguments, float)"; \
    op_symbol = "atan2f";                               \
    mpfr_func = mpfr_atan2;                             \
    break;                                              \
  case OP_HYPOT:                                        \
    plain_opname = "hypotenuse";                        \
    op_symbol = "hypot";                                \
    mpfr_func = mpfr_hypot;                             \
    break;                                              \
  case OP_COPYSIGN:                                     \
    plain_opname = "copy sign";                         \
    op_symbol = "copysign";                             \
    mpfr_func = mpfr_copysign;                          \
    break;                                              \
  case OP_COPYSIGNF:                                    \
    plain_opname = "copy sign (float)";                 \
    op_symbol = "copysignf";                            \
    mpfr_func = mpfr_copysign;                          \
    break;                                              \
  default:                                              \
    return;                                             \
  }

// call the wrapping macro, defined at the call site in hg_mathwrap.c,
// to wrap each function we support.
#define WRAP_UNARY_OPS                          \
  WRAP_UNARY(sqrt, OP_SQRT);                  \
  WRAP_UNARY(exp, OP_EXP);                    \
  WRAP_UNARY(log, OP_LOG);                    \
  WRAP_UNARY(fabs, OP_ABS);                   \
  WRAP_UNARY(expm1, OP_EXPM1);                \
  WRAP_UNARY(log1p, OP_LOG1P);                \
  WRAP_UNARY(cbrt, OP_CBRT);                  \
  WRAP_UNARY(cbrtf, OP_CBRTF);                \
  WRAP_UNARY(ceil, OP_CEIL);                  \
  WRAP_UNARY(ceilf, OP_CEILF);                \
                                                \
  WRAP_UNARY(erf, OP_ERF);                    \
  WRAP_UNARY(erff, OP_ERFF);                  \
  WRAP_UNARY(erfc, OP_ERFC);                  \
  WRAP_UNARY(erfcf, OP_ERFCF);                \
                                                \
  WRAP_UNARY(cos, OP_COS);                    \
  WRAP_UNARY(cosf, OP_COSF);                  \
  WRAP_UNARY(sin, OP_SIN);                    \
  WRAP_UNARY(sinf, OP_SINF);                  \
  WRAP_UNARY(tan, OP_TAN);                    \
  WRAP_UNARY(tanf, OP_TANF);                  \
  WRAP_UNARY(asin, OP_ASIN);                  \
  WRAP_UNARY(asinf, OP_ASINF);                \
  WRAP_UNARY(acos, OP_ACOS);                  \
  WRAP_UNARY(acosf, OP_ACOSF);                \
  WRAP_UNARY(atan, OP_ATAN);                  \
  WRAP_UNARY(atanf, OP_ATANF);                \
                                                \
  WRAP_UNARY(sinh, OP_SINH);                  \
  WRAP_UNARY(sinhf, OP_SINHF);                \
  WRAP_UNARY(cosh, OP_COSH);                  \
  WRAP_UNARY(coshf, OP_COSHF);                \
  WRAP_UNARY(tanh, OP_TANH);                  \
  WRAP_UNARY(tanhf, OP_TANHF);                \
  WRAP_UNARY(asinh, OP_ASINH);                \
  WRAP_UNARY(asinhf, OP_ASINHF);              \
  WRAP_UNARY(acosh, OP_ACOSH);                \
  WRAP_UNARY(acoshf, OP_ACOSHF);              \
  WRAP_UNARY(atanh, OP_ATANH);                \
  WRAP_UNARY(atanhf, OP_ATANHF);

// Same for binary ops.
#define WRAP_BINARY_OPS                         \
  WRAP_BINARY(fmod, OP_MOD);                    \
  WRAP_BINARY(pow, OP_POW);                     \
  WRAP_BINARY(atan2, OP_ATAN2);                 \
  WRAP_BINARY(hypot, OP_HYPOT);                 \
  WRAP_BINARY(copysign, OP_COPYSIGN);           \
  WRAP_BINARY(copysignf, OP_COPYSIGNF);         \

// Finally, define an enum for the operations we support.
typedef enum {
  // Unary functions
  UNARY_OPS_LIST,
  // Binary
  BINARY_OPS_LIST
} OpType;

#endif
