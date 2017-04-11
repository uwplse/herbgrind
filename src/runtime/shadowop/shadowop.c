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
#include "realop.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "error.h"
#include "symbolic-op.h"
#include "local-op.h"
#include "influence-op.h"
#include "../../helper/ir-info.h"

VG_REGPARM(1) ShadowTemp* executeShadowOp(ShadowOpInfo* opInfo){
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
                     opInfo->argTemps[i]);
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
    if (print_errors_long || print_errors){
      printOpInfo(opInfo);
      VG_(printf)(":\n");
    }
  }
  for(int i = opInfo->exinfo.numSIMDOperands;
      i < opInfo->exinfo.numChannels; ++i){
    // According to the libvex_ir.h documentation, the non-operated
    // values should be copied from the first operand.
    result->values[i] = args[0]->values[i];
    ownNonNullShadowValue(result->values[i]);
  }
  if (PRINT_VALUE_MOVES){
    ppIROp(opInfo->op_code);
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
    if (opInfo->argTemps[i] == -1){
      disownShadowTemp_fast(args[i]);
    }
  }
  return result;
}
ShadowTemp* getArg(int argIdx, int numChannels, FloatType argPrecision,
                   IRTemp argTemp){
  if (argTemp == -1 ||
      shadowTemps[argTemp] == NULL){
    ShadowTemp* result = mkShadowTemp(numChannels);
    for(int j = 0; j < numChannels; j++){
      double value = argPrecision == Ft_Double ?
        computedArgs.argValues[argIdx][j] :
        computedArgs.argValuesF[argIdx][j];
      result->values[j] =
        mkShadowValue(argPrecision, value);
    }
    if (argTemp != -1){
      shadowTemps[argTemp] = result;
    }
    return result;
  } else {
    return shadowTemps[argTemp];
  }
}
ShadowValue* executeChannelShadowOp(ShadowOpInfo* opinfo,
                                    ShadowValue** args,
                                    double* clientArgs,
                                    double clientResult){
  if (!dont_ignore_pure_zeroes){
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
        execSymbolicOp(opinfo, &(result->expr), result->real, args);
        return result;
      }
    default:
      break;
    }
  }
  ShadowValue* result = mkShadowValueBare(opinfo->exinfo.argPrecision);
  execRealOp(opinfo->op_code, &(result->real), args);
  execSymbolicOp(opinfo, &(result->expr), result->real, args);
  execLocalOp(opinfo, result->real, result, args);
  updateError(&(opinfo->eagg), result->real, clientResult);
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
  switch(opinfo->op_code){
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
    if (getDouble(args[0]->real) == 0){
      if (clientArgs[0] != 0.0){
        ULong inputError = ulpd(getDouble(args[1]->real), clientArgs[1]);
        ULong outputError = ulpd(getDouble(result->real), clientResult);
        if (inputError < outputError){
          result->influences = cloneInfluences(args[1]->influences);
          return result;
        }
      }
    }
    if (getDouble(args[1]->real) == 0){
      if (clientArgs[1] != 0.0){
        ULong inputError = ulpd(getDouble(args[0]->real), clientArgs[0]);
        ULong outputError = ulpd(getDouble(result->real), clientResult);
        if (inputError < outputError){
          result->influences = cloneInfluences(args[0]->influences);
          return result;
        }
      }
    }
  default:
    execInfluencesOp(opinfo, &(result->influences), args);
    break;
  }
  return result;
}
