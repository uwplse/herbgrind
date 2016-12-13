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

#include "conversion.h"
#include "../helper/instrument-util.h"
#include "../runtime/value-shadowstate/value-shadowstate.h"
#include "../runtime/shadowop/conversions.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"
#include "instrument-storage.h"

void instrumentConversion(IRSB* sbOut, IROp op_code, IRExpr** argExprs,
                          IRTemp dest){
  IRTemp shadowInput1T, shadowInput2T;
  IRTemp inputPreexisting = newIRTemp(sbOut->tyenv, Ity_I1);
  IRTemp newShadowT = newIRTemp(sbOut->tyenv, Ity_I64);

  return;

  if (numConversionInputs(op_code) == 1){
    int inputIndex = conversionInputArgIndex(op_code);
    if (argExprs[inputIndex]->tag == Iex_Const ||
        !isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[0]))){
      return;
    }
    shadowInput1T = runLoad64C(sbOut, &(shadowTemps[argExprs[inputIndex]->Iex.RdTmp.tmp]));
    shadowInput2T = 0;
    inputPreexisting = runNonZeroCheck64(sbOut, shadowInput1T);
  } else {
    if ((argExprs[0]->tag == Iex_Const ||
         !isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[0]))) &&
        (argExprs[1]->tag == Iex_Const ||
         !isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[1])))){
      return;
    }
    if (argExprs[0]->tag == Iex_Const ||
        !isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[0]))){
      shadowInput1T = runMakeInputG(sbOut, inputPreexisting, argExprs[0], argPrecision(op_code));
      shadowInput2T = runLoad64C(sbOut, &(shadowTemps[argExprs[1]->Iex.RdTmp.tmp]));
      inputPreexisting = runNonZeroCheck64(sbOut, shadowInput2T);
    } else if (argExprs[1]->tag == Iex_Const ||
               !isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[1]))){
      shadowInput1T = runLoad64C(sbOut, &(shadowTemps[argExprs[0]->Iex.RdTmp.tmp]));
      shadowInput2T = runMakeInputG(sbOut, inputPreexisting, argExprs[1], argPrecision(op_code));
      inputPreexisting = runNonZeroCheck64(sbOut, shadowInput1T);
    } else {
      IRTemp loadedShadowInput1T = runLoad64C(sbOut, &(shadowTemps[argExprs[0]->Iex.RdTmp.tmp]));
      IRTemp loadedShadowInput2T = runLoad64C(sbOut, &(shadowTemps[argExprs[1]->Iex.RdTmp.tmp]));
      IRTemp input1Preexisting = runNonZeroCheck64(sbOut, loadedShadowInput1T);
      IRTemp input2Preexisting = runNonZeroCheck64(sbOut, loadedShadowInput2T);
      inputPreexisting =
        runOr(sbOut, input1Preexisting, input2Preexisting);
      IRTemp input1Null = runUnop(sbOut, Iop_Not1, IRExpr_RdTmp(input1Preexisting));
      IRTemp input2Null = runUnop(sbOut, Iop_Not1, IRExpr_RdTmp(input2Preexisting));

      IRTemp mkInput1 = runAnd(sbOut, input1Null, input2Preexisting);
      IRTemp mkInput2 = runAnd(sbOut, input2Null, input1Preexisting);

      IRTemp freshShadowInput1T = runMakeInputG(sbOut, mkInput1, argExprs[0], argPrecision(op_code));
      IRTemp freshShadowInput2T = runMakeInputG(sbOut, mkInput2, argExprs[1], argPrecision(op_code));
      addStoreGC(sbOut, mkInput1, IRExpr_RdTmp(freshShadowInput1T),
                 &(shadowTemps[argExprs[0]->Iex.RdTmp.tmp]));
      addStoreGC(sbOut, mkInput1, IRExpr_RdTmp(freshShadowInput2T),
                 &(shadowTemps[argExprs[1]->Iex.RdTmp.tmp]));
      shadowInput1T = runITE(sbOut, input1Preexisting,
                             IRExpr_RdTmp(loadedShadowInput1T),
                             IRExpr_RdTmp(freshShadowInput1T));
      shadowInput2T = runITE(sbOut, input2Preexisting,
                             IRExpr_RdTmp(loadedShadowInput2T),
                             IRExpr_RdTmp(freshShadowInput1T));
    }
  }

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
      IRDirty* copyTempDirty =
        mkConvert(newShadowT, shadowInput1T, deepCopyShadowTemp);
      copyTempDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyTempDirty));

      IRDirty* changeTypeDirty =
        unsafeIRDirty_0_N(2, "changeSingleValueType",
                          VG_(fnptr_to_fnentry)(changeSingleValueType),
                          mkIRExprVec_2(IRExpr_RdTmp(newShadowT),
                                        mkU64(op_code == Iop_F32toF64 ?
                                              Ft_Double : Ft_Single)));
      changeTypeDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(changeTypeDirty));
    }
    break;
    // These manipulate SIMD values
  case Iop_ZeroHI96ofV128:
    {
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, zeroHi96ofV128);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_ZeroHI64ofV128:
    {
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, zeroHi64ofV128);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_V128to32:
    {
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, v128to32);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_V128to64:
    {
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, v128to64);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_V128HIto64:
    {
      IRDirty* copySecondFloatDirty =
        mkConvert(newShadowT, shadowInput1T, v128Hito64);
      copySecondFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copySecondFloatDirty));
    }
    break;
  case Iop_SetV128lo32:
    {
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
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, f128Loto64);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  case Iop_F128HItoF64:
    {
      IRDirty* copySecondFloatDirty =
        mkConvert(newShadowT, shadowInput1T, f128Hito64);
      copySecondFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copySecondFloatDirty));
    }
    break;
  case Iop_64HLtoV128:
    {
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
  case Iop_64UtoV128:
    {
      IRDirty* copyFirstFloatDirty =
        mkConvert(newShadowT, shadowInput1T, f128Loto64);
      copyFirstFloatDirty->guard = IRExpr_RdTmp(inputPreexisting);
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyFirstFloatDirty));
    }
    break;
  default:
    tl_assert(0);
  }
  // Store the result if the input already existed, store NULL
  // otherwise.
  IRTemp resultVal = runITE(sbOut, inputPreexisting,
                            IRExpr_RdTmp(newShadowT),
                            mkU64(0));
  addStoreC(sbOut, IRExpr_RdTmp(resultVal),
            &(shadowTemps[dest]));

  // Finally, if we created inputs for constants, free them up, since
  // we have no where to put them.
  if (numConversionInputs(op_code) == 2){
    if (argExprs[0]->tag == Iex_Const){
      addDisownNonNull(sbOut, shadowInput1T, numChannelsIn(op_code));
    }
    if (argExprs[1]->tag == Iex_Const){
      addDisownNonNull(sbOut, shadowInput2T, numChannelsIn(op_code));
    }
  }
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
    return 1;
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
    return 2;
  default:
    tl_assert(0);
    return -1;
  }
}
