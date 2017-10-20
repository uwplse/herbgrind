/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie              ir-info.c ---*/
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

#include "ir-info.h"

#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "../instrument/floattypes.h"
#include <math.h>

int numChannelsOut(IROp op_code){
  switch(op_code){
  case Iop_V128to32:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_V128to64:
  case Iop_V128HIto64:
    return 1;
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
    return 2;
  default:
    return numChannelsIn(op_code);
  }
}

int numChannelsIn(IROp op_code){
  switch((int)op_code){
  case Iop_RecipEst32Fx4:
  case Iop_RSqrtEst32Fx4:
  case Iop_Abs32Fx4:
  case Iop_Neg32Fx4:
  case IEop_Neg32F0x4:
  case Iop_ZeroHI96ofV128:
  case Iop_V128to32:
  case Iop_Sqrt32F0x4:
  case Iop_RecipEst32F0x4:
  case Iop_RSqrtEst32F0x4:
    return 4;
  case Iop_RecipEst64Fx2:
  case Iop_RSqrtEst64Fx2:
  case Iop_Abs64Fx2:
  case Iop_Neg64Fx2:
  case IEop_Neg64F0x2:
  case Iop_Neg32Fx2:
  case Iop_Abs32Fx2:
  case Iop_RSqrtEst32Fx2:
  case Iop_RecipEst32Fx2:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_Sqrt64F0x2:
    return 2;
  case Iop_RSqrtEst5GoodF64:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F32toF64:
  case Iop_NegF32:
  case Iop_AbsF32:
  case Iop_NegF64:
  case Iop_AbsF64:
  case Iop_64UtoV128:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
    return 1;
    // Binary Ops
  case Iop_RecipStep32Fx4:
  case Iop_RSqrtStep32Fx4:
  case Iop_Add32F0x4:
  case Iop_Sub32F0x4:
  case Iop_Mul32F0x4:
  case Iop_Div32F0x4:
  case Iop_Max32F0x4:
  case Iop_Min32F0x4:
  case Iop_Min32Fx4:
  case Iop_Max32Fx4:
  case Iop_SetV128lo32:
  case Iop_CmpLT32F0x4:
  case Iop_CmpUN32F0x4:
    return 4;
  case Iop_Add32Fx2:
  case Iop_Sub32Fx2:
  case Iop_Sqrt64Fx2:
  case Iop_RecipStep32Fx2:
  case Iop_RSqrtStep32Fx2:
  case Iop_RecipStep64Fx2:
  case Iop_RSqrtStep64Fx2:
  case Iop_Mul64F0x2:
  case Iop_Div64F0x2:
  case Iop_Sub64F0x2:
  case Iop_Add64F0x2:
  case Iop_Min64F0x2:
  case Iop_Max64F0x2:
  /* case Iop_XorV128: */
  case Iop_CmpLT64F0x2:
  case Iop_CmpLE64F0x2:
  case Iop_CmpUN64F0x2:
    return 2;
  case Iop_RecpExpF64:
  case Iop_RecpExpF32:
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
  case Iop_SinF64:
  case Iop_CosF64:
  case Iop_TanF64:
  case Iop_2xm1F64:
  case Iop_SqrtF64:
  case Iop_SqrtF32:
  case Iop_CmpF64:
  case Iop_CmpF32:
    return 1;
    // Ternary Ops
  case Iop_Add32Fx8:
  case Iop_Sub32Fx8:
  case Iop_Mul32Fx8:
  case Iop_Div32Fx8:
    return 8;
  case Iop_Add64Fx4:
  case Iop_Sub64Fx4:
  case Iop_Mul64Fx4:
  case Iop_Div64Fx4:
  case Iop_Add32Fx4:
  case Iop_Sub32Fx4:
  case Iop_Mul32Fx4:
  case Iop_Div32Fx4:
    return 4;
  case Iop_Add64Fx2:
  case Iop_Sub64Fx2:
  case Iop_Mul64Fx2:
  case Iop_Div64Fx2:
    return 2;
  case Iop_AtanF64:
  case Iop_Yl2xF64:
  case Iop_Yl2xp1F64:
  case Iop_ScaleF64:
  case Iop_AddF128:
  case Iop_SubF128:
  case Iop_MulF128:
  case Iop_DivF128:
  case Iop_AddF64:
  case Iop_SubF64:
  case Iop_MulF64:
  case Iop_DivF64:
  case Iop_AddF32:
  case Iop_SubF32:
  case Iop_MulF32:
  case Iop_DivF32:
  case Iop_AddF64r32:
  case Iop_SubF64r32:
  case Iop_MulF64r32:
  case Iop_DivF64r32:
    return 1;
    // Quadnary ops
  case Iop_MAddF32:
  case Iop_MSubF32:
  case Iop_MAddF64:
  case Iop_MSubF64:
  case Iop_MAddF64r32:
  case Iop_MSubF64r32:
    return 1;
  case Iop_SetV128lo64:
    tl_assert2(0, "Couldn't infer type!\n");
  default:
    ppIROp(op_code);
    tl_assert(0);
    return 0;
  }
}

