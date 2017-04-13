/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie               realop.c ---*/
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

#include "realop.h"
#include "../value-shadowstate/real.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "../../helper/ir-info.h"

void execRealOp(IROp op_code, Real* result, ShadowValue** args){
  switch((int)op_code){
  case Iop_RecipEst32Fx4:
  case Iop_RecipEst32Fx2:
  case Iop_RecipEst64Fx2:
  case Iop_RecipEst32F0x4:
    CALL1(recip, (*result)->RVAL, args[0]->real->RVAL);
    break;
  case Iop_RSqrtEst32Fx4:
  case Iop_RSqrtEst32F0x4:
  case Iop_RSqrtEst64Fx2:
  case Iop_RSqrtEst32Fx2:
  case Iop_RSqrtEst5GoodF64:
    CALL1(rec_sqrt, (*result)->RVAL, args[0]->real->RVAL);
    break;
  case Iop_Abs32Fx4:
  case Iop_Abs32Fx2:
  case Iop_Abs64Fx2:
  case Iop_AbsF32:
  case Iop_AbsF64:
    CALL1(abs, (*result)->RVAL, args[0]->real->RVAL);
    break;
  case Iop_Neg32Fx4:
  case IEop_Neg32F0x4:
  case Iop_Neg32Fx2:
  case Iop_Neg64Fx2:
  case IEop_Neg64F0x2:
  case Iop_NegF32:
  case Iop_NegF64:
    CALL1(neg, (*result)->RVAL, args[0]->real->RVAL);
    break;
  case Iop_SinF64:
    #ifndef USE_MPFR
    tl_assert2(0, "GMP doesn't support shadowing the Sin64 instruction");
    #else
    CALL1(sin, (*result)->RVAL, args[0]->real->RVAL);
    #endif
    break;
  case Iop_CosF64:
    #ifndef USE_MPFR
    tl_assert2(0, "GMP doesn't support shadowing the Cos64 instruction");
    #else
    CALL1(cos, (*result)->RVAL, args[0]->real->RVAL);
    #endif
    break;
  case Iop_TanF64:
    #ifndef USE_MPFR
    tl_assert2(0, "GMP doesn't support shadowing the Tan64 instruction");
    #else
    CALL1(tan, (*result)->RVAL, args[0]->real->RVAL);
    #endif
    break;
  case Iop_2xm1F64:
    #ifndef USE_MPFR
    tl_assert2(0, "GMP doesn't support shadowing the 2xm164 instruction");
    #else
    CALL1(2xm1, (*result)->RVAL, args[0]->real->RVAL);
    #endif
    break;
  case Iop_SqrtF64:
  case Iop_SqrtF32:
  case Iop_Sqrt32F0x4:
  case Iop_Sqrt64F0x2:
  case Iop_Sqrt64Fx2:
    if (GETD(args[0]->real->RVAL) >= 0.0){
      CALL1(sqrt, (*result)->RVAL, args[0]->real->RVAL);
    }
    break;
  case Iop_RecpExpF64:
  case Iop_RecpExpF32:
    #ifndef USE_MPFR
    tl_assert2(0,
               "GMP doesn't support shadowing the recpexp instructions");
    #else
    CALL1(recp_exp, (*result)->RVAL, args[0]->real->RVAL);
    #endif
    break;
    // Binary Ops
  case Iop_RecipStep32Fx4:
  case Iop_RecipStep32Fx2:
  case Iop_RecipStep64Fx2:
    CALL2(recip_step, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    break;
  case Iop_RSqrtStep32Fx4:
  case Iop_RSqrtStep32Fx2:
  case Iop_RSqrtStep64Fx2:
    CALL2(recip_sqrt_step, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    break;
  case Iop_Add64Fx4:
  case Iop_Add64Fx2:
  case Iop_Add64F0x2:
  case Iop_Add32F0x4:
  case Iop_Add32Fx2:
  case Iop_Add32Fx4:
  case Iop_Add32Fx8:
  case Iop_AddF128:
  case Iop_AddF64:
  case Iop_AddF32:
  case Iop_AddF64r32:
    CALL2(add, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    break;
  case Iop_Sub64F0x2:
  case Iop_Sub32F0x4:
  case Iop_Sub32Fx2:
  case Iop_Sub32Fx8:
  case Iop_Sub64Fx4:
  case Iop_Sub32Fx4:
  case Iop_Sub64Fx2:
  case Iop_SubF128:
  case Iop_SubF32:
  case Iop_SubF64:
  case Iop_SubF64r32:
    CALL2(sub, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    break;
  case Iop_Mul32F0x4:
  case Iop_Mul64F0x2:
  case Iop_Mul32Fx8:
  case Iop_Mul64Fx4:
  case Iop_Mul32Fx4:
  case Iop_Mul64Fx2:
  case Iop_MulF128:
  case Iop_MulF64:
  case Iop_MulF32:
  case Iop_MulF64r32:
    CALL2(mul, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    break;
  case Iop_Div32F0x4:
  case Iop_Div64F0x2:
  case Iop_Div32Fx8:
  case Iop_Div64Fx4:
  case Iop_Div32Fx4:
  case Iop_DivF128:
  case Iop_DivF64:
  case Iop_DivF32:
  case Iop_DivF64r32:
  case Iop_Div64Fx2:
    if (GETD(args[1]->real->RVAL) != 0.0){
      CALL2(div, (*result)->RVAL,
            args[0]->real->RVAL,
            args[1]->real->RVAL);
    }
    break;
  case Iop_Max64F0x2:
  case Iop_Max64Fx2:
  case Iop_Max32F0x4:
  case Iop_Max32Fx4:
  case Iop_Max32Fx2:
    CALL2(max, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    break;
  case Iop_Min64F0x2:
  case Iop_Min64Fx2:
  case Iop_Min32F0x4:
  case Iop_Min32Fx4:
  case Iop_Min32Fx2:
    CALL2(min, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    break;
  /* case Iop_XorV128: */
  case Iop_AtanF64:
    #ifndef USE_MPFR
    tl_assert2(0, "GMP doesn't support shadwing the atan64 instruction");
    #else
    CALL2(atan2, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    #endif
    break;
  case Iop_Yl2xF64:
    #ifndef USE_MPFR
    tl_assert2(0,
               "GMP doesn't support shadowing the y12xf64 instruction");
    #else
    CALL2(yl2x, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    #endif
    break;
  case Iop_Yl2xp1F64:
    #ifndef USE_MPFR
    tl_assert2(0,
               "GMP doesn't support shadowing the t12xp1f64 instruction");
    #else
    CALL2(yl2xp, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    #endif
    break;
  case Iop_ScaleF64:
    #ifndef USE_MPFR
    tl_assert2(0,
               "GMP doesn't support shadowing the scale64 instruction");
    #else
    CALL2(scale, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL);
    #endif
    break;
    // Quadnary ops
  case Iop_MAddF32:
  case Iop_MAddF64:
  case Iop_MAddF64r32:
    CALL3(fma, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL,
          args[2]->real->RVAL);
    break;
  case Iop_MSubF32:
  case Iop_MSubF64:
  case Iop_MSubF64r32:
    CALL3(fms, (*result)->RVAL,
          args[0]->real->RVAL,
          args[1]->real->RVAL,
          args[2]->real->RVAL);
    break;
  default:
    VG_(printf)("Don't recognize (%d) ", op_code);
    ppIROp_Extended(op_code);
    VG_(printf)("\n");
    tl_assert(0);
    return;
  }
}
DEF1(recip){
  RET CALL2(ui_div, res, 1, arg);
}
DEF2(recip_step){
  CALL2(mul, res, arg1, arg2);
  RET CALL2(ui_sub, res, 2, res);
}
DEF2(recip_sqrt_step){
  CALL2(mul, res, arg1, arg2);
  CALL2(ui_sub, res, 3, res);
  RET CALL2(div_ui, res, res, 2);
}
#ifdef USE_MPFR
DEF1(2xm1){
  CALL1(exp2, res, arg);
  return CALL2(sub_d, res, res, 1.0);
}
DEF1(recp_exp){
  CALL1(neg, res, arg);
  return CALL1(exp, res, res);
}
DEF2(yl2x){
  CALL1(log2, res, arg2);
  return CALL2(mul, res, arg1, res);
}
DEF2(yl2xp){
  CALL2(add_ui, res, arg2, 1);
  CALL1(log2, res, res);
  return CALL2(mul, res, arg1, res);
}
DEF2(scale){
  mpfr_trunc(res, arg2);
  CALL1(exp2, res, res);
  return CALL2(mul, res, arg1, res);
}
DEF1(logb){
  mpfr_set_ui(res, mpfr_get_exp(arg), round);
  return 0;
}
DEF1(lgamma2){
  int _x;
  return mpfr_lgamma(res, &_x, arg, round);
}
#else
DEF3(fma){
  mpf_mul(res, arg1, arg2);
  mpf_add(res, res, arg3);
}
DEF3(fms){
  mpf_mul(res, arg1, arg2);
  mpf_sub(res, res, arg3);
}
DEF1(rec_sqrt){
  mpf_sqrt(res, arg);
  mpf_ui_div(res, 1, res);
}
DEF2(max){
  if (mpf_cmp(arg1, arg2) > 0){
    mpf_set(res, arg1);
  } else {
    mpf_set(res, arg2);
  }
}
DEF2(min){
  if (mpf_cmp(arg1, arg2) < 0){
    mpf_set(res, arg1);
  } else {
    mpf_set(res, arg2);
  }
}
#endif
