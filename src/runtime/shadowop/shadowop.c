/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie             shadowop.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
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

VG_REGPARM(1) ShadowTemp* executeShadowOp(ShadowOpInfo* opInfo){
  tl_assert(opInfo->op_code < Iop_LAST);
  ShadowTemp* result = mkShadowTemp(opInfo->exinfo.numChannels);
  if (print_temp_moves){
    VG_(printf)("Making %p for result of shadow op.\n",
                result);
  }
  ShadowTemp* args[4];
  for(int i = 0; i < opInfo->exinfo.nargs; ++i){
    if (opInfo->argTemps[i] == -1 ||
        shadowTemps[opInfo->argTemps[i]] == NULL){
      args[i] = mkShadowTemp(opInfo->exinfo.numChannels);
      for(int j = 0; j < opInfo->exinfo.numChannels; j++){
        args[i]->values[j] =
          mkShadowValue(opInfo->exinfo.argPrecision,
                        opInfo->exinfo.argPrecision == Ft_Double ?
                        computedArgs.argValues[i][j] :
                        computedArgs.argValuesF[i][j]);
      }
      if (opInfo->argTemps[i] != -1){
        shadowTemps[opInfo->argTemps[i]] = args[i];
      }
    } else {
      args[i] = shadowTemps[opInfo->argTemps[i]];
      tl_assert(args[i]->num_vals == opInfo->exinfo.numChannels);
    }
  }
  for(int i = 0; i < opInfo->exinfo.numSIMDOperands; ++i){
    ShadowValue* vals[opInfo->exinfo.nargs];
    for(int j = 0; j < opInfo->exinfo.nargs; ++j){
      vals[j] = args[j]->values[i];
    }
    result->values[i] =
      executeChannelShadowOp(opInfo->exinfo.nargs,
                             opInfo->exinfo.argPrecision,
                             opInfo,
                             vals);
    if (print_errors_long || print_errors){
      printOpInfo(opInfo);
      VG_(printf)(":\n");
    }
    updateError(&(opInfo->eagg), result->values[i]->real,
                (opInfo->exinfo.argPrecision == Ft_Single ?
                 computedResult.f[i] : computedResult.d[i]));
  }
  for(int i = opInfo->exinfo.numSIMDOperands;
      i < opInfo->exinfo.numChannels; ++i){
    // According to the libvex_ir.h documentation, the non-operated
    // values should be copied from the first operand.
    result->values[i] = args[0]->values[i];
    ownShadowValue(result->values[i]);
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
  return result;
}
ShadowValue* executeChannelShadowOp(int nargs,
                                    FloatType type,
                                    ShadowOpInfo* opinfo,
                                    ShadowValue** args){
  ShadowValue* result = mkShadowValueBare(type);
  execRealOp(opinfo->op_code, &(result->real), args);
  execSymbolicOp(opinfo, &(result->expr), result->real, args);
  execLocalOp(opinfo, result->real, result, args);
  execInfluencesOp(opinfo, &(result->influences), args);
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
  return result;
}
