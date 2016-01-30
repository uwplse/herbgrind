#ifndef _HG_MATHREPLACE_FUNCS
#define _HG_MATHREPLACE_FUNCS

// These operations are based on the ones that Herbie supports natively.

typedef enum {
  // Unary functions
  OP_SQRT,
  OP_EXP,
  OP_LOG,
  OP_COS,
  OP_SIN,
  OP_TAN,
  /* OP_COTAN, */ // Looks like libm doesn't have this one.
  OP_ASIN,
  OP_ACOS,
  OP_ATAN,
  OP_SINH,
  OP_COSH,
  OP_TANH,
  OP_ABS,
  OP_EXPM1,
  OP_LOG1P,

  // Binary
  OP_MOD,
  OP_POW,
  OP_ATAN2,
  OP_HYPOT
} OpType;

#endif