// The number of SIMD operands that the operation operates on. That
// means that operations that take a simd value with four channels,
// and only operate on the first one, return 1.
int numSIMDOperands(IROp op_code){
  switch((int)op_code){
  case Iop_RecipEst32Fx4:
  case Iop_RSqrtEst32Fx4:
  case Iop_Abs32Fx4:
  case Iop_Neg32Fx4:
    return 4;
  case Iop_RecipEst64Fx2:
  case Iop_RSqrtEst64Fx2:
  case Iop_Abs64Fx2:
  case Iop_Neg64Fx2:
  case Iop_RSqrtEst32Fx2:
  case Iop_RecipEst32Fx2:
    return 2;
  case Iop_RSqrtEst5GoodF64:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_F32toF64:
  case Iop_NegF32:
  case IEop_Neg32F0x4:
  case Iop_AbsF32:
  case Iop_NegF64:
  case IEop_Neg64F0x2:
  case Iop_AbsF64:
  case Iop_Sqrt64F0x2:
  case Iop_RecipEst32F0x4:
  case Iop_Sqrt32F0x4:
  case Iop_RSqrtEst32F0x4:
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_64UtoV128:
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
    return 1;
    // Binary Ops
  case Iop_RecipStep32Fx4:
  case Iop_RSqrtStep32Fx4:
  case Iop_Max32Fx4:
  case Iop_Min32Fx4:
    return 4;
  case Iop_Add32Fx2:
  case Iop_Sub32Fx2:
  case Iop_Sqrt64Fx2:
  case Iop_RecipStep32Fx2:
  case Iop_RSqrtStep32Fx2:
  case Iop_RecipStep64Fx2:
  case Iop_RSqrtStep64Fx2:
  case Iop_Neg32Fx2:
  case Iop_Abs32Fx2:
  case Iop_Max32Fx2:
  case Iop_Min32Fx2:
  case Iop_Max64Fx2:
  case Iop_Min64Fx2:
    return 2;
  case Iop_Add32F0x4:
  case Iop_Sub32F0x4:
  case Iop_Mul32F0x4:
  case Iop_Div32F0x4:
  case Iop_RecpExpF64:
  case Iop_RecpExpF32:
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
  case Iop_SinF64:
  case Iop_CosF64:
  case Iop_TanF64:
  case Iop_2xm1F64:
  case Iop_SqrtF64:
  case Iop_SqrtF32:
  case Iop_Mul64F0x2:
  case Iop_Div64F0x2:
  case Iop_XorV128:
  case Iop_Sub64F0x2:
  case Iop_Add64F0x2:
  case Iop_Max64F0x2:
  case Iop_Max32F0x4:
  case Iop_Min64F0x2:
  case Iop_Min32F0x4:
  case Iop_CmpF64:
  case Iop_CmpF32:
  case Iop_CmpLT32F0x4:
  case Iop_CmpLT64F0x2:
  case Iop_CmpLE64F0x2:
  case Iop_CmpUN64F0x2:
  case Iop_CmpUN32F0x4:
    return 1;
    // Ternary Ops
  case Iop_Add32Fx8:
  case Iop_Sub32Fx8:
  case Iop_Mul32Fx8:
  case Iop_Div32Fx8:
    return 8;
  case Iop_Add64Fx4:
  case Iop_Sub64Fx4:
  case Iop_Mul64Fx4:
  case Iop_Div64Fx4:
  case Iop_Add32Fx4:
  case Iop_Sub32Fx4:
  case Iop_Mul32Fx4:
  case Iop_Div32Fx4:
    return 4;
  case Iop_Add64Fx2:
  case Iop_Sub64Fx2:
  case Iop_Mul64Fx2:
  case Iop_Div64Fx2:
    return 2;
  case Iop_AtanF64:
  case Iop_Yl2xF64:
  case Iop_Yl2xp1F64:
  case Iop_ScaleF64:
  case Iop_AddF128:
  case Iop_SubF128:
  case Iop_MulF128:
  case Iop_DivF128:
  case Iop_AddF64:
  case Iop_SubF64:
  case Iop_MulF64:
  case Iop_DivF64:
  case Iop_AddF32:
  case Iop_SubF32:
  case Iop_MulF32:
  case Iop_DivF32:
  case Iop_AddF64r32:
  case Iop_SubF64r32:
  case Iop_MulF64r32:
  case Iop_DivF64r32:
    return 1;
    // Quadnary ops
  case Iop_MAddF32:
  case Iop_MSubF32:
  case Iop_MAddF64:
  case Iop_MSubF64:
  case Iop_MAddF64r32:
  case Iop_MSubF64r32:
    return 1;
  default:
    tl_assert(0);
    return 0;
  }
}

