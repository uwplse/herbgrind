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

VG_REGPARM(2) ShadowTemp* executeShadowOp(ShadowOpInfo* opInfo,
                                          ShadowTemp** args){
  ShadowTemp* result = mkShadowTemp(opInfo->exinfo.numChannels);
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
  }
  return result;
}
ShadowValue* executeChannelShadowOp(int nargs,
                                    FloatType type,
                                    IROp op_code,
                                    ShadowValue** args){
  ShadowValue* result = mkShadowValueBare(type);
  result->real = execRealOp(op_code, args);
  return result;
}
