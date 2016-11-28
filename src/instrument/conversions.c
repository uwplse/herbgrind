/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          conversions.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016 Alex Sanchez-Stern

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

#include "conversions.h"
#include "helpers.h"
#include "../runtime/value-shadowstate/value-shadowstate.h"
#include "../runtime/shadowop/conversions.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"

void instrumentConversion(IRSB* sbOut, IROp op_code, IRExpr** argExprs,
                          IRTemp dest){
  IRTemp shadowInput1T, shadowInput2T;
  IRTemp inputPreexisting = newIRTemp(sbOut->tyenv, Ity_I1);
  IRTemp newShadowT = newIRTemp(sbOut->tyenv, Ity_I64);

  if (numConversionInputs(op_code) == 1){
    int inputIndex = conversionInputArgIndex(op_code);
    if (argExprs[inputIndex]->tag == Iex_Const) return;
    shadowInput1T = newIRTemp(sbOut->tyenv, Ity_I64);
    shadowInput2T = 0;
    addLoad64C(sbOut, shadowInput1T,
               &(shadowTemps[argExprs[inputIndex]->Iex.RdTmp.tmp]));
    addNonZeroCheck64(sbOut, inputPreexisting, shadowInput1T);
  } else {
    shadowInput1T = newIRTemp(sbOut->tyenv, Ity_I64);
    shadowInput2T = newIRTemp(sbOut->tyenv, Ity_I64);
    if (argExprs[0]->tag == Iex_Const &&
        argExprs[1]->tag == Iex_Const) return;
    if (argExprs[0]->tag == Iex_Const){
      addLoad64C(sbOut, shadowInput2T,
                 &(shadowTemps[argExprs[1]->Iex.RdTmp.tmp]));
      addNonZeroCheck64(sbOut, inputPreexisting, shadowInput2T);
      addMakeInputG(sbOut, shadowInput1T, argExprs[0],
                    inputPreexisting, conversionInputType(op_code));
    } else if (argExprs[1]->tag == Iex_Const){
      addLoad64C(sbOut, shadowInput1T,
                 &(shadowTemps[argExprs[0]->Iex.RdTmp.tmp]));
      addNonZeroCheck64(sbOut, inputPreexisting, shadowInput1T);
      addMakeInputG(sbOut, shadowInput2T, argExprs[0],
                    inputPreexisting, conversionInputType(op_code));
    } else {
      IRTemp loadedShadowInput1T = newIRTemp(sbOut->tyenv, Ity_I64);
      IRTemp loadedShadowInput2T = newIRTemp(sbOut->tyenv, Ity_I64);
      addLoad64C(sbOut, loadedShadowInput1T,
                 &(shadowTemps[argExprs[0]->Iex.RdTmp.tmp]));
      addLoad64C(sbOut, loadedShadowInput2T,
                 &(shadowTemps[argExprs[1]->Iex.RdTmp.tmp]));
      IRTemp input1Preexisting, input2Preexisting;
      input1Preexisting = newIRTemp(sbOut->tyenv, Ity_I1);
      input2Preexisting = newIRTemp(sbOut->tyenv, Ity_I1);
      addNonZeroCheck64(sbOut, input1Preexisting, loadedShadowInput1T);
      addNonZeroCheck64(sbOut, input2Preexisting, loadedShadowInput2T);
      addOr(sbOut, inputPreexisting, input1Preexisting, input2Preexisting);
      IRTemp input1Null = newIRTemp(sbOut->tyenv, Ity_I1);
      IRTemp input2Null = newIRTemp(sbOut->tyenv, Ity_I1);
      addUnop(sbOut, Iop_Not1, input1Null,
              IRExpr_RdTmp(input1Preexisting));
      addUnop(sbOut, Iop_Not1, input2Null,
              IRExpr_RdTmp(input2Preexisting));

      IRTemp mkInput1 = newIRTemp(sbOut->tyenv, Ity_I1);
      IRTemp mkInput2 = newIRTemp(sbOut->tyenv, Ity_I1);
      addAnd(sbOut, mkInput1, input1Null, input2Preexisting);
      addAnd(sbOut, mkInput2, input2Null, input1Preexisting);

      IRTemp freshShadowInput1T = newIRTemp(sbOut->tyenv, Ity_I64);
      IRTemp freshShadowInput2T = newIRTemp(sbOut->tyenv, Ity_I64);
      addMakeInputG(sbOut, freshShadowInput1T, argExprs[0], mkInput1,
                    conversionInputType(op_code));
      addMakeInputG(sbOut, freshShadowInput2T, argExprs[1], mkInput2,
                    conversionInputType(op_code));
      addStmtToIRSB(sbOut,
                    IRStmt_WrTmp(shadowInput1T,
                                 IRExpr_ITE(IRExpr_RdTmp(input1Preexisting),
                                            IRExpr_RdTmp(loadedShadowInput1T),
                                            IRExpr_RdTmp(freshShadowInput1T))));
      addStmtToIRSB(sbOut,
                    IRStmt_WrTmp(shadowInput2T,
                                 IRExpr_ITE(IRExpr_RdTmp(input2Preexisting),
                                            IRExpr_RdTmp(loadedShadowInput2T),
                                            IRExpr_RdTmp(freshShadowInput2T))));
    }
  }
  return;

  switch(op_code){
    // These are noops to the shadow value, since they just round the
    // computed value, something which has no real number semantics.
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
    {
      VG_(printf)("Adding copyShadowTemp\n");
      IRDirty* copyTempDirty =
        mkConvert(newShadowT, shadowInput1T, copyShadowTemp);
      copyTempDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyTempDirty));
    }
    break;
    // These change the type of the output, but are otherwise like the
    // above.
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_F64toF32:
  case Iop_F32toF64:
    {
      VG_(printf)("Adding deep copyShadowTemp.\n");
      IRDirty* copyTempDirty =
        mkConvert(newShadowT, shadowInput1T, deepCopyShadowTemp);
      copyTempDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyTempDirty));

      VG_(printf)("Adding changeSingleVaueType.\n");
      IRDirty* changeTypeDirty =
        unsafeIRDirty_0_N(2, "changeSingleValueType",
                          VG_(fnptr_to_fnentry)(changeSingleValueType),
                          mkIRExprVec_2(IRExpr_RdTmp(newShadowT),
                                        mkU64(op_code == Iop_F32toF64 ?
                                              Ft_Double : Ft_Single)));
      changeTypeDirty->guard = IRExpr_RdTmp(inputPreexisting);
      changeTypeDirty->mAddr = mkU64((uintptr_t)&(shadowTemps[dest]));
      changeTypeDirty->mSize = sizeof(ShadowTemp*);
      addStmtToIRSB(sbOut, IRStmt_Dirty(changeTypeDirty));
    }
    break;
    // These manipulate SIMD values
  case Iop_ZeroHI96ofV128:
    {
      VG_(printf)("Adding zeroHi96ofV128.\n");
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, zeroHi96ofV128);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_ZeroHI64ofV128:
    {
      VG_(printf)("Adding zeroHi64ofV128.\n");
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, zeroHi64ofV128);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_V128to32:
    {
      VG_(printf)("Adding V128to32.\n");
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, v128to32);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_V128to64:
    {
      VG_(printf)("Adding v128to64.\n");
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, v128to64);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_V128HIto64:
    {
      VG_(printf)("Adding v128Hito64\n");
      IRDirty* copySecondFloatDirty =
        mkConvert(newShadowT, shadowInput1T, v128Hito64);
      copySecondFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copySecondFloatDirty));
    }
    break;
  case Iop_SetV128lo32:
    {
      VG_(printf)("Adding setV128lo32\n");
      IRDirty* combineFloatsDirty =
        unsafeIRDirty_1_N(newShadowT, 1,
                          "setV128lo32",
                          VG_(fnptr_to_fnentry)(setV128lo32),
                          mkIRExprVec_2(IRExpr_RdTmp(shadowInput1T),
                                        IRExpr_RdTmp(shadowInput2T)));
      combineFloatsDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(combineFloatsDirty));
    }
    break;
  case Iop_SetV128lo64:
    {
      VG_(printf)("Adding setV128lo64\n");
      IRDirty* combineFloatsDirty =
        unsafeIRDirty_1_N(newShadowT, 1,
                          "setV128lo64",
                          VG_(fnptr_to_fnentry)(setV128lo64),
                          mkIRExprVec_2(IRExpr_RdTmp(shadowInput1T),
                                        IRExpr_RdTmp(shadowInput2T)));
      combineFloatsDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(combineFloatsDirty));
    }
    break;
  case Iop_F128LOtoF64:
    {
      VG_(printf)("Adding f128Loto64\n");
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, f128Loto64);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_F128HItoF64:
    {
      VG_(printf)("Adding f128Hito64\n");
      IRDirty* copySecondFloatDirty =
        mkConvert(newShadowT, shadowInput1T, f128Hito64);
      copySecondFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copySecondFloatDirty));
    }
    break;
  case Iop_64HLtoV128:
    {
      VG_(printf)("Adding i64HLtoV128\n");
      IRDirty* combineFloatsDirty =
        unsafeIRDirty_1_N(newShadowT, 1,
                          "i64HLtoV128",
                          VG_(fnptr_to_fnentry)(i64HLtoV128),
                          mkIRExprVec_2(IRExpr_RdTmp(shadowInput1T),
                                        IRExpr_RdTmp(shadowInput2T)));
      combineFloatsDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(combineFloatsDirty));
    }
    break;
  case Iop_F64HLtoF128:
    {
      VG_(printf)("Adding f64HLtoF128\n");
      IRDirty* combineFloatsDirty =
        unsafeIRDirty_1_N(newShadowT, 1,
                          "f64HLtoF128",
                          VG_(fnptr_to_fnentry)(f64HLtoF128),
                          mkIRExprVec_2(IRExpr_RdTmp(shadowInput1T),
                                        IRExpr_RdTmp(shadowInput2T)));
      combineFloatsDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(combineFloatsDirty));
    }
    break;
  default:
    tl_assert(0);
  }
  addStoreGC(sbOut, newShadowT,
             &(shadowTemps[dest]), inputPreexisting);
}

Bool isConversionOp(IROp op_code){
  switch(op_code){
    // Unary ops
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
    // Binary ops
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
    return True;
  default:
    return False;
  }
}
int conversionInputType(IROp op_code){
  switch(op_code){
  case Iop_ZeroHI96ofV128:
  case Iop_V128to32:
  case Iop_F32toF64:
  case Iop_RoundF32toInt:
  case Iop_SetV128lo32:
    return Ft_Single;
  case Iop_ZeroHI64ofV128:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_RoundF64toInt:
  case Iop_F64toF32:
  case Iop_SetV128lo64:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
    return Ft_Double;
  default:
    tl_assert(0);
    return -1;
  }
}

int conversionInputArgIndex(IROp op_code){
  switch(op_code){
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_64UtoV128:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_F32toF64:
    return 0;
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
    return 1;
  default:
    tl_assert(0);
    return -1;
  }
}
int numConversionInputs(IROp op_code){
  switch(op_code){
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_64UtoV128:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_F32toF64:
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_F64toF32:
    return 1;
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
    return 2;
  default:
    tl_assert(0);
    return -1;
  }
}
