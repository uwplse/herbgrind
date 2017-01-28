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
#ifndef _INSTRUMENT_STORAGE_H
#define _INSTRUMENT_STORAGE_H

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "../helper/ir-info.h"

void initInstrumentationState(void);
void instrumentRdTmp(IRSB* sbOut, IRTemp dest, IRTemp src);
void instrumentWriteConst(IRSB* sbOut, IRTemp dest,
                          IRConst* con);
void instrumentITE(IRSB* sbOut, IRTemp dest,
                   IRExpr* trueExpr, IRExpr* falseExpr);
void instrumentPut(IRSB* sbOut, Int tsDest, IRExpr* data);
void instrumentPutI(IRSB* sbOut,
                    IRExpr* varOffset, Int constOffset,
                    Int arrayBase, Int numElems, IRType elemType,
                    IRExpr* data);
void instrumentGet(IRSB* sbOut, IRTemp dest,
                   Int tsSrc, IRType type);
void instrumentGetI(IRSB* sbOut, IRTemp dest,
                    IRExpr* varOffset, Int constOffset,
                    Int arrayBase, Int numElems, IRType elemType);
void instrumentLoad(IRSB* sbOut, IRTemp dest,
                    IRExpr* addr, IRType type);
void instrumentLoadG(IRSB* sbOut, IRTemp dest,
                     IRExpr* altValue, IRExpr* guard,
                     IRExpr* addr, IRLoadGOp conversion);
void instrumentStore(IRSB* sbOut, IRExpr* addr,
                     IRExpr* data);
void instrumentStoreG(IRSB* sbOut, IRExpr* addr,
                      IRExpr* guard, IRExpr* data);
void instrumentCAS(IRSB* sbOut,
                   IRCAS* details);
void finishInstrumentingBlock(IRSB* sbOut);
void addBlockCleanupG(IRSB* sbOut, IRExpr* guard);
void cleanupAtEndOfBlock(IRSB* sbOut, IRTemp shadowed_temp);
void addDynamicDisown(IRSB* sbOut, IRTemp idx);
void addDynamicDisownNonNull(IRSB* sbOut, IRTemp idx);
void addDynamicDisownNonNullDetached(IRSB* sbOut, IRExpr* st);
void addDisownNonNull(IRSB* sbOut, IRExpr* shadow_temp, int num_vals);
void addDisown(IRSB* sbOut, IRExpr* shadow_temp, int num_vals);
void addDisownG(IRSB* sbOut, IRExpr* guard, IRExpr* shadow_temp, int num_vals);
void addSVDisown(IRSB* sbOut, IRExpr* sv);
void addSVDisownNonNull(IRSB* sbOut, IRExpr* sv);
void addSVDisownG(IRSB* sbOut, IRExpr* guard, IRExpr* sv);
void addSVOwn(IRSB* sbOut, IRExpr* sv);
void addSVOwnNonNullG(IRSB* sbOut, IRExpr* guard, IRExpr* sv);
void addSVOwnNonNull(IRSB* sbOut, IRExpr* sv);
void addClear(IRSB* sbOut, IRTemp shadowed_temp, int num_vals);

IRExpr* runMkShadowTempValues(IRSB* sbOut, int num_values,
                              IRExpr** values);
IRExpr* runMkShadowVal(IRSB* sbOut, FloatType type, IRExpr* valExpr);
IRExpr* runMkShadowValG(IRSB* sbOut, IRExpr* guard,
                        FloatType type, IRExpr* valExpr);
IRExpr* runMakeInputG(IRSB* sbOut, IRExpr* guard,
                      IRExpr* argExpr,
                      FloatType type, int num_vals);
IRExpr* runMakeInput(IRSB* sbOut, IRExpr* argExpr,
                     FloatType type, int num_vals);

IRExpr* runLoadTemp(IRSB* sbOut, int idx);
IRExpr* runGetTSVal(IRSB* sbOut, Int tsSrc);
IRExpr* runGetTSValDynamic(IRSB* sbOut, IRExpr* tsSrc);
void addSetTSValNonNull(IRSB* sbOut, Int tsDest,
                        IRExpr* newVal, FloatType floatType);
void addSetTSValNonFloat(IRSB* sbOut, Int tsDest);
void addSetTSValUnshadowed(IRSB* sbOut, Int tsDest);
void addSetTSValUnknown(IRSB* sbOut, Int tsDest, IRExpr* newVal);
void addSetTSVal(IRSB* sbOut, Int tsDest, IRExpr* newVal);
void addSetTSValDynamic(IRSB* sbOut, IRExpr* tsDest, IRExpr* newVal);
void addStoreTemp(IRSB* sbOut, IRExpr* shadow_temp,
                  FloatType type,
                  int idx);
void addStoreTempG(IRSB* sbOut, IRExpr* guard,
                   IRExpr* shadow_temp,
                   FloatType type,
                   int idx);
void addStoreTempNonFloat(IRSB* sbOut, int idx);
void addStoreTempUnknown(IRSB* sbOut, IRExpr* shadow_temp_maybe, int idx);
void addStoreTempUnshadowed(IRSB* sbOut, int idx);
Bool tempIsTyped(int idx);
FloatType tempType(int idx);
FloatType inferTSType64(Int tsAddr);
Bool hasStaticShadow(IRExpr* expr);
Bool canHaveShadow(IRTypeEnv* tyenv, IRExpr* expr);
Bool canBeFloat(IRTypeEnv* typeEnv, IRExpr* expr);
Bool canStoreShadow(IRTypeEnv* typeEnv, IRExpr* expr);
IRExpr* toDoubleBytes(IRSB* sbOut, IRExpr* floatExpr);
int valueSize(IRSB* sbOut, int idx);
int numTempValues(IRSB* sbOut, int idx);
int exprSize(IRTypeEnv* tyenv, IRExpr* expr);
int typeSize(IRType type);
Bool tsAddrCanHoldShadow(Int tsAddr);
Bool tsHasStaticShadow(Int tsAddr);
IRExpr* mkArrayLookupExpr(IRSB* sbOut,
                          Int base, IRExpr* idx,
                          Int bias, Int len,
                          IRType elemSize);
#endif
