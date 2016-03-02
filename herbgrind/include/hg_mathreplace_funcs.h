#ifndef _HG_MATHREPLACE_FUNCS
#define _HG_MATHREPLACE_FUNCS

// These operations are based on the ones that Herbie supports natively.

typedef enum {
  // Unary functions
  OP_SQRT,
  OP_EXP,
  OP_LOG,
  OP_ABS,
  OP_EXPM1,
  OP_LOG1P,
  OP_CBRT,
  OP_CBRTF,
  OP_CEIL,
  OP_CEILF,

  OP_COS,
  OP_COSF,
  OP_SIN,
  OP_SINF,
  OP_TAN,
  OP_TANF,
  /* OP_COTAN, */ // Looks like libm doesn't have this one.
  OP_ASIN,
  OP_ASINF,
  OP_ACOS,
  OP_ACOSF,
  OP_ATAN,
  OP_ATANF,

  OP_SINH,
  OP_SINHF,
  OP_COSH,
  OP_COSHF,
  OP_TANH,
  OP_TANHF,
  OP_ASINH,
  OP_ASINHF,
  OP_ACOSH,
  OP_ACOSHF,
  OP_ATANH,
  OP_ATANHF,

  // Binary
  OP_MOD,
  OP_POW,
  OP_ATAN2,
  OP_ATAN2F,
  OP_HYPOT,

  OP_COPYSIGN,
  OP_COPYSIGNF,
} OpType;

#endif
