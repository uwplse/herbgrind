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

VG_REGPARM(1) ShadowTemp* executeShadowOp(ShadowOpInfoInstance* infoInstance){
  ShadowOpInfo* opInfo = infoInstance->info;
  // Make sure the op code is sane, so that things don't go bonkers
  // later.
  tl_assert(opInfo->op_code <
            IEop_REALLY_LAST_FOR_REAL_GUYS);

  // Create a shadow temp for the result.
  ShadowTemp* result = mkShadowTemp(opInfo->exinfo.numChannels);
  if (print_temp_moves){
    VG_(printf)("Making %p for result of shadow op.\n",
                result);
  }

  // Get the computed and shadow arguments.
  ShadowTemp* args[4];
  double clientArgs[4][4];
  for(int i = 0; i < opInfo->exinfo.nargs; ++i){
    args[i] = getArg(i,
                     opInfo->exinfo.numChannels,
                     opInfo->exinfo.argPrecision,
                     infoInstance->argTemps[i]);
    for (int j = 0; j < opInfo->exinfo.numChannels; ++j){
      clientArgs[j][i] =
        opInfo->exinfo.argPrecision == Ft_Double ?
        computedArgs.argValues[i][j] :
        computedArgs.argValuesF[i][j];
    }
  }
  for(int i = 0; i < opInfo->exinfo.numSIMDOperands; ++i){
    ShadowValue* vals[opInfo->exinfo.nargs];
    for(int j = 0; j < opInfo->exinfo.nargs; ++j){
      vals[j] = args[j]->values[i];
    }
    double computedOutput =
      (opInfo->exinfo.argPrecision == Ft_Single ?
       computedResult.f[i] : computedResult.d[i]);
    result->values[i] =
      executeChannelShadowOp(opInfo,
                             vals,
                             clientArgs[i],
                             computedOutput);
  }
  for(int i = opInfo->exinfo.numSIMDOperands;
      i < opInfo->exinfo.numChannels; ++i){
    // According to the libvex_ir.h documentation, the non-operated
    // values should be copied from the first operand.
    tl_assert2(args[0]->values[i] != NULL,
               "Value %d of %p (%d vals) is NULL!\n",
               i, args[0], args[0]->num_vals);
    result->values[i] = args[0]->values[i];
    ownNonNullShadowValue(result->values[i]);
  }
  if (PRINT_VALUE_MOVES){
    ppIROp_Extended(opInfo->op_code);
    VG_(printf)(": Making value(s) ");
    for(int i = 0; i < opInfo->exinfo.numSIMDOperands; ++i){
      if (i == 0){
        VG_(printf)("%p", result->values[i]);
      } else {
        VG_(printf)(", %p", result->values[i]);
      }
    }
    if (opInfo->exinfo.numSIMDOperands < opInfo->exinfo.numChannels){
      VG_(printf)(" and copying shadow value(s) ");
      for(int i = opInfo->exinfo.numSIMDOperands;
          i < opInfo->exinfo.numChannels; ++i){
        VG_(printf)("%p (new rc %lu), ",
                    result->values[i], result->values[i]->ref_count);
      }
      VG_(printf)("from %p to %p\n", args[0], result);
    } else {
      VG_(printf)("\n");
    }
  }
  for(int i = 0; i < opInfo->exinfo.nargs; ++i){
    if (infoInstance->argTemps[i] == -1){
      disownShadowTemp_fast(args[i]);
    }
  }
  return result;
}
ShadowTemp* getArg(int argIdx, int numChannels, FloatType argPrecision,
                   IRTemp argTemp){
  if (argTemp == -1 ||
      shadowTemps[argTemp] == NULL ||
      shadowTemps[argTemp]->num_vals != numChannels){
    if (argTemp != -1 && shadowTemps[argTemp] != NULL &&
        shadowTemps[argTemp]->num_vals != numChannels){
      /* VG_(umsg)("WARNING: hit a shadow value mismatch. Bits were treated as different types of floats at different times.\n"); */
      disownShadowTemp(shadowTemps[argTemp]);
      shadowTemps[argTemp] = NULL;
    }
    ShadowTemp* result = mkShadowTemp(numChannels);
    if (PRINT_TEMP_MOVES){
      VG_(printf)("Making shadow temp %p (%d vals) for argument %d\n",
                  result, numChannels, argIdx);
    }
    for(int j = 0; j < numChannels; j++){
      double value = argPrecision == Ft_Double ?
        computedArgs.argValues[argIdx][j] :
        computedArgs.argValuesF[argIdx][j];
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
    /* tl_assert2(shadowTemps[argTemp]->num_vals == numChannels, */
    /*            "Got a temp with the wrong number of shadows for arg %d! " */
    /*            "Expected %d shadows, got %d from temp %d", */
    /*            argIdx, numChannels, shadowTemps[argTemp]->num_vals, */
    /*            argTemp); */
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
      if (getDouble(args[0]->real) == 0 || getDouble(args[1]->real) == 0){
        ShadowValue* result =
          mkShadowValue(opinfo->exinfo.argPrecision, 0);
        execSymbolicOp(opinfo, &(result->expr), result->real, args, False);
        return result;
      }
    default:
      break;
    }
  }
  if (print_inputs){
    for(int i = 0; i < opinfo->exinfo.nargs; ++i){
      VG_(printf)("Arg %d is computed as %f, and is shadowed as %f\n",
                  i + 1, clientArgs[i], getDouble(args[i]->real));
    }
  }
  ShadowValue* result = mkShadowValueBare(opinfo->exinfo.argPrecision);
  execRealOp(opinfo->op_code, &(result->real), args);

  if (print_errors_long || print_errors){
    printOpInfo(opinfo);
    VG_(printf)(":\n");
  }
  if (print_errors_long || print_errors){
    VG_(printf)("Local:\n");
  }
  execLocalOp(opinfo, result->real, result, args);
  if (print_errors_long || print_errors){
    VG_(printf)("Global:");
  }
  double bitsGlobalError =
    updateError(&(opinfo->agg.global_error), result->real, clientResult);
  execSymbolicOp(opinfo, &(result->expr), result->real, args,
                 bitsGlobalError > error_threshold);
  if (print_expr_refs){
    VG_(printf)("Making new expression %p for value %p with 0 references.\n",
                result->expr, result);
  }
  if (use_ranges){
    updateRanges(opinfo->agg.inputs.range_records, clientArgs, opinfo->exinfo.nargs);
  }
  if (print_semantic_ops){
    VG_(printf)("%p = ", result);
    ppIROp(opinfo->op_code);
    switch(opinfo->exinfo.nargs){
    case 0:
      tl_assert(0);
      return NULL;
    case 1:
      VG_(printf)("(%p)\n", args[0]);
      break;
    default:
      VG_(printf)("(%p,", args[0]);
      for(int i = 1; i < opinfo->exinfo.nargs; ++i){
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
      if (getDouble(args[1]->real) == 0){
        ULong inputError = ulpd(getDouble(args[0]->real), clientArgs[0]);
        ULong outputError = ulpd(getDouble(result->real), clientResult);
        if (outputError <= inputError){
          result->influences = cloneInfluences(args[0]->influences);
          return result;
        }
      }
    case Iop_Sub32F0x4:
    case Iop_Sub64F0x2:
    case Iop_SubF64:
    case Iop_SubF32:
      if (getDouble(args[0]->real) == 0){
        ULong inputError = ulpd(getDouble(args[1]->real), clientArgs[1]);
        ULong outputError = ulpd(getDouble(result->real), clientResult);
        if (outputError <= inputError){
          result->influences = cloneInfluences(args[1]->influences);
          return result;
        }
      }
      if (getDouble(args[1]->real) == 0){
        ULong inputError = ulpd(getDouble(args[0]->real), clientArgs[0]);
        ULong outputError = ulpd(getDouble(result->real), clientResult);
        if (outputError <= inputError){
          result->influences = cloneInfluences(args[0]->influences);
          return result;
        }
      }
    default:
      break;
    }
  }
  execInfluencesOp(opinfo, &(result->influences), args);
  return result;
}
