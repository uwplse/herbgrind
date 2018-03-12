/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie             shadowop.c ---*/
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

#include "shadowop.h"
#include "../value-shadowstate/value-shadowstate.h"
#include "../value-shadowstate/range.h"
#include "realop.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "error.h"
#include "symbolic-op.h"
#include "local-op.h"
#include "influence-op.h"
#include "../../helper/ir-info.h"
#include "../../helper/runtime-util.h"

VG_REGPARM(1) ShadowTemp* executeShadowOp(ShadowOpInfoInstance* infoInstance){
  ShadowOpInfo* opInfo = infoInstance->info;
  // Make sure the op code is sane, so that things don't go bonkers
  // later.
  tl_assert(opInfo->op_code <
            IEop_REALLY_LAST_FOR_REAL_GUYS);

  // Create a shadow temp for the result.
  FloatBlocks numBlocks = numOpBlocks(opInfo->op_code);
  ShadowTemp* result = mkShadowTemp(numBlocks);

  // Get the computed and shadow arguments.
  int nargs = numArgs(opInfo->op_code);
  int numChannels = numChannelsOut(opInfo->op_code);
  ShadowTemp* args[4];
  double clientArgs[4][4];
  for(int i = 0; i < nargs; ++i){
    args[i] = getArg(i, opInfo->op_code, infoInstance->argTemps[i]);
    for (int j = 0; j < numChannels; ++j){
      ValueType argPrecision = opBlockArgPrecision(opInfo->op_code, j / 2);
      clientArgs[j][i] = argPrecision == Vt_Double ?
        computedArgs.argValues[i][j] :
        computedArgs.argValuesF[i][j];
    }
  }
  // Do the operation on the operand channels
  int numOperandChannels = numSIMDOperands(opInfo->op_code);
  int numOperandBlocks =
    numOperandChannels * (opArgPrecision(opInfo->op_code) == Vt_Double ? 2 : 1);
  for(int i = 0; i < numOperandBlocks; ++i){
    ShadowValue* vals[nargs];
    ValueType argPrecision = opArgPrecision(opInfo->op_code);
    if (argPrecision == Vt_Double && i % 2 == 1){
      result->values[i] = NULL;
      continue;
    }
    for(int j = 0; j < nargs; ++j){
      vals[j] = args[j]->values[i];
    }
    double computedOutput = (argPrecision == Vt_Single ?
       computedResult.f[i] : computedResult.d[i / 2]);
    result->values[i] =
      executeChannelShadowOp(opInfo,
                             vals,
                             clientArgs[i],
                             computedOutput);
  }
  // Copy across argument on the non-operand channels
  for(int i = numOperandBlocks; i < INT(numBlocks); ++i){
    // According to the libvex_ir.h documentation, the non-operated
    // values should be copied from the first operand.
    result->values[i] = args[0]->values[i];
    ownShadowValue(result->values[i]);
  }

  // Print debug info about the operation if those flags are on
  if (PRINT_TEMP_MOVES){
    VG_(printf)("Making %p for result of shadow op.\n",
                result);
  }

  if (PRINT_VALUE_MOVES){
    ppIROp_Extended(opInfo->op_code);
    VG_(printf)(": Making value(s) ");
    for(int i = 0; i < numOperandChannels; ++i){
      if (i == 0){
        VG_(printf)("%p", result->values[i]);
      } else {
        VG_(printf)(", %p", result->values[i]);
      }
    }
    if (numOperandChannels < numChannels){
      VG_(printf)(" and copying shadow value(s) ");
      for(int i = numOperandChannels;
          i < numChannels; ++i){
        VG_(printf)("%p (new rc %lu), ",
                    result->values[i], result->values[i]->ref_count);
      }
      VG_(printf)("from %p to %p -> ", args[0], result);
    } else {
      VG_(printf)(" -> ");
    }
  }

  // Clean up any args we made for constants
  for(int i = 0; i < nargs; ++i){
    if (infoInstance->argTemps[i] == -1){
      disownShadowTemp_fast(args[i]);
    }
  }
  return result;
}
ShadowTemp* getArg(int argIdx, IROp op, IRTemp argTemp){
  if (argTemp == -1 ||
      shadowTemps[argTemp] == NULL){
    FloatBlocks numBlocks = numOpBlocks(op);
    int numChannels = numChannelsIn(op);
    ShadowTemp* result = mkShadowTemp(numBlocks);
    if (PRINT_TEMP_MOVES){
      VG_(printf)("Making shadow temp %p (%d vals) for argument %d\n",
                  result, numChannels, argIdx);
    }
    for(int j = 0; j < INT(numBlocks); j++){
      ValueType argPrecision = opBlockArgPrecision(op, j);
      if (argPrecision == Vt_Double && j % 2 == 1){
        result->values[j] = NULL;
        continue;
      }
      double value = argPrecision == Vt_Double ?
        computedArgs.argValues[argIdx][j] :
        computedArgs.argValuesF[argIdx][j/2];
      result->values[j] =
        mkShadowValue(argPrecision, value);
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Making shadow value %p for argument %d (%p) in t%d.\n",
                    result->values[j], argIdx, result, argTemp);
      }
    }
    if (argTemp != -1){
      if (PRINT_TEMP_MOVES){
        VG_(printf)("Storing shadow temp %p (%d vals) at t%d for argument\n",
                    result, numChannels, argTemp);
      }
      shadowTemps[argTemp] = result;
    }
    return result;
  } else {
    return shadowTemps[argTemp];
  }
}
void ppInfluenceAddrs(ShadowValue* val);
void ppInfluenceAddrs(ShadowValue* val){
  tl_assert(val != NULL);
  InfluenceList list = val->influences;
  if (list != NULL){
    VG_(printf)("%lX", list->item->op_addr);
    for(InfluenceList curNode = list->next; curNode != NULL; curNode = curNode->next){
      VG_(printf)(", and %lX", curNode->item->op_addr);
    }
  }
}
ShadowValue* executeChannelShadowOp(ShadowOpInfo* opinfo,
                                    ShadowValue** args,
                                    double* clientArgs,
                                    double clientResult){
  // Two things should conspire to make this safe. First of all, the
  // type system: if the operation we're dispatched on operates on
  // multiple values, and the types of those values don't match the
  // type of the first value, the type system should let us know at
  // instrument time, and we'll never get here. Secondly, the
  // organization of executeShadowOp above: if we're dealing with an
  // instruction where the value types DON'T have to match (*32F0x4
  // and *64F0x2), then we should only be run on the first value in
  // that instruction.
  ValueType argPrecision = opArgPrecision(opinfo->op_code);
  int nargs = numArgs(opinfo->op_code);
  if (!dont_ignore_pure_zeroes && !no_reals){
    switch((int)opinfo->op_code){
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
      if ((getDouble(args[0]->real) == 0 && !isNaN(args[1]->real)) ||
          (getDouble(args[1]->real) == 0 && !isNaN(args[0]->real))){
        ShadowValue* result =
          mkShadowValue(argPrecision, clientResult);
        if (use_ranges){
          updateRanges(opinfo->agg.inputs.range_records, clientArgs, nargs);
        }
        execSymbolicOp(opinfo, &(result->expr), clientResult, args, False);
        return result;
      }
      break;
    default:
      break;
    }
  }
  if (print_inputs){
    for(int i = 0; i < nargs; ++i){
      VG_(printf)("Arg %d is computed as ", i + 1);
      ppFloat(clientArgs[i]);
      VG_(printf)(", and is shadowed as ");
      ppFloat(getDouble(args[i]->real));
      VG_(printf)("\n");
    }
  }
  ShadowValue* result = mkShadowValueBare(argPrecision);
  execRealOp(opinfo->op_code, &(result->real), args);
  if (use_ranges){
    updateRanges(opinfo->agg.inputs.range_records, clientArgs, nargs);
  }

  if (print_errors_long || print_errors){
    printOpInfo(opinfo);
    VG_(printf)(":\n");
  }
  if (print_errors_long || print_errors){
    VG_(printf)("Local:\n");
  }
  execLocalOp(opinfo, result->real, result, args);
  if (print_errors_long || print_errors){
    VG_(printf)("Global:\n");
  }
  double bitsGlobalError =
    updateError(&(opinfo->agg.global_error), result->real, clientResult);
  execSymbolicOp(opinfo, &(result->expr), clientResult, args,
                 bitsGlobalError > error_threshold);
  if (print_expr_refs){
    VG_(printf)("Making new expression %p for value %p with 0 references.\n",
                result->expr, result);
  }
  if (print_semantic_ops){
    VG_(printf)("%p = ", result);
    ppIROp(opinfo->op_code);
    switch(nargs){
    case 0:
      tl_assert(0);
      return NULL;
    case 1:
      VG_(printf)("(%p)\n", args[0]);
      break;
    default:
      VG_(printf)("(%p,", args[0]);
      for(int i = 1; i < nargs; ++i){
        VG_(printf)(" %p", args[i]);
      }
      VG_(printf)(")\n");
    }
  }
  if (compensation_detection && !no_reals){
    switch((int)opinfo->op_code){
    case Iop_Add32F0x4:
    case Iop_Add64F0x2:
    case Iop_AddF64:
    case Iop_AddF32:
      if (getDouble(args[0]->real) == 0){
        ULong inputError = ulpd(getDouble(args[1]->real), clientArgs[1]);
        ULong outputError = ulpd(getDouble(result->real), clientResult);
        if (outputError <= inputError){
          result->influences = cloneInfluences(args[1]->influences);
          return result;
        }
      }
      // Intentional overflow to the next set of cases: both adds and
      // subtracts are considered compensating if their second
      // argument is zero in the reals (and the error decreases), but
      // only adds also are compensating if their first argument is
      // zero in the reals.
    case Iop_Sub32F0x4:
    case Iop_Sub64F0x2:
    case Iop_SubF64:
    case Iop_SubF32:
      if (getDouble(args[1]->real) == 0){
        ULong inputError = ulpd(getDouble(args[0]->real), clientArgs[0]);
        ULong outputError = ulpd(getDouble(result->real), clientResult);
        if (outputError <= inputError){
          result->influences = cloneInfluences(args[0]->influences);
          return result;
        }
      }
      break;
    default:
      break;
    }
  }
  execInfluencesOp(opinfo, &(result->influences), args);
  return result;
}

FloatBlocks numOpBlocks(IROp op){
  IRType destType;
  IRType argTypes[4];
  typeOfPrimop(op, &destType,
               &(argTypes[0]), &(argTypes[1]), &(argTypes[2]), &(argTypes[3]));
  return typeSize(destType);
}