int inferOtherNumChannels(int inferIndex, IRExpr* arg, IROp op_code){
  switch(op_code){
  case Iop_SetV128lo64:
    tl_assert(arg->tag == Iex_RdTmp);
    ValueType otherArgType = tempType(arg->Iex.RdTmp.tmp);
    if (otherArgType == Vt_Single){
      if (inferIndex == 0){
        return 4;
      } else {
        return 2;
      }
    } else {
      if (inferIndex == 0){
        return 2;
      } else {
        return 1;
      }
    }
    break;
  case Iop_SetV128lo32:
    if (inferIndex == 0){
      return 4;
    } else {
      return 1;
    }
    break;
  default:
    return numChannelsIn(op_code);
  }
}

const char* getOpcodeSymbol(IROp op_code){
  switch((int)op_code){
  case Iop_RecipEst32Fx4:
  case Iop_RecipEst64Fx2:
  case Iop_RecipEst32Fx2:
  case Iop_RecipEst32F0x4:
    return "recip-est";
  case Iop_RecipStep32Fx4:
  case Iop_RecipStep32Fx2:
  case Iop_RecipStep64Fx2:
    return "recip-step";
  case Iop_RSqrtEst32Fx4:
  case Iop_RSqrtEst64Fx2:
  case Iop_RSqrtEst32Fx2:
  case Iop_RSqrtEst5GoodF64:
  case Iop_RSqrtEst32F0x4:
    return "rsqrt-est";
  case Iop_RSqrtStep32Fx4:
  case Iop_RSqrtStep32Fx2:
  case Iop_RSqrtStep64Fx2:
    return "rsqrt-step";
  case Iop_Abs32Fx4:
  case Iop_Abs64Fx2:
  case Iop_AbsF32:
  case Iop_AbsF64:
  case Iop_Abs32Fx2:
    return "abs";
  case Iop_Neg32Fx4:
  case IEop_Neg64F0x2:
  case Iop_Neg64Fx2:
  case Iop_NegF32:
  case Iop_NegF64:
  case IEop_Neg32F0x4:
  case Iop_Neg32Fx2:
    return "-";
  case Iop_Sqrt64F0x2:
  case Iop_Sqrt32F0x4:
    return "sqrt";
    // Binary Ops
  case Iop_Max32Fx4:
  case Iop_Max32Fx2:
  case Iop_Max64Fx2:
  case Iop_Max64F0x2:
  case Iop_Max32F0x4:
    return "max";
  case Iop_Min32Fx4:
  case Iop_Min32Fx2:
  case Iop_Min64Fx2:
  case Iop_Min64F0x2:
  case Iop_Min32F0x4:
    return "min";
  case Iop_Add32Fx2:
  case Iop_Add32F0x4:
  case Iop_Add64F0x2:
  case Iop_Add32Fx8:
  case Iop_Add64Fx4:
  case Iop_Add32Fx4:
  case Iop_Add64Fx2:
  case Iop_AddF128:
  case Iop_AddF64:
  case Iop_AddF32:
  case Iop_AddF64r32:
    return "+";
  case Iop_Sub32Fx2:
  case Iop_Sub32F0x4:
  case Iop_Sub64F0x2:
  case Iop_Sub32Fx8:
  case Iop_Sub64Fx4:
  case Iop_Sub32Fx4:
  case Iop_Sub64Fx2:
  case Iop_SubF128:
  case Iop_SubF64:
  case Iop_SubF32:
  case Iop_SubF64r32:
    return "-";
  case Iop_Sqrt64Fx2:
  case Iop_SqrtF64:
  case Iop_SqrtF32:
    return "sqrt";
  case Iop_Mul32F0x4:
  case Iop_Mul64F0x2:
  case Iop_Mul32Fx8:
  case Iop_Mul64Fx4:
  case Iop_Mul32Fx4:
  case Iop_Mul64Fx2:
  case Iop_MulF64:
  case Iop_MulF128:
  case Iop_MulF32:
  case Iop_MulF64r32:
    return "*";
  case Iop_Div32F0x4:
  case Iop_Div64F0x2:
  case Iop_Div32Fx8:
  case Iop_Div64Fx4:
  case Iop_Div32Fx4:
  case Iop_Div64Fx2:
  case Iop_DivF128:
  case Iop_DivF64:
  case Iop_DivF32:
  case Iop_DivF64r32:
    return "/";
  case Iop_RecpExpF64:
  case Iop_RecpExpF32:
    return "recp-exp";
  case Iop_SinF64:
    return "sin";
  case Iop_CosF64:
    return "cos";
  case Iop_TanF64:
    return "tan";
  case Iop_AtanF64:
    return "atan";
  case Iop_2xm1F64:
    return "2xm1";
  case Iop_Yl2xF64:
    return "y12x";
  case Iop_Yl2xp1F64:
    return "y12xp";
  case Iop_XorV128:
    return "xor";
  case Iop_ScaleF64:
    return "scale";
    // Quadnary ops
  case Iop_MAddF32:
  case Iop_MAddF64:
  case Iop_MAddF64r32:
    return "fma";
  case Iop_MSubF32:
  case Iop_MSubF64:
  case Iop_MSubF64r32:
    return "fms";
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_64UtoV128:
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_F32toF64:
  default:
    tl_assert(0);
    return 0;
  }
}

