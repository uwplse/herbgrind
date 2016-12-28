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
#include "../runtime/shadowop/shadowop.h"
#include "../helper/instrument-util.h"
#include "../helper/debug.h"

#include "pub_tool_libcprint.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_xarray.h"
#include "pub_tool_libcbase.h"

XArray* tempDebt;
FloatType tyenv[MAX_TEMPS];

void initInstrumentationState(void){
  tempDebt = VG_(newXA)(VG_(malloc), "temp debt array",
                        VG_(free), sizeof(IRTemp));
  initValueShadowState();
}

void instrumentRdTmp(IRSB* sbOut, IRTemp dest, IRTemp src){
  tl_assert2(typeOfIRTemp(sbOut->tyenv, dest) ==
             typeOfIRTemp(sbOut->tyenv, src),
             "Source of temp move doesn't match dest!");
  if (!canHaveShadow(sbOut->tyenv, IRExpr_RdTmp(src))){
    return;
  }
  // Load the new temp into memory.
  IRExpr* newShadowTemp = runLoadTemp(sbOut, src);

  // Copy across the new temp and increment it's ref count.
  // Increment the ref count of the new temp
  IRTemp newShadowTempCopy = newIRTemp(sbOut->tyenv, Ity_I64);
  IRDirty* copyShadowTempDirty =
    unsafeIRDirty_1_N(newShadowTempCopy,
                      1, "copyShadowTemp",
                      VG_(fnptr_to_fnentry)(&copyShadowTemp),
                      mkIRExprVec_1(newShadowTemp));
  copyShadowTempDirty->mFx = Ifx_Read;
  copyShadowTempDirty->mAddr = newShadowTemp;
  copyShadowTempDirty->mSize = sizeof(ShadowTemp);

  IRExpr* tempNonNull = runNonZeroCheck64(sbOut, newShadowTemp);
  copyShadowTempDirty->guard = tempNonNull;
  addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowTempDirty));
  addStoreTempG(sbOut, tempNonNull,
                IRExpr_RdTmp(newShadowTempCopy),
                tyenv[src], dest);
}
void instrumentWriteConst(IRSB* sbOut, IRTemp dest,
                          IRConst* con){
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
void finishInstrumentingBlock(IRSB* sbOut){
  VG_(memset)(tyenv, 0, sizeof tyenv);
  IRTemp* curDebtContents =
    VG_(perm_malloc)(sizeof(IRTemp) * VG_(sizeXA)(tempDebt),
                     vg_alignof(IRTemp));
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    curDebtContents[i] = *(IRTemp*)VG_(indexXA)(tempDebt, i);
  }
  IRDirty* dynCleanupDirty =
    unsafeIRDirty_0_N(2, "dynamicCleanup",
                      dynamicCleanup,
                      mkIRExprVec_2(mkU64(VG_(sizeXA)(tempDebt)),
                                    mkU64((uintptr_t)curDebtContents)));
  dynCleanupDirty->mFx = Ifx_Modify;
  dynCleanupDirty->mAddr = mkU64((uintptr_t)shadowTemps);
  dynCleanupDirty->mSize = sizeof(ShadowTemp) * MAX_TEMPS;
  addStmtToIRSB(sbOut, IRStmt_Dirty(dynCleanupDirty));
  VG_(deleteXA)(tempDebt);
  tempDebt = VG_(newXA)(VG_(malloc), "temp debt array", VG_(free),
                        sizeof(IRTemp));
}
void addBlockCleanupG(IRSB* sbOut, IRExpr* guard){
  IRTemp* curDebtContents =
    VG_(perm_malloc)(sizeof(IRTemp) * VG_(sizeXA)(tempDebt),
                     vg_alignof(IRTemp));
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    curDebtContents[i] = *(IRTemp*)VG_(indexXA)(tempDebt, i);
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
void cleanupAtEndOfBlock(IRSB* sbOut, IRTemp shadowed_temp){
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    IRTemp* prevEntry = VG_(indexXA)(tempDebt, i);
    if (*prevEntry == shadowed_temp){
      return;
    }
  }
  VG_(addToXA)(tempDebt, &shadowed_temp);
}
void addDynamicDisown(IRSB* sbOut, IRTemp idx){
  IRDirty* disownDirty =
    unsafeIRDirty_0_N(1, "disownShadowTempDynamic",
                      VG_(fnptr_to_fnentry)(disownShadowTempDynamic),
                      mkIRExprVec_1(mkU64(idx)));
  disownDirty->mFx = Ifx_Modify;
  disownDirty->mAddr = mkU64((uintptr_t)&(shadowTemps[idx]));
  disownDirty->mSize = sizeof(ShadowTemp*);
  addStmtToIRSB(sbOut, IRStmt_Dirty(disownDirty));
}
void addDynamicDisownNonNull(IRSB* sbOut, IRTemp idx){
  IRDirty* disownDirty =
    unsafeIRDirty_0_N(1, "disownShadowTempNonNullDynamic",
                      VG_(fnptr_to_fnentry)(disownShadowTempNonNullDynamic),
                      mkIRExprVec_1(mkU64(idx)));
  disownDirty->mFx = Ifx_Modify;
  disownDirty->mAddr = mkU64((uintptr_t)&(shadowTemps[idx]));
  disownDirty->mSize = sizeof(ShadowTemp*);
  addStmtToIRSB(sbOut, IRStmt_Dirty(disownDirty));
}
void addDynamicDisownNonNullDetached(IRSB* sbOut, IRExpr* st){
  runPureCCall(sbOut,
               mkIRCallee(1, "disownShadowTemp",
                          VG_(fnptr_to_fnentry)(disownShadowTemp)),
               Ity_I64,
               mkIRExprVec_1(st));
}
void addDisownNonNull(IRSB* sbOut, IRExpr* shadow_temp, int num_vals){
  IRExpr* valuesAddr = runArrow(sbOut, shadow_temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    IRExpr* value = runIndex(sbOut, valuesAddr, ShadowValue*, i);
    addSVDisown(sbOut, value);
  }
  addStackPush(sbOut, freedTemps[num_vals - 1], shadow_temp);
}
void addDisown(IRSB* sbOut, IRExpr* shadow_temp, int num_vals){
  IRExpr* tempNonNull = runNonZeroCheck64(sbOut, shadow_temp);
  IRExpr* valuesAddr =
    runArrowG(sbOut, tempNonNull, shadow_temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    IRExpr* value = runIndexG(sbOut, tempNonNull,
                              valuesAddr,
                              ShadowValue*, i);
    addSVDisownG(sbOut, tempNonNull, value);
  }
  addStackPushG(sbOut, tempNonNull, freedTemps[num_vals - 1],
                shadow_temp);
}
void addDisownG(IRSB* sbOut, IRExpr* guard, IRExpr* shadow_temp, int num_vals){
  IRExpr* valuesAddr =
    runArrowG(sbOut, guard, shadow_temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    IRExpr* value = runIndexG(sbOut, guard,
                              valuesAddr,
                              ShadowValue*, i);
    addSVDisownG(sbOut, guard, value);
  }
  addStackPushG(sbOut, guard, freedTemps[num_vals - 1],
                shadow_temp);
}
int num_freed = 0;
void addSVDisown(IRSB* sbOut, IRExpr* sv){
  IRExpr* valueNonNull = runNonZeroCheck64(sbOut, sv);
  IRExpr* prevRefCount =
    runArrowG(sbOut, valueNonNull, sv, ShadowValue, ref_count);
  IRExpr* lastRef = runBinop(sbOut, Iop_CmpEQ64, prevRefCount, mkU64(1));
  IRExpr* shouldPush = runAnd(sbOut, valueNonNull, lastRef);
  addStackPushG(sbOut, shouldPush, freedVals, sv);

  IRExpr* newRefCount =
    runBinop(sbOut, Iop_Sub64, prevRefCount, mkU64(1));
  IRExpr* shouldUpdateRefCount =
    runAnd(sbOut, valueNonNull,
           runUnop(sbOut, Iop_Not1, lastRef));
  addStoreArrowG(sbOut, shouldUpdateRefCount, sv, ShadowValue,
                 ref_count, newRefCount);
}
void addSVDisownG(IRSB* sbOut, IRExpr* guard, IRExpr* sv){
  IRExpr* valueNonNull = runNonZeroCheck64(sbOut, sv);
  IRExpr* shouldDoAnythingAtAll = runAnd(sbOut, valueNonNull, guard);
  IRExpr* prevRefCount =
    runArrowG(sbOut, shouldDoAnythingAtAll, sv, ShadowValue, ref_count);
  IRExpr* lastRef = runBinop(sbOut, Iop_CmpEQ64, prevRefCount, mkU64(1));
  IRExpr* shouldPush = runAnd(sbOut, shouldDoAnythingAtAll, lastRef);
  addStackPushG(sbOut, shouldPush, freedVals, sv);

  IRExpr* newRefCount =
    runBinop(sbOut, Iop_Sub64, prevRefCount, mkU64(1));
  IRExpr* shouldUpdateRefCount =
    runAnd(sbOut, shouldDoAnythingAtAll,
           runUnop(sbOut, Iop_Not1, lastRef));
  addStoreArrowG(sbOut, shouldUpdateRefCount, sv, ShadowValue,
                 ref_count, newRefCount);
}
void addClear(IRSB* sbOut, IRTemp dest, int num_vals){
  IRExpr* oldShadowTemp = runLoad64C(sbOut, &(shadowTemps[dest]));
  addDisownNonNull(sbOut, oldShadowTemp, num_vals);
  addStoreC(sbOut, mkU64(0), &(shadowTemps[dest]));
}
IRExpr* runMakeInput(IRSB* sbOut, IRExpr* argExpr,
                     FloatType valType, int num_vals){
  IRExpr* result;
  IRType bytesType = typeOfIRExpr(sbOut->tyenv, argExpr);
  if (num_vals == 1){
    IRExpr* argI64 = toDoubleBytes(sbOut, argExpr);;
    if (valType == Ft_Single){
      result = runPureCCall64(sbOut, mkShadowTempOneSingle, argI64);
    } else {
      result = runPureCCall64(sbOut, mkShadowTempOneDouble, argI64);
    }
  } else if (num_vals == 2 && valType == Ft_Double) {
    tl_assert(bytesType == Ity_V128);
    addStoreC(sbOut, argExpr, computedArgs.argValues[0]); 
    result = runPureCCall64(sbOut, mkShadowTempTwoDoubles,
                          mkU64((uintptr_t)computedArgs.argValues[0]));
  } else if (num_vals == 2 && valType == Ft_Single) {
    tl_assert(bytesType == Ity_I64);
    result = runPureCCall64(sbOut, mkShadowTempTwoSingles, argExpr);
  } else if (num_vals == 4) {
    tl_assert(valType == Ft_Single);
    tl_assert(bytesType == Ity_V128);
    addStoreC(sbOut, argExpr, computedArgs.argValues[0]);
    result = runPureCCall64(sbOut, mkShadowTempFourSingles,
                          mkU64((uintptr_t)computedArgs.argValues[0]));
  } else {
    tl_assert2(0, "Hey, you can't have %d vals!\n", num_vals);
  }
  if (canHaveShadow(sbOut->tyenv, argExpr)){
    addStoreTemp(sbOut, result, valType,
                 argExpr->Iex.RdTmp.tmp);
  }
  return result;
}
IRExpr* runMakeInputG(IRSB* sbOut, IRExpr* guard, 
                      IRExpr* argExpr,
                      FloatType valType, int num_vals){
  IRExpr* result;
  IRType bytesType = typeOfIRExpr(sbOut->tyenv, argExpr);
  if (num_vals == 1){
    if (valType == Ft_Single){
      tl_assert(bytesType == Ity_I32);
    } else {
      tl_assert(bytesType == Ity_I64 || bytesType == Ity_F64);
    }
    IRExpr* argI64 = toDoubleBytes(sbOut, argExpr);
    result = runDirtyG_1_1(sbOut, guard,
                           valType == Ft_Single ?
                           (void*)mkShadowTempOneSingle :
                           (void*)mkShadowTempOneDouble,
                           argI64);
  } else if (num_vals == 2 && valType == Ft_Single){
    tl_assert(bytesType == Ity_I64);
    result = runDirtyG_1_1(sbOut, guard, mkShadowTempTwoSingles, argExpr);
  } else if (num_vals == 2 && valType == Ft_Double){ 
    tl_assert(bytesType == Ity_V128);
    addStoreGC(sbOut, guard, argExpr, computedArgs.argValues[0]);
    result = runDirtyG_1_1(sbOut, guard,
                           mkShadowTempTwoDoubles,
                           mkU64((uintptr_t)computedArgs.argValues[0]));
  } else if (num_vals == 4){
    tl_assert(valType == Ft_Single);
    tl_assert(bytesType == Ity_V128);
    addStoreGC(sbOut, guard, argExpr, computedArgs.argValues[0]);
    result = runDirtyG_1_1(sbOut, guard,
                           mkShadowTempFourSingles,
                           mkU64((uintptr_t)computedArgs.argValues[0]));
  } else {
    tl_assert2(0, "Hey, you can't have %d vals!\n", num_vals);
  }
  if (canHaveShadow(sbOut->tyenv, argExpr)){
    addStoreTempG(sbOut, guard, result, valType,
                  argExpr->Iex.RdTmp.tmp);
  }
  return result;
}
IRExpr* runLoadTemp(IRSB* sbOut, int idx){
  return runLoad64C(sbOut, &(shadowTemps[idx]));
}
void addStoreTemp(IRSB* sbOut, IRExpr* shadow_temp,
                  FloatType type,
                  int idx){
  tl_assert2(tyenv[idx] == Ft_Invalid,
             "Tried to set an already set temp %d!\n",
             idx);
  tyenv[idx] = type;
  addStoreC(sbOut, shadow_temp, &(shadowTemps[idx]));
  cleanupAtEndOfBlock(sbOut, idx);
}
void addStoreTempG(IRSB* sbOut, IRExpr* guard,
                   IRExpr* shadow_temp,
                   FloatType type,
                   int idx){
  tl_assert2(tyenv[idx] == Ft_Invalid ||
             tyenv[idx] == type,
             "Tried to conditionally set a"
             " temp already set with a different"
             " type temp!\n");
  addStoreGC(sbOut, guard, shadow_temp, &(shadowTemps[idx]));
  cleanupAtEndOfBlock(sbOut, idx);
}
Bool tempIsTyped(int idx){
  return tyenv[idx] != Ft_Invalid;
}
FloatType tempType(int idx){
  tl_assert2(tyenv[idx] != Ft_Invalid,
             "Tried to get the type of temp %d, "
             "but it hasn't been set yet this SB!\n",
             idx);
  return tyenv[idx];
}
Bool hasStaticShadow(IRExpr* expr){
  switch(expr->tag){
  case Iex_Const:
    return False;
  case Iex_RdTmp:
    return tempIsTyped(expr->Iex.RdTmp.tmp);
  default:
    VG_(printf)("Hey, what are you trying to pull here, man? "
                "You can't check the shadow of a non-trivial "
                "expression. It's got to be either a RdTmp or "
                "a Const, not %p ", expr);
    ppIRExpr(expr);
    tl_assert(0);
  }
}
Bool canHaveShadow(IRTypeEnv* typeEnv, IRExpr* expr){
  if (expr->tag == Iex_Const){
    return False;
  } else if (!isFloatType(typeOfIRExpr(typeEnv, expr))){
    return False;
  } else {
    return True;
  }
}
IRExpr* toDoubleBytes(IRSB* sbOut, IRExpr* floatExpr){
  IRExpr* result;
  IRType bytesType = typeOfIRExpr(sbOut->tyenv, floatExpr);
  switch(bytesType){
  case Ity_F32:
    result = runUnop(sbOut, Iop_ReinterpF64asI64,
                     runUnop(sbOut, Iop_F32toF64, floatExpr));
    break;
  case Ity_I32:
    result = runUnop(sbOut, Iop_ReinterpF64asI64,
                     runUnop(sbOut, Iop_F32toF64,
                             runUnop(sbOut, Iop_ReinterpI32asF32,
                                     floatExpr)));
    break;
  case Ity_F64:
    result = runUnop(sbOut, Iop_ReinterpF64asI64, floatExpr);
    break;
  case Ity_I64:
    result = floatExpr;
    break;
  default:
    tl_assert(0);
  }
  return result;
}
