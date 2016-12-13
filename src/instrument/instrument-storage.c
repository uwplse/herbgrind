/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie   instrument-storage.c ---*/
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

#include "instrument-storage.h"
#include "../runtime/value-shadowstate/value-shadowstate.h"
#include "../helper/instrument-util.h"
#include "../helper/debug.h"

#include "pub_tool_libcprint.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_xarray.h"

XArray* tempDebt;

void initInstrumentationState(void){
  tempDebt = VG_(newXA)(VG_(malloc), "temp debt array",
                        VG_(free), sizeof(TempDebtEntry));
}

void instrumentRdTmp(IRSB* sbOut, IRTemp dest, IRTemp src){
  tl_assert2(typeOfIRTemp(sbOut->tyenv, dest) ==
             typeOfIRTemp(sbOut->tyenv, src),
             "Source of temp move doesn't match dest!");
  return;
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }

  // Load the new temp into memory.
  IRTemp newShadowTemp = runLoad64C(sbOut, &(shadowTemps[src]));

  // Copy across the new temp and increment it's ref count.
  // Increment the ref count of the new temp
  IRTemp newTempNonNull = runNonZeroCheck64(sbOut, newShadowTemp);

  IRTemp newShadowTempCopy = newIRTemp(sbOut->tyenv, Ity_I64);
  IRDirty* copyShadowTempDirty =
    unsafeIRDirty_1_N(newShadowTempCopy,
                      1, "copyShadowTemp",
                      VG_(fnptr_to_fnentry)(&copyShadowTemp),
                      mkIRExprVec_1(IRExpr_RdTmp(newShadowTemp)));
  copyShadowTempDirty->mFx = Ifx_Read;
  copyShadowTempDirty->mAddr = IRExpr_RdTmp(newShadowTemp);
  copyShadowTempDirty->mSize = sizeof(ShadowTemp);
  copyShadowTempDirty->guard = IRExpr_RdTmp(newTempNonNull);
  addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowTempDirty));

  IRTemp newTemp = runITE(sbOut, newTempNonNull, IRExpr_RdTmp(newShadowTempCopy), mkU64(0));
  addStoreC(sbOut, IRExpr_RdTmp(newTemp), &(shadowTemps[dest]));
}
void instrumentWriteConst(IRSB* sbOut, IRTemp dest,
                          IRConst* con){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
}
void instrumentITE(IRSB* sbOut, IRTemp dest,
                   IRExpr* trueExpr, IRExpr* falseExpr){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
}
void instrumentPut(IRSB* sbOut, Int tsDest, IRExpr* data){
}
void instrumentPutI(IRSB* sbOut,
                    IRExpr* varOffset, Int constOffset,
                    Int arrayBase, Int numElems, IRType elemType,
                    IRExpr* data){
}
void instrumentGet(IRSB* sbOut, IRTemp dest,
                   Int tsSrc, IRType type){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
}
void instrumentGetI(IRSB* sbOut, IRTemp dest,
                    IRExpr* varOffset, Int constOffset,
                    Int arrayBase, Int numElems, IRType elemType){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
}
void instrumentLoad(IRSB* sbOut, IRTemp dest,
                    IRExpr* addr, IRType type){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
}
void instrumentLoadG(IRSB* sbOut, IRTemp dest,
                     IRExpr* altValue, IRExpr* guard,
                     IRExpr* addr, IRLoadGOp conversion){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
}
void instrumentStore(IRSB* sbOut, IRExpr* addr,
                     IRExpr* data){
}
void instrumentStoreG(IRSB* sbOut, IRExpr* addr,
                      IRExpr* guard, IRExpr* data){
}
void instrumentCAS(IRSB* sbOut,
                   IRCAS* details){
}
void addBlockCleanup(IRSB* sbOut){
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    TempDebtEntry* entry = VG_(indexXA)(tempDebt, i);
    addClear(sbOut, entry->temp, entry->num_vals);
  }
  VG_(deleteXA)(tempDebt);
  tempDebt = VG_(newXA)(VG_(malloc), "temp debt array", VG_(free),
                        sizeof(TempDebtEntry));
}
void addBlockCleanupG(IRSB* sbOut, IRExpr* guard){
  TempDebtEntry* curDebtContents =
    VG_(perm_malloc)(sizeof(TempDebtEntry) * VG_(sizeXA)(tempDebt),
                     vg_alignof(TempDebtEntry));
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    curDebtContents[i] = *(TempDebtEntry*)VG_(indexXA)(tempDebt, i);
  }
  IRDirty* dynCleanupDirty =
    unsafeIRDirty_0_N(2, "dynamicCleanup",
                      dynamicCleanup,
                      mkIRExprVec_2(mkU64(VG_(sizeXA)(tempDebt)),
                                    mkU64((uintptr_t)curDebtContents)));
  dynCleanupDirty->guard = guard;
  dynCleanupDirty->mFx = Ifx_Modify;
  dynCleanupDirty->mAddr = mkU64((uintptr_t)shadowTemps);
  dynCleanupDirty->mSize = sizeof(ShadowTemp) * MAX_TEMPS;
  addStmtToIRSB(sbOut, IRStmt_Dirty(dynCleanupDirty));
}
void cleanupAtEndOfBlock(IRSB* sbOut, IRTemp shadow_temp, int num_vals){
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    TempDebtEntry* prevEntry = VG_(indexXA)(tempDebt, i);
    if (prevEntry->temp == shadow_temp){
      return;
    }
  }
  TempDebtEntry entry;
  entry.temp = shadow_temp;
  entry.num_vals = num_vals;
  VG_(addToXA)(tempDebt, &entry);
}
void addDisownNonNull(IRSB* sbOut, IRTemp temp, int num_vals){
  IRTemp valuesAddr =
    runArrow(sbOut, temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    IRTemp value = runIndex(sbOut, IRExpr_RdTmp(valuesAddr),
                            ShadowValue*, i);
    IRTemp valueNonNull = runNonZeroCheck64(sbOut, value);
    addStackPushG(sbOut, valueNonNull,
                  freedVals, value);
  }
  addStackPush(sbOut, freedTemps[num_vals - 1], temp);
}
void addDisown(IRSB* sbOut, IRTemp temp, int num_vals){
  IRTemp tempNonNull = runNonZeroCheck64(sbOut, temp);
  IRTemp valuesAddr =
    runArrowG(sbOut, tempNonNull,
              temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    IRTemp value = runIndexG(sbOut, tempNonNull,
                             IRExpr_RdTmp(valuesAddr),
                             ShadowValue*, i);
    IRTemp valueNonNull = runNonZeroCheck64(sbOut, value);
    IRTemp shouldPush = runAnd(sbOut, tempNonNull, valueNonNull);
    addStackPushG(sbOut, shouldPush,
                  freedVals, value);
  }
  addStackPushG(sbOut, tempNonNull, freedTemps[num_vals - 1], temp);
}
void addClear(IRSB* sbOut, IRTemp dest, int num_vals){
  IRTemp oldShadowTemp = runLoad64C(sbOut, &(shadowTemps[dest]));
  addDisownNonNull(sbOut, oldShadowTemp, num_vals);
  addStoreC(sbOut, mkU64(0), &(shadowTemps[dest]));
}
IRTemp runNewShadowTempG(IRSB* sbOut, IRTemp guard,
                         int num_vals){
  return runDirtyG_1_1(sbOut, guard, mkShadowTemp, mkU64(num_vals));



  IRTemp stackEmpty = runStackEmpty(sbOut, freedTemps[num_vals - 1]);
  IRTemp shouldMakeNew = runAnd(sbOut, guard, stackEmpty);
  IRTemp freshTemp = runDirtyG_1_1(sbOut, shouldMakeNew,
                                   newShadowTemp, mkU64(num_vals));
  IRTemp shouldPop = runAnd(sbOut, guard,
                            runUnopT(sbOut, Iop_Not1, stackEmpty));
  IRTemp poppedTemp = runStackPopG(sbOut, shouldPop,
                                   freedTemps[num_vals - 1]);
  IRTemp resultTemp = runITE(sbOut, stackEmpty, IRExpr_RdTmp(freshTemp), IRExpr_RdTmp(poppedTemp));
  return resultTemp;
}
IRTemp runNewShadowTemp(IRSB* sbOut, int num_vals){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest,
                                    IRExpr_CCall(mkIRCallee(1, "mkShadowTemp",
                                                            VG_(fnptr_to_fnentry(mkShadowTemp))),
                                                 Ity_I64,
                                                 mkIRExprVec_1(mkU64(num_vals)))));
  return dest;




  IRTemp stackEmpty = runStackEmpty(sbOut, freedTemps[num_vals - 1]);
  IRTemp freshTemp = runDirtyG_1_1(sbOut, stackEmpty,
                                   newShadowTemp, mkU64(num_vals));
  IRTemp poppedTemp = runStackPopG(sbOut,
                                   runUnop(sbOut, Iop_Not1, IRExpr_RdTmp(stackEmpty)),
                                   freedTemps[num_vals - 1]);
  IRTemp resultTemp = runITE(sbOut, stackEmpty, IRExpr_RdTmp(freshTemp), IRExpr_RdTmp(poppedTemp));
  return resultTemp;
}
IRTemp runMkShadowTempG(IRSB* sbOut, IRTemp guard,
                        int num_vals, FloatType valPrecision,
                        IRExpr* valExpr){
  IRTemp temp = runNewShadowTempG(sbOut, guard, num_vals);
  IRTemp tempValues = runArrowG(sbOut, guard, temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    addStoreIndexG(sbOut, guard, IRExpr_RdTmp(tempValues),
                   ShadowValue*, i, mkU64(0));
  }
  return temp;
}
IRTemp runMkShadowTemp(IRSB* sbOut,
                       int num_vals, FloatType valPecision,
                       IRExpr* valExpr){
  IRTemp temp = runNewShadowTemp(sbOut, num_vals);
  IRTemp tempValues = runArrow(sbOut, temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    addStoreIndex(sbOut, IRExpr_RdTmp(tempValues),
                  ShadowValue*, i, mkU64(0));
  }
  return temp;
}

