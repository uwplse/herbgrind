/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie             mathwrap.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017 Alex Sanchez-Stern

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include "pub_tool_clreq.h"
#include "pub_tool_redir.h"

#include <stdio.h>
#include <complex.h>

#include "../include/herbgrind.h"
#include "../include/mathreplace-funcs.h"

#define LIBM libmZdsoZa
#define LIBM_CPP libmZhZaZdsoZa
#define LIBGCC libgccZa

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

#define WRAP_UNARY_64_(soname, fnname, opname)             \
  double VG_REPLACE_FUNCTION_ZU(soname, fnname)(double x);        \
  double VG_REPLACE_FUNCTION_ZU(soname, fnname)(double x){        \
    double result;                                              \
    double args[1];                                             \
    args[0] = x;                                              \
    HERBGRIND_PERFORM_OP(opname, &result, args);              \
    return result;                                            \
  }
#define WRAP_UNARY_32_(soname, fnname, opname)             \
  float VG_REPLACE_FUNCTION_ZU(soname, fnname)(float x);        \
  float VG_REPLACE_FUNCTION_ZU(soname, fnname)(float x){        \
    float result;                                              \
    float args[1];                                             \
    args[0] = x;                                              \
    HERBGRIND_PERFORM_OPF(opname, &result, args);              \
    return result;                                            \
  }

#define WRAP_UNARY_64(fnname, opname)         \
  WRAP_UNARY_64_(LIBM, fnname, opname)        \
  WRAP_UNARY_64_(LIBM_CPP, fnname, opname)    \
  WRAP_UNARY_64_(NONE, fnname, opname)
#define WRAP_UNARY_32(fnname, opname)         \
  WRAP_UNARY_32_(LIBM, fnname, opname)        \
  WRAP_UNARY_32_(LIBM_CPP, fnname, opname)    \
  WRAP_UNARY_32_(NONE, fnname, opname)