double runEmulatedOp(IROp op_code, double* args){
  switch((int)op_code){
  case Iop_Abs32Fx4:
  case Iop_Abs32Fx2:
  case Iop_AbsF32:
    return fabsf(args[0]);
  case Iop_Abs64Fx2:
  case Iop_AbsF64:
    return fabs(args[0]);
  case IEop_Neg32F0x4:
  case Iop_Neg32Fx4:
  case Iop_Neg32Fx2:
  case Iop_NegF32:
    return -(float)args[0];
  case Iop_Neg64Fx2:
  case IEop_Neg64F0x2:
  case Iop_NegF64:
    return -args[0];
  case Iop_Sqrt32F0x4:
  case Iop_SqrtF32:
    return sqrtf(args[0]);
  case Iop_Sqrt64F0x2:
  case Iop_Sqrt64Fx2:
  case Iop_SqrtF64:
    return sqrt(args[0]);
    // Binary Ops
  case Iop_Max32Fx4:
  case Iop_Max32Fx2:
  case Iop_Max32F0x4:
    return fmaxf(args[0], args[1]);
  case Iop_Max64Fx2:
  case Iop_Max64F0x2:
    return fmax(args[0], args[1]);
  case Iop_Min32Fx4:
  case Iop_Min32Fx2:
  case Iop_Min32F0x4:
    return fminf(args[0], args[1]);
  case Iop_Min64Fx2:
  case Iop_Min64F0x2:
    return fmin(args[0], args[1]);
  case Iop_Add32Fx8:
  case Iop_Add32F0x4:
  case Iop_Add32Fx4:
  case Iop_Add32Fx2:
  case Iop_AddF32:
    return ((float)args[0]) + ((float)(args[1]));
  case Iop_Add64F0x2:
  case Iop_Add64Fx4:
  case Iop_Add64Fx2:
  case Iop_AddF64:
  case Iop_AddF64r32:
    return args[0] + args[1];
  case Iop_SubF32:
  case Iop_Sub32Fx2:
  case Iop_Sub32F0x4:
  case Iop_Sub32Fx4:
  case Iop_Sub32Fx8:
    return ((float)args[0]) - ((float)(args[1]));
  case Iop_Sub64F0x2:
  case Iop_Sub64Fx4:
  case Iop_Sub64Fx2:
  case Iop_SubF64:
  case Iop_SubF64r32:
    return args[0] - args[1];
  case Iop_MulF32:
  case Iop_Mul32Fx4:
  case Iop_Mul32F0x4:
  case Iop_Mul32Fx8:
    return ((float)args[0]) * ((float)(args[1]));
  case Iop_Mul64F0x2:
  case Iop_Mul64Fx4:
  case Iop_Mul64Fx2:
  case Iop_MulF64:
  case Iop_MulF64r32:
    return args[0] * args[1];
  case Iop_Div32F0x4:
  case Iop_Div32Fx4:
  case Iop_Div32Fx8:
  case Iop_DivF32:
    return ((float)args[0]) / ((float)args[1]);
  case Iop_Div64F0x2:
  case Iop_Div64Fx4:
  case Iop_Div64Fx2:
  case Iop_DivF64:
  case Iop_DivF64r32:
    return args[0] / args[1];
  case Iop_SinF64:
    return sin(args[0]);
  case Iop_CosF64:
    return cos(args[0]);
  case Iop_TanF64:
    return tan(args[0]);
  case Iop_AtanF64:
    return atan(args[0]/args[1]);
  case Iop_MAddF32:
    return fmaf(args[0], args[1], args[2]);
  case Iop_MAddF64:
  case Iop_MAddF64r32:
    return fma(args[0], args[1], args[2]);
  case Iop_MSubF32:
    return fmaf(args[0], args[1], -args[2]);
  case Iop_MSubF64:
  case Iop_MSubF64r32:
    return fma(args[0], args[1], -args[2]);
  case Iop_ScaleF64:
  case Iop_XorV128:
  case Iop_Yl2xp1F64:
  case Iop_Yl2xF64:
  case Iop_2xm1F64:
  case Iop_RecpExpF64:
  case Iop_RecpExpF32:
  case Iop_RSqrtStep32Fx4:
  case Iop_RSqrtStep32Fx2:
  case Iop_RSqrtStep64Fx2:
  case Iop_RSqrtEst32Fx4:
  case Iop_RSqrtEst64Fx2:
  case Iop_RSqrtEst32Fx2:
  case Iop_RSqrtEst5GoodF64:
  case Iop_RSqrtEst32F0x4:
  case Iop_RecipStep32Fx4:
  case Iop_RecipStep32Fx2:
  case Iop_RecipStep64Fx2:
  case Iop_RecipEst32Fx4:
  case Iop_RecipEst64Fx2:
  case Iop_RecipEst32Fx2:
  case Iop_RecipEst32F0x4:
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_64UtoV128:
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_F32toF64:
  default:
    tl_assert(0);
    return 0;
  }
}

void ppIROp_Extended(int op_code){
  switch(op_code){
  case IEop_Neg32F0x4:
    VG_(printf)("IEop_Neg32F0x4");
    break;
  case IEop_Neg64F0x2:
    VG_(printf)("IEop_Neg64F0x2");
    break;
  default:
    ppIROp(op_code);
    break;
  }
}