IRTemp runMakeInput(IRSB* sbOut, 
                    IRExpr* argExpr,
                    FloatType valType){
  IRType argType = typeOfIRExpr(sbOut->tyenv, argExpr);
  if (argType == Ity_F32 || argType == Ity_I32) {
    IRTemp argDouble;
    if (argType == Ity_F32){
      argDouble = runUnop(sbOut, Iop_F32toF64, argExpr);
    } else {
      IRTemp argSingle = runUnop(sbOut, Iop_ReinterpI32asF32, argExpr);
      argDouble = runUnopT(sbOut, Iop_F32toF64, argSingle);
    }
    IRTemp argBytes = runUnop(sbOut, Iop_ReinterpF64asI64,
                              IRExpr_RdTmp(argDouble));
    return runMkShadowTemp(sbOut, 1, Ft_Single, IRExpr_RdTmp(argBytes));
  } else if (argType == Ity_F64 || argType == Ity_I64) {
    IRExpr* argBytes;
    if (argType == Ity_F64){
      argBytes = IRExpr_RdTmp(runUnop(sbOut, Iop_ReinterpF64asI64,
                                      argExpr));
    } else {
      argBytes = argExpr;
    }
    return runMkShadowTemp(sbOut, 1, Ft_Double, argBytes);
  } else if (argType == Ity_V128 || argType == Ity_F128 ||
             argType == Ity_I128){
    if (valType == Ft_Single){
      return runMkShadowTemp(sbOut, 4, Ft_Single, argExpr);
    } else {
      return runMkShadowTemp(sbOut, 2, Ft_Double, argExpr);
    }
  } else {
    tl_assert(0);
  }
}
IRTemp runMakeInputG(IRSB* sbOut, IRTemp guard,
                     IRExpr* argExpr, FloatType valType){
  IRType argType = typeOfIRExpr(sbOut->tyenv, argExpr);
  if (argType == Ity_F32 || argType == Ity_I32) {
    IRTemp argDouble;
    if (argType == Ity_F32){
      argDouble = runUnop(sbOut, Iop_F32toF64, argExpr);
    } else {
      IRTemp argSingle = runUnop(sbOut, Iop_ReinterpI32asF32, argExpr);
      argDouble = runUnopT(sbOut, Iop_F32toF64, argSingle);
    }
    IRTemp argBytes = runUnopT(sbOut, Iop_ReinterpF64asI64, argDouble);
    return runMkShadowTempG(sbOut, guard, 1, Ft_Single, IRExpr_RdTmp(argBytes));
  } else if (argType == Ity_F64 || argType == Ity_I64) {
    IRExpr* argBytes;
    if (argType == Ity_F64){
      argBytes = IRExpr_RdTmp(runUnop(sbOut, Iop_ReinterpF64asI64,
                                      argExpr));
    } else {
      argBytes = argExpr;
    }
    return runMkShadowTempG(sbOut, guard, 1, Ft_Double, argBytes);
  } else if (argType == Ity_V128 || argType == Ity_F128 ||
             argType == Ity_I128) {
    if (valType == Ft_Single){
      return runMkShadowTempG(sbOut, guard, 4, Ft_Single, argExpr);
    } else {
      return runMkShadowTempG(sbOut, guard, 2, Ft_Double, argExpr);
    }
  } else {
    tl_assert(0);
    return 0;
  }
}
