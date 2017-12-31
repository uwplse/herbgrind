/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie          conversions.c ---*/
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

#include "conversion.h"
#include "../helper/instrument-util.h"
#include "../runtime/shadowop/conversions.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"

#include "instrument-storage.h"
#include "ownership.h"
#include "../runtime/value-shadowstate/value-shadowstate.h"
#include "../runtime/shadowop/shadowop.h"
#include "../helper/debug.h"
#include "../helper/ir-info.h"

typedef enum {
  Uncertain,
  DefinitelyFalse,
  DefinitelyTrue,
} Booly;

void instrumentConversion(IRSB* sbOut, IROp op_code, IRExpr** argExprs,
                          IRTemp dest, int instrIdx){
  IRExpr* shadowInputs[2];
  IRExpr* inputPreexisting;
  Booly inputsPreexistingStatic[2] = {Uncertain, Uncertain};
  IRExpr* inputsPreexistingDynamic[2] = {NULL, NULL};
  IRExpr* shadowOutput;

  if (print_conversions){
    VG_(printf)("Instrumenting ");
    ppIROp(op_code);
    VG_(printf)("\n");
    addPrint("Running ");
    addPrintOp(op_code);
    addPrint(" to ");
    addPrint2("t%d\n", mkU64(dest));
  }

  if (numConversionInputs(op_code) == 1){
    int inputIndex = conversionInputArgIndex(op_code);
    if (argExprs[inputIndex]->tag == Iex_RdTmp){
      tempShadowStatus[dest] = tempShadowStatus[argExprs[inputIndex]->Iex.RdTmp.tmp];
    }
    if (canStoreShadow(sbOut->tyenv, argExprs[inputIndex])){
      shadowInputs[0] =
        runLoadTemp(sbOut, argExprs[inputIndex]->Iex.RdTmp.tmp);
      if (staticallyShadowed(argExprs[inputIndex])){
        // To make sure we don't accidentally use this in any guarded
        // calls when this happens, because it means we statically know
        // whether it preexists or not.
        inputPreexisting = NULL;
      } else {
        inputPreexisting = runNonZeroCheck64(sbOut, shadowInputs[0]);
      }
    } else {
      return;
    }
    shadowInputs[1] = 0;
  } else {
    if (!canStoreShadow(sbOut->tyenv, argExprs[0]) &&
        !canStoreShadow(sbOut->tyenv, argExprs[1])){
      tempShadowStatus[dest] = Ss_Unshadowed;
      return;
    } else if (staticallyShadowed(argExprs[0]) ||
               staticallyShadowed(argExprs[1])) {
      tempShadowStatus[dest] = Ss_Shadowed;
      for (int i = 0; i < 2; ++i){
        if (canStoreShadow(sbOut->tyenv, argExprs[i])){
          shadowInputs[i] =
            runLoadTemp(sbOut, argExprs[i]->Iex.RdTmp.tmp);
          if (!staticallyShadowed(argExprs[i])){
            IRExpr* loadedNull =
              runZeroCheck64(sbOut, shadowInputs[i]);
            int numVals =
              inferOtherNumChannels(i, argExprs[1-i], op_code);
            IRExpr* freshInput =
              runMakeInputG(sbOut, loadedNull,
                            argExprs[i],
                            tempType(argExprs[1-i]->Iex.RdTmp.tmp),
                            numVals);
            shadowInputs[i] = runITE(sbOut, loadedNull,
                                     freshInput,
                                     shadowInputs[i]);
            inputsPreexistingStatic[i] = Uncertain;
            inputsPreexistingDynamic[i] =
              runUnop(sbOut, Iop_Not1, loadedNull);
          } else {
            inputsPreexistingStatic[i] = DefinitelyTrue;
            inputsPreexistingDynamic[i] = NULL;
          }
        } else {
          inputsPreexistingStatic[i] = DefinitelyFalse;
          inputsPreexistingDynamic[i] = NULL;
          int numVals =
            inferOtherNumChannels(i, argExprs[1-i], op_code);
          shadowInputs[i] =
            runMakeInput(sbOut, argExprs[i],
                         tempType(argExprs[1-i]->Iex.RdTmp.tmp),
                         numVals);
        }
      }
      // To make sure we don't accidentally use this in any guarded
      // calls when this happens, because it means we statically know
      // whether it preexists or not.
      inputPreexisting = NULL;
    } else if (!canStoreShadow(sbOut->tyenv, argExprs[0])) {
      shadowInputs[1] =
        runLoadTemp(sbOut, argExprs[1]->Iex.RdTmp.tmp);
      inputPreexisting =
        runNonZeroCheck64(sbOut, shadowInputs[1]);
      ValueType inferredPrecision = argPrecision(op_code);
      // You better make sure you handle all the cases where this
      // could be false lower down.
      if (inferredPrecision != Vt_Unknown){
        int num_vals =
          inferOtherNumChannels(0, shadowInputs[1], op_code);
        shadowInputs[0] =
          runMakeInputG(sbOut, inputPreexisting, argExprs[0],
                        inferredPrecision, num_vals);
      } else {
        shadowInputs[0] = NULL;
      }
      inputsPreexistingStatic[0] = DefinitelyFalse;
      inputsPreexistingStatic[1] = Uncertain;
      inputsPreexistingDynamic[0] = NULL;
      inputsPreexistingDynamic[1] = inputPreexisting;
    } else if (!canStoreShadow(sbOut->tyenv, argExprs[1])) {
      tempShadowStatus[dest] = Ss_Unknown;
      shadowInputs[0] =
        runLoadTemp(sbOut, argExprs[0]->Iex.RdTmp.tmp);
      inputPreexisting =
        runNonZeroCheck64(sbOut, shadowInputs[0]);
      // You better make sure you handle all the cases where this
      // could be false lower down.
      ValueType inferredPrecision = argPrecision(op_code);
      if (inferredPrecision != Vt_Unknown){
        int numChannels = inferOtherNumChannels(1, shadowInputs[0],
                                                op_code);
        shadowInputs[1] =
          runMakeInputG(sbOut, inputPreexisting, argExprs[1],
                        inferredPrecision,
                        numChannels);
      } else {
        shadowInputs[1] = NULL;
      }
      inputsPreexistingStatic[0] = Uncertain;
      inputsPreexistingStatic[1] = DefinitelyFalse;
      inputsPreexistingDynamic[0] = inputPreexisting;
      inputsPreexistingDynamic[1] = NULL;
    } else {
      tempShadowStatus[dest] = Ss_Unknown;
      inputsPreexistingStatic[0] = Uncertain;
      inputsPreexistingStatic[1] = Uncertain;
      for(int i = 0; i < 2; ++i){
        shadowInputs[i] =
          runLoadTemp(sbOut, argExprs[i]->Iex.RdTmp.tmp);
        inputsPreexistingDynamic[i] =
          runNonZeroCheck64(sbOut, shadowInputs[i]);
      }
      for(int i = 0; i < 2; ++i){
        IRExpr* shouldMake =
          runAnd(sbOut,
                 runUnop(sbOut, Iop_Not1,
                         inputsPreexistingDynamic[i]),
                 inputsPreexistingDynamic[1-i]);
        ValueType inferredPrecision = argPrecision(op_code);
        // You better make sure you handle all the cases where this
        // could be false lower down.
        if (inferredPrecision != Vt_Unknown){
          IRExpr* freshInput =
            runMakeInputG(sbOut, shouldMake, argExprs[i],
                          inferredPrecision,
                          inferOtherNumChannels(i, argExprs[i-1],
                                                op_code));
          shadowInputs[i] =
            runITE(sbOut, shouldMake, freshInput, shadowInputs[i]);
        }
      }
      inputPreexisting =
        runOr(sbOut,
              inputsPreexistingDynamic[0],
              inputsPreexistingDynamic[1]);
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
      if (inputPreexisting == NULL){
        tl_assert(shadowInputs[0]);
        shadowOutput =
          runPureCCall64(sbOut, copyShadowTemp, shadowInputs[0]);
      } else {
        tl_assert(shadowInputs[0]);
        shadowOutput =
          runDirtyG_1_1(sbOut, inputPreexisting,
                        copyShadowTemp, shadowInputs[0]);
      }
    }
    break;
    // These change the type of the output, but are otherwise like the
    // above.
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_F64toF32:
  case Iop_F32toF64:
    {
      if (inputPreexisting == NULL){
        tl_assert(shadowInputs[0]);
        shadowOutput =
          runPureCCall64(sbOut, deepCopyShadowTemp, shadowInputs[0]);
      } else {
        tl_assert(shadowInputs[0]);
        shadowOutput =
          runDirtyG_1_1(sbOut, inputPreexisting, deepCopyShadowTemp,
                        shadowInputs[0]);
      }

      IRDirty* changeTypeDirty =
        unsafeIRDirty_0_N(2, "changeSingleValueType",
                          VG_(fnptr_to_fnentry)(changeSingleValueType),
                          mkIRExprVec_2(shadowOutput,
                                        mkU64(op_code == Iop_F32toF64 ?
                                              Vt_Double : Vt_Single)));
      if (inputPreexisting != NULL){
        changeTypeDirty->guard = inputPreexisting;
      }
      addStmtToIRSB(sbOut, IRStmt_Dirty(changeTypeDirty));
    }
    break;
    // These manipulate SIMD values
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_64UtoV128:
  case Iop_32UtoV128:
  case Iop_32Uto64:
  case Iop_64to32:
    {
      ShadowTemp* (*convertFunc)(ShadowTemp* input);
      switch(op_code){
      case Iop_ZeroHI96ofV128:
        convertFunc = zeroHi96ofV128;
        break;
      case Iop_ZeroHI64ofV128:
        convertFunc = zeroHi64ofV128;
        break;
      case Iop_V128to32:
        convertFunc = v128to32;
        break;
      case Iop_V128to64:
        convertFunc = v128to64;
        break;
      case Iop_V128HIto64:
        convertFunc = v128Hito64;
        break;
      case Iop_64UtoV128:
        convertFunc = i64UtoV128;
        break;
      case Iop_32UtoV128:
        convertFunc = i32UtoV128;
        break;
      case Iop_32Uto64:
        convertFunc = i32Uto64;
        break;
      case Iop_64to32:{
        // This is the one conversion that can fail based on the FORM
        // of the shadow temp. If its not two singles, but is instead
        // one double, then taking the first half is meaningless. So
        // we only execute the conversion if the number of values
        // matches "2", which we check here.
        IRExpr* numValsMatch;
        if (inputPreexisting == NULL){
          numValsMatch = runBinop(sbOut, Iop_CmpEQ64, mkU64(2),
                                  runArrow(sbOut, shadowInputs[0], ShadowTemp,
                                           num_vals));
          inputPreexisting = numValsMatch;
        } else {
          numValsMatch = runBinop(sbOut, Iop_CmpEQ64, mkU64(2),
                                          runArrowG(sbOut, inputPreexisting,
                                                    shadowInputs[0], ShadowTemp,
                                                    num_vals));
          inputPreexisting = runAnd(sbOut, inputPreexisting,
                                    numValsMatch);
        }
        convertFunc = i64to32;
      }
        break;
      default:
        tl_assert(0);
        return;
      }
      if (inputPreexisting == NULL){
        tl_assert(shadowInputs[0]);
        shadowOutput =
          runPureCCall64(sbOut, convertFunc, shadowInputs[0]);
      } else {
        tl_assert(shadowInputs[0]);
        shadowOutput =
          runDirtyG_1_1(sbOut, inputPreexisting,
                        convertFunc, shadowInputs[0]);
      }
    }
    break;
  case Iop_SetV128lo64:
    {
      // In this case we'll be able to infer the types, so we will
      // have already constructed both inputs.
      if (inputsPreexistingStatic[0] == DefinitelyTrue ||
          inputsPreexistingStatic[1] == DefinitelyTrue){
        tl_assert(shadowInputs[0]);
        tl_assert(shadowInputs[1]);
        shadowOutput =
          runPureCCall(sbOut,
                       mkIRCallee(2, "setV128lo64",
                                  VG_(fnptr_to_fnentry)(setV128lo64)),
                       Ity_I64, mkIRExprVec_2(shadowInputs[0], shadowInputs[1]));
      } else if (inputsPreexistingStatic[0] == DefinitelyFalse){
        tl_assert(inputsPreexistingDynamic[1]);
        addStoreC(sbOut, argExprs[0], computedArgs.argValues[0]);
        shadowOutput =
          runDirtyG_1_3(sbOut, inputsPreexistingDynamic[1],
                        setV128lo64Dynamic1,
                        shadowInputs[1],
                        mkU64(argExprs[0]->Iex.RdTmp.tmp),
                        mkU64((uintptr_t)computedArgs.argValues[0]));
        cleanupAtEndOfBlock(sbOut, argExprs[0]->Iex.RdTmp.tmp);
      } else if (inputsPreexistingStatic[1] == DefinitelyFalse){
        tl_assert(inputsPreexistingDynamic[0]);
        shadowOutput =
          runDirtyG_1_3(sbOut, inputsPreexistingDynamic[0],
                        setV128lo64Dynamic2,
                        shadowInputs[0],
                        mkU64(argExprs[1]->Iex.RdTmp.tmp),
                        argExprs[1]);
        cleanupAtEndOfBlock(sbOut, argExprs[1]->Iex.RdTmp.tmp);
      } else {
        // Otherwise we couldn't infer types statically, so we have to
        // use guarded dynamic calls depending on which one(s) already
        // exist.

        // If neither exist, we won't do anything, just like every
        // other conversion operation.

        // If one exists, but the other does not, at runtime we'll
        // infer the type of the one that doesn't exist based on the
        // type of the one that does, so we can create a shadow value
        // for it and do the operation.
        tl_assert(inputsPreexistingDynamic[0]);
        tl_assert(inputsPreexistingDynamic[1]);
        IRExpr* shouldCreate1 =
          runAnd(sbOut,
                 runUnop(sbOut, Iop_Not1, inputsPreexistingDynamic[0]),
                 inputsPreexistingDynamic[1]);
        IRExpr* result1;
        if (argExprs[0]->tag == Iex_RdTmp){
          tl_assert(shadowInputs[1]);
          addStoreC(sbOut, argExprs[0], computedArgs.argValues[0]);
          result1 =
            runDirtyG_1_3(sbOut, shouldCreate1,
                          setV128lo64Dynamic1,
                          shadowInputs[1],
                          mkU64(argExprs[0]->Iex.RdTmp.tmp),
                          mkU64((uintptr_t)computedArgs.argValues[0]));
          cleanupAtEndOfBlock(sbOut, argExprs[0]->Iex.RdTmp.tmp);
        } else {
          tl_assert(shadowInputs[1]);
          result1 =
            runDirtyG_1_3(sbOut, shouldCreate1,
                          setV128lo64Dynamic1,
                          shadowInputs[1],
                          mkU64(IRTemp_INVALID),
                          argExprs[0]);
        }
        IRExpr* shouldCreate2 =
          runAnd(sbOut,
                 runUnop(sbOut, Iop_Not1, inputsPreexistingDynamic[1]),
                 inputsPreexistingDynamic[0]);
        IRExpr* result2;
        if (argExprs[1]->tag == Iex_RdTmp){
          tl_assert(shadowInputs[0]);
          result2 =
            runDirtyG_1_3(sbOut, shouldCreate2,
                          setV128lo64Dynamic2,
                          shadowInputs[0],
                          mkU64(argExprs[1]->Iex.RdTmp.tmp),
                          argExprs[1]);
          cleanupAtEndOfBlock(sbOut, argExprs[1]->Iex.RdTmp.tmp);
        } else {
          tl_assert(shadowInputs[0]);
          result2 =
            runDirtyG_1_3(sbOut, shouldCreate2,
                          setV128lo64Dynamic2,
                          shadowInputs[0],
                          mkU64(IRTemp_INVALID),
                          argExprs[1]);
        }
        IRExpr* shouldCreateNeither =
          runAnd(sbOut,
                 inputsPreexistingDynamic[0],
                 inputsPreexistingDynamic[1]);
        IRExpr* result3 =
          runDirtyG_1_2(sbOut, shouldCreateNeither,
                        setV128lo64,
                        shadowInputs[0],
                        shadowInputs[1]);

        shadowOutput =
          runITE(sbOut, shouldCreateNeither,
                 result3,
                 runITE(sbOut, shouldCreate1,
                        result1,
                        result2));
      }
    }
    break;
  case Iop_SetV128lo32:
  case Iop_64HLtoV128:
    {
      ShadowTemp* (*combineFunc)(ShadowTemp* in1, ShadowTemp* in2);
      switch(op_code){
      case Iop_SetV128lo32:
        combineFunc = setV128lo32;
        break;
      case Iop_64HLtoV128:
        combineFunc = i64HLtoV128;
        break;
      default:
        tl_assert(0);
        return;
      }
      if (inputPreexisting == NULL){
        shadowOutput =
          runPureCCall(sbOut,
                       mkIRCallee(2, "setV128lo32",
                                  VG_(fnptr_to_fnentry)(combineFunc)),
                       Ity_I64,
                       mkIRExprVec_2(shadowInputs[0], shadowInputs[1]));
      } else {
        shadowOutput = runDirtyG_1_2(sbOut, inputPreexisting,
                                     combineFunc,
                                     shadowInputs[0],
                                     shadowInputs[1]);
      }
    }
    break;
  default:
    tl_assert(0);
    return;
  }
  ValueType outputPrecision = resultPrecision(op_code);
  if (outputPrecision == Vt_Unknown){
    if (staticallyShadowed(argExprs[0])){
      outputPrecision = tempType(argExprs[0]->Iex.RdTmp.tmp);
    } else if (numConversionInputs(op_code) == 2 &&
               staticallyShadowed(argExprs[1])){
      outputPrecision = tempType(argExprs[1]->Iex.RdTmp.tmp);
    }
  }
  if ((numConversionInputs(op_code) == 1 &&
       staticallyShadowed(argExprs[conversionInputArgIndex(op_code)])) ||
      (numConversionInputs(op_code) == 2 &&
       (staticallyShadowed(argExprs[0]) ||
        staticallyShadowed(argExprs[1])))){
    if (print_temp_moves){
      addPrintOp(op_code);
      addPrint3(": Putting converted temp %p in %d\n",
                shadowOutput, mkU64(dest));
    }
    addStoreTemp(sbOut, shadowOutput,
                 outputPrecision,
                 dest);
  } else {
    if (print_temp_moves){
      addPrintOpG(inputPreexisting, op_code);
      addPrintG3(inputPreexisting, ": Putting converted temp %p in %d\n",
                 shadowOutput, mkU64(dest));
    }
    tl_assert(inputPreexisting != NULL);
    addStoreTempG(sbOut, inputPreexisting, shadowOutput,
                  outputPrecision,
                  dest);
  }

  // Finally, if we created inputs for constants, free them up, since
  // we have no where to put them.
  if (numConversionInputs(op_code) == 2){
    if (inputPreexisting == NULL){
      for (int i = 0; i < 2; ++i){
        if (!canStoreShadow(sbOut->tyenv, argExprs[i])){
          if (argPrecision(op_code) == Vt_Unknown){
            addDynamicDisownNonNullDetached(sbOut, shadowInputs[i]);
          } else {
            addDisownNonNull(sbOut, shadowInputs[i],
                             inferOtherNumChannels(i, argExprs[i],
                                                   op_code));
          }
        }
      }
    } else {
      for (int i = 0; i < 2; ++i){
        if (!canStoreShadow(sbOut->tyenv, argExprs[i])){
          if (argPrecision(op_code) == Vt_Unknown){
            addDynamicDisown(sbOut, i);
          } else {
            int num_vals =
              inferOtherNumChannels(i, shadowInputs[1-i], op_code);
            addDisownG(sbOut, inputPreexisting,
                       shadowInputs[i],
                       num_vals);
          }
        }
      }
    }
  }
}

int conversionInputArgIndex(IROp op_code){
  switch(op_code){
  case Iop_32Uto64:
  case Iop_64to32:
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_64UtoV128:
  case Iop_32UtoV128:
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
  case Iop_32Uto64:
  case Iop_64to32:
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_64UtoV128:
  case Iop_32UtoV128:
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
