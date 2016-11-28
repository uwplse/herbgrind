/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie              helpers.h ---*/
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

#include "helpers.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "../runtime/shadowop/shadowop.h"

OpArgs argsStruct;

IRStmt* mkGetXG(IRTemp dest_temp,
                IRExpr* arg, IRExpr* guard,
                void* getFunc, const char* getFuncLabel){
  IRDirty* dirty =
    unsafeIRDirty_1_N(dest_temp, 1, getFuncLabel,
                      VG_(fnptr_to_fnentry)(getFunc),
                      mkIRExprVec_1(arg));
  dirty->guard = guard;
  return IRStmt_Dirty(dirty);
}
void addAnd(IRSB* sbOut, IRTemp dest_temp,
           IRTemp arg1_temp, IRTemp arg2_temp){
  IRTemp arg1_temp8 =
    newIRTemp(sbOut->tyenv, Ity_I8);
  IRTemp arg2_temp8 =
    newIRTemp(sbOut->tyenv, Ity_I8);
  addUnop(sbOut, Iop_1Uto8, arg1_temp8, IRExpr_RdTmp(arg1_temp));
  addUnop(sbOut, Iop_1Uto8, arg2_temp8, IRExpr_RdTmp(arg2_temp));
  IRTemp dest_temp8 =
    newIRTemp(sbOut->tyenv, Ity_I8);
  addBinop(sbOut, Iop_And8, dest_temp8,
           IRExpr_RdTmp(arg1_temp8),
           IRExpr_RdTmp(arg2_temp8));
  IRTemp dest_temp32 =
    newIRTemp(sbOut->tyenv, Ity_I32);
  addUnop(sbOut, Iop_8Uto32, dest_temp32, IRExpr_RdTmp(dest_temp8));
  addUnop(sbOut, Iop_32to1, dest_temp, IRExpr_RdTmp(dest_temp32));
}

void addOr(IRSB* sbOut, IRTemp dest_temp,
           IRTemp arg1_temp, IRTemp arg2_temp){
  IRTemp arg1_temp8 =
    newIRTemp(sbOut->tyenv, Ity_I8);
  IRTemp arg2_temp8 =
    newIRTemp(sbOut->tyenv, Ity_I8);
  addUnop(sbOut, Iop_1Uto8, arg1_temp8, IRExpr_RdTmp(arg1_temp));
  addUnop(sbOut, Iop_1Uto8, arg2_temp8, IRExpr_RdTmp(arg2_temp));
  IRTemp dest_temp8 =
    newIRTemp(sbOut->tyenv, Ity_I8);
  addBinop(sbOut, Iop_Or8, dest_temp8,
           IRExpr_RdTmp(arg1_temp8),
           IRExpr_RdTmp(arg2_temp8));
  IRTemp dest_temp32 =
    newIRTemp(sbOut->tyenv, Ity_I32);
  addUnop(sbOut, Iop_8Uto32, dest_temp32, IRExpr_RdTmp(dest_temp8));
  addUnop(sbOut, Iop_32to1, dest_temp, IRExpr_RdTmp(dest_temp32));
}
void addMakeInputG(IRSB* sbOut, IRTemp dest_temp,
                   IRExpr* argExpr, IRTemp guard,
                   FloatType valType){
  IRType argType = typeOfIRExpr(sbOut->tyenv, argExpr);
  if (argType == Ity_F32) {
    IRTemp argDouble = newIRTemp(sbOut->tyenv, Ity_F64);
    addUnop(sbOut, Iop_F32toF64, argDouble, argExpr);
    addGetXG(sbOut, dest_temp, IRExpr_RdTmp(argDouble),
             mkShadowTempOneSingle,
             guard);
  } else if (argType == Ity_I32) {
    IRTemp argSingle = newIRTemp(sbOut->tyenv, Ity_F32);
    addUnop(sbOut, Iop_ReinterpI32asF32, argSingle, argExpr);
    IRTemp argDouble = newIRTemp(sbOut->tyenv, Ity_F64);
    addUnop(sbOut, Iop_F32toF64, argDouble, IRExpr_RdTmp(argSingle));
    IRTemp argBytes = newIRTemp(sbOut->tyenv, Ity_I64);
    addUnop(sbOut, Iop_ReinterpF64asI64,
            argBytes, IRExpr_RdTmp(argDouble));
    addGetXG(sbOut, dest_temp, IRExpr_RdTmp(argBytes),
             mkShadowTempOneSingle,
             guard);
  } else if (argType == Ity_F64 || argType == Ity_I64) {
    addGetXG(sbOut, dest_temp, argExpr, mkShadowTempOneDouble,
             guard);
  } else if (argType == Ity_V128 || argType == Ity_F128 ||
             argType == Ity_I128) {
    if (valType == Ft_Single){
      addStoreGE(sbOut, argExpr,
                 &(argsStruct.values.argValuesF[0]), guard);
      IRStmt* mkShadow =
        mkGetXG(dest_temp,
                mkU64((uintptr_t)argsStruct.values.argValuesF[0]),
                IRExpr_RdTmp(guard),
                mkShadowTempFourSingles,
                "mkShadowTempFourSingles");
      mkShadow->Ist.Dirty.details->mFx = Ifx_Read;
      mkShadow->Ist.Dirty.details->mAddr = mkU64((uintptr_t)argsStruct.values.argValuesF[0]);
      mkShadow->Ist.Dirty.details->mSize = sizeof(float) * 4;
      addStmtToIRSB(sbOut, mkShadow);
    } else {
      addStoreGE(sbOut, argExpr,
                &(argsStruct.values.argValues[0]), guard);
      IRStmt* mkShadow =
        mkGetXG(dest_temp,
                mkU64((uintptr_t)argsStruct.values.argValues[0]),
                IRExpr_RdTmp(guard),
                mkShadowTempTwoDoubles,
                "mkShadowTempTwoDoubles");
      mkShadow->Ist.Dirty.details->mFx = Ifx_Read;
      mkShadow->Ist.Dirty.details->mAddr = mkU64((uintptr_t)argsStruct.values.argValues[0]);
      mkShadow->Ist.Dirty.details->mSize = sizeof(double) * 2;
      addStmtToIRSB(sbOut, mkShadow);
    }
  } else {
    tl_assert(0);
  }
}

VG_REGPARM(1) ShadowTemp* mkShadowTempOneDouble(double val){
  VG_(printf)("Making*\n");
  ShadowTemp* result = newShadowTemp(1);
  result->values[0] = newShadowValue(Ft_Double, val);
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempOneSingle(double val){
  VG_(printf)("Making**\n");
  ShadowTemp* result = newShadowTemp(1);
  result->values[0] = newShadowValue(Ft_Single, val);
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSingles(float* vals){
  VG_(printf)("Making****\n");
  ShadowTemp* result = newShadowTemp(4);
  for(int i = 0; i < 4; ++i){
    result->values[i] = newShadowValue(Ft_Single, vals[i]);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoDoubles(double* vals){
  VG_(printf)("Making*****\n");
  ShadowTemp* result = newShadowTemp(2);
  for(int i = 0; i < 2; ++i){
    result->values[i] = newShadowValue(Ft_Double, vals[i]);
  }
  return result;
}
