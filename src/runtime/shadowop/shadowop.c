/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie             shadowop.c ---*/
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

#include "shadowop.h"
#include "../value-shadowstate/value-shadowstate.h"
#include "realop.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"

VG_REGPARM(2) ShadowTemp* executeShadowOp(ShadowOpInfo* opInfo,
                                          ShadowTemp** args){
  tl_assert(opInfo->op_code < Iop_LAST);
  ShadowTemp* result = mkShadowTemp(opInfo->exinfo.numChannels);
  if (print_value_moves){
    ppIROp(opInfo->op_code);
    VG_(printf)(": Making value(s) ");
  }
  if (print_temp_moves){
    VG_(printf)("Making %p for result of shadow op.\n",
                result);
  }
  for(int i = 0; i < opInfo->exinfo.numSIMDOperands; ++i){
    ShadowValue* vals[opInfo->exinfo.nargs];
    for(int j = 0; j < opInfo->exinfo.nargs; ++j){
      vals[j] = args[j]->values[i];
    }
    result->values[i] =
      executeChannelShadowOp(opInfo->exinfo.nargs,
                             opInfo->exinfo.argPrecision,
                             opInfo->op_code,
                             vals);
    if (print_value_moves){
      if (i == 0){
        VG_(printf)("%p", result->values[i]);
      } else {
        VG_(printf)(", %p", result->values[i]);
      }
    }
  }
  if (opInfo->exinfo.numSIMDOperands < opInfo->exinfo.numChannels &&
      print_value_moves){
    VG_(printf)(" and copying shadow value(s) ");
  }
  for(int i = opInfo->exinfo.numSIMDOperands;
      i < opInfo->exinfo.numChannels; ++i){
    // According to the libvex_ir.h documentation, the non-operated
    // values should be copied from the first operand.
    result->values[i] = args[0]->values[i];
    ownShadowValue(result->values[i]);
    if (print_value_moves){
      VG_(printf)("%p (new rc %lu), ",
                  result->values[i], result->values[i]->ref_count);
    }
  }
  if (print_value_moves){
    if (opInfo->exinfo.numSIMDOperands < opInfo->exinfo.numChannels){
      VG_(printf)("from %p to %p\n", args[0], result);
    } else {
      VG_(printf)("\n");
    }
  }
  tl_assert(opInfo->op_code < Iop_LAST);
  return result;
}
ShadowValue* executeChannelShadowOp(int nargs,
                                    FloatType type,
                                    IROp op_code,
                                    ShadowValue** args){
  ShadowValue* result = mkShadowValueBare(type);
  if (print_value_moves){
    VG_(printf)("Getting new shadow value %p for result of op ", result);
    ppIROp(op_code);
    VG_(printf)("\n");
  }
  execRealOp(op_code, &(result->real), args);
  return result;
}