#define WRAP_UNARY_COMPLEX_64_(soname, fnname, opname)              \
  complex double VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex double x); \
  complex double VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex double x){ \
    double rResult, iResult;                                             \
    double args[2];                                                       \
    args[0] = creal(x);                                                 \
    args[1] = cimag(x);                                                 \
    HERBGRIND_PERFORM_OP(opname##R, &rResult, args);                    \
    HERBGRIND_PERFORM_OP(opname##I, &iResult, args);                    \
    return rResult + iResult * I;                                       \
  }
#define WRAP_UNARY_COMPLEX_32_(soname, fnname, opname)            \
  complex float VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex float x); \
  complex float VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex float x){ \
    double rResult, iResult;                                             \
    double args[2];                                                       \
    args[0] = creal(x);                                                 \
    args[1] = cimag(x);                                                 \
    HERBGRIND_PERFORM_OPF(opname##R, &rResult, args);                    \
    HERBGRIND_PERFORM_OPF(opname##I, &iResult, args);                    \
    return rResult + iResult * I;                                       \
  }

#define WRAP_UNARY_COMPLEX_64(fnname, opname)      \
  WRAP_UNARY_COMPLEX_64_(LIBM, fnname, opname)
#define WRAP_UNARY_COMPLEX_64_BUILTIN(fnname, opname)   \
  WRAP_UNARY_COMPLEX_64_(LIBGCC, fnname, opname)
#define WRAP_UNARY_COMPLEX_32(fnname, opname)      \
  WRAP_UNARY_COMPLEX_32_(LIBM, fnname, opname)
#define WRAP_UNARY_COMPLEX_32_BUILTIN(fnname, opname)   \
  WRAP_UNARY_COMPLEX_32_(LIBGCC, fnname, opname)

// This macro is defined in include/hg_mathreplace_funcs.h, and
// invokes the above macro for each unary operation that needs to be
// wrapped.
#ifndef DONT_WRAP
WRAP_UNARY_OPS
#endif

/* float VG_REPLACE_FUNCTION_ZU(LIBM, powf)(float x, float y); */
/* float VG_REPLACE_FUNCTION_ZU(LIBM, powf)(float x, float y){ */
/*   float result; */
/*   float* resultAddr; */
/*   float args[2]; */
/*   args[0] = x; */
/*   args[1] = y; */
/*   unsigned int wordresult = HERBGRIND_PERFORM_OPF(OP_POWF, &result, args); */
/*   return result; */
/* } */


/*----------------------------
====== Binary Ops ============
----------------------------*/

#define WRAP_BINARY_64_(soname, fnname, opname)              \
  double VG_REPLACE_FUNCTION_ZU(soname, fnname)(double x, double y);  \
  double VG_REPLACE_FUNCTION_ZU(soname, fnname)(double x, double y){  \
    double result;                                               \
    double args[2];                                              \
    args[0] = x;                                                 \
    args[1] = y;                                                 \
    HERBGRIND_PERFORM_OP(opname, &result, args);                 \
    return result;                                               \
  }
#define WRAP_BINARY_32_(soname, fnname, opname)              \
  float VG_REPLACE_FUNCTION_ZU(soname, fnname)(float x, float y);  \
  float VG_REPLACE_FUNCTION_ZU(soname, fnname)(float x, float y){  \
    float result;                                               \
    float args[2];                                              \
    args[0] = x;                                                 \
    args[1] = y;                                                 \
    HERBGRIND_PERFORM_OPF(opname, &result, args);                 \
    return result;                                               \
  }

#define WRAP_BINARY_64(fnname, opname)                               \
  WRAP_BINARY_64_(LIBM, fnname, opname)                              \
  WRAP_BINARY_64_(LIBM_CPP, fnname, opname)                          \
  WRAP_BINARY_64_(NONE, fnname, opname)
#define WRAP_BINARY_32(fnname, opname)                               \
  WRAP_BINARY_32_(LIBM, fnname, opname)                              \
  WRAP_BINARY_32_(LIBM_CPP, fnname, opname)                          \
  WRAP_BINARY_32_(NONE, fnname, opname)

#define WRAP_BINARY_COMPLEX_64_(soname, fnname, opname)              \
  complex double VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex double x,   \
                                                      complex double y);  \
  complex double VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex double x,   \
                                                      complex double y){  \
    double rResult, iResult;                                              \
    double args[4];                                                       \
    args[0] = creal(x);                                                 \
    args[1] = cimag(x);                                                 \
    args[2] = creal(y);                                                 \
    args[3] = cimag(y);                                                 \
    HERBGRIND_PERFORM_OP(opname##R, &rResult, args);                    \
    HERBGRIND_PERFORM_OP(opname##I, &iResult, args);                    \
    return rResult + iResult * I;                                       \
  }
#define WRAP_BINARY_COMPLEX_32_(soname, fnname, opname)              \
  complex float VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex float x,   \
                                                      complex float y);  \
  complex float VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex float x,   \
                                                      complex float y){  \
    double rResult, iResult;                                              \
    double args[4];                                                       \
    args[0] = creal(x);                                                 \
    args[1] = cimag(x);                                                 \
    args[2] = creal(y);                                                 \
    args[3] = cimag(y);                                                 \
    HERBGRIND_PERFORM_OPF(opname##R, &rResult, args);                    \
    HERBGRIND_PERFORM_OPF(opname##I, &iResult, args);                    \
    return rResult + iResult * I;                                       \
  }
#define WRAP_BINARY_COMPLEX_64(fnname, opname)             \
  WRAP_BINARY_COMPLEX_64_(LIBM, fnname, opname)
#define WRAP_BINARY_COMPLEX_64_BUILTIN(fnname, opname)      \
  WRAP_BINARY_COMPLEX_64_(LIBGCC, fnname, opname)
#define WRAP_BINARY_COMPLEX_32(fnname, opname)             \
  WRAP_BINARY_COMPLEX_32_(LIBM, fnname, opname)
#define WRAP_BINARY_COMPLEX_32_BUILTIN(fnname, opname)      \
  WRAP_BINARY_COMPLEX_32_(LIBGCC, fnname, opname)
// This macro is defined in include/hg_mathreplace_funcs.h, and
// invokes the above macro for each binary operation that needs to be
// wrapped.
#ifndef DONT_WRAP
WRAP_BINARY_OPS
#endif

/*----------------------------
====== Ternary Ops ===========
----------------------------*/

#define WRAP_TERNARY_64_(soname, fnname, opname)                     \
  double VG_REPLACE_FUNCTION_ZU(soname, fnname)(double x, double y, double z);  \
  double VG_REPLACE_FUNCTION_ZU(soname, fnname)(double x, double y, double z){  \
    double result;                                                        \
    double args[3];                                                       \
    args[0] = x;                                                        \
    args[1] = y;                                                        \
    args[2] = z;                                                        \
    HERBGRIND_PERFORM_OP(opname, &result, args);                        \
    return result;                                                      \
  }
#define WRAP_TERNARY_32_(soname, fnname, opname)                     \
  float VG_REPLACE_FUNCTION_ZU(soname, fnname)(float x, float y, float z);  \
  float VG_REPLACE_FUNCTION_ZU(soname, fnname)(float x, float y, float z){  \
    float result;                                                        \
    float args[3];                                                       \
    args[0] = x;                                                        \
    args[1] = y;                                                        \
    args[2] = z;                                                        \
    HERBGRIND_PERFORM_OPF(opname, &result, args);                        \
    return result;                                                      \
  }

#define WRAP_TERNARY_64(fnname, opname)         \
  WRAP_TERNARY_64_(LIBM, fnname, opname)        \
  WRAP_TERNARY_64_(LIBM_CPP, fnname, opname)    \
  WRAP_TERNARY_64_(NONE, fnname, opname)
#define WRAP_TERNARY_32(fnname, opname)         \
  WRAP_TERNARY_32_(LIBM, fnname, opname)        \
  WRAP_TERNARY_32_(LIBM_CPP, fnname, opname)    \
  WRAP_TERNARY_32_(NONE, fnname, opname)

#define WRAP_TERNARY_COMPLEX_(prec, soname, fnname, opname)              \
  complex prec VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex prec x,   \
                                                      complex prec y,   \
                                                      complex prec z);  \
  complex prec VG_REPLACE_FUNCTION_ZU(soname, fnname)(complex prec x,   \
                                                      complex prec y,   \
                                                      complex prec z){  \
    double rResult, iResult;                                              \
    double args[6];                                                       \
    args[0] = creal(x);                                                 \
    args[1] = cimag(x);                                                 \
    args[2] = creal(x);                                                 \
    args[3] = cimag(x);                                                 \
    args[4] = creal(x);                                                 \
    args[5] = cimag(x);                                                 \
    HERBGRIND_PERFORM_OP(opname##R, &rResult, args);                    \
    HERBGRIND_PERFORM_OP(opname##I, &iResult, args);                    \
    return rResult + iResult * I;                                       \
  }

#define WRAP_TERNARY_COMPLEX_64_(soname, fnname, opname) \
  WRAP_TERNARY_COMPLEX_(double, soname, fnname, opname);
#define WRAP_TERNARY_COMPLEX_32_(soname, fnname, opname) \
  WRAP_TERNARY_COMPLEX_(float, soname, fnname, opname);

#define WRAP_TERNARY_COMPLEX_64(fnname, opname)      \
  WRAP_TERNARY_COMPLEX_64_(LIBM, fnname, opname)
#define WRAP_TERNARY_COMPLEX_64_BUILTIN(fnname, opname)   \
  WRAP_TERNARY_COMPLEX_64_(LIBGCC, fnname, opname)
#define WRAP_TERNARY_COMPLEX_32(fnname, opname)      \
  WRAP_TERNARY_COMPLEX_32_(LIBM, fnname, opname)
#define WRAP_TERNARY_COMPLEX_32_BUILTIN(fnname, opname)   \
  WRAP_TERNARY_COMPLEX_32_(LIBGCC, fnname, opname)
// This macro is defined in include/hg_mathreplace_funcs.h, and
// invokes the above macro for each ternary operation that needs to be
// wrapped.
#ifndef DONT_WRAP
WRAP_TERNARY_OPS
#endif

#ifndef DONT_WRAP
// This is a special wrap
void VG_REPLACE_FUNCTION_ZU(LIBM_CPP, sincos)(double x, double* p_sin, double* p_cos);
void VG_REPLACE_FUNCTION_ZU(LIBM_CPP, sincos)(double x, double* p_sin, double* p_cos){
  double args[1];
  args[0] = x;
  HERBGRIND_PERFORM_SPECIAL_OP(OP_SINCOS, args, p_sin, p_cos);
}
void VG_REPLACE_FUNCTION_ZU(LIBM, sincos)(double x, double* p_sin, double* p_cos);
void VG_REPLACE_FUNCTION_ZU(LIBM, sincos)(double x, double* p_sin, double* p_cos){
  double args[1];
  args[0] = x;
  HERBGRIND_PERFORM_SPECIAL_OP(OP_SINCOS, args, p_sin, p_cos);
}

complex float VG_REPLACE_FUNCTION_ZU(LIBM, __mulsc3)(complex float x, complex float y);
complex float VG_REPLACE_FUNCTION_ZU(LIBM, __mulsc3)(complex float x, complex float y){
  return x * y;
}
complex double VG_REPLACE_FUNCTION_ZU(LIBM, __muldc3)(complex double x, complex double y);
complex double VG_REPLACE_FUNCTION_ZU(LIBM, __muldc3)(complex double x, complex double y){
  return x * y;
}
complex double VG_REPLACE_FUNCTION_ZU(LIBM, __multc3)(complex double x, complex double y);
complex double VG_REPLACE_FUNCTION_ZU(LIBM, __multc3)(complex double x, complex double y){
  return x * y;
}
complex double VG_REPLACE_FUNCTION_ZU(LIBM, __mulxc3)(complex double x, complex double y);
complex double VG_REPLACE_FUNCTION_ZU(LIBM, __mulxc3)(complex double x, complex double y){
  return x * y;
}

complex float VG_REPLACE_FUNCTION_ZU(LIBM, __divsc3)(complex float x, complex float y);
complex float VG_REPLACE_FUNCTION_ZU(LIBM, __divsc3)(complex float x, complex float y){
  return x / y;
}
WRAP_BINARY_COMPLEX_64_BUILTIN(__divdc3, OP_CDIV);
WRAP_BINARY_COMPLEX_64_BUILTIN(__divtc3, OP_CDIV);
WRAP_BINARY_COMPLEX_64_BUILTIN(__divxc3, OP_CDIV);

#endif
