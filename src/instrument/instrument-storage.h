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
void addDisownNonNull(IRSB* sbOut, IRExpr* shadow_temp, int num_vals);
void addDisown(IRSB* sbOut, IRExpr* shadow_temp, int num_vals);
void addDisownG(IRSB* sbOut, IRExpr* guard, IRExpr* shadow_temp, int num_vals);
void addSVDisown(IRSB* sbOut, IRExpr* sv);
void addSVDisownG(IRSB* sbOut, IRExpr* guard, IRExpr* sv);
void addClear(IRSB* sbOut, IRTemp shadowed_temp, int num_vals);

IRExpr* runMakeInputG(IRSB* sbOut, IRExpr* guard,
                      IRExpr* argExpr,
                      FloatType type, int num_vals);
IRExpr* runMakeInput(IRSB* sbOut, IRExpr* argExpr,
                     FloatType type, int num_vals);

IRExpr* runLoadTemp(IRSB* sbOut, int idx);
void addStoreTemp(IRSB* sbOut, IRExpr* shadow_temp,
                  FloatType type,
                  int idx);
void addStoreTempG(IRSB* sbOut, IRExpr* guard,
                   IRExpr* shadow_temp,
                   FloatType type,
                   int idx);
Bool tempIsTyped(int idx);
FloatType tempType(int idx);
Bool hasStaticShadow(IRExpr* expr);
Bool canHaveShadow(IRTypeEnv* tyenv, IRExpr* expr);
IRExpr* toDoubleBytes(IRSB* sbOut, IRExpr* floatExpr);

/* IRExpr* runNewShadowTempG(IRSB* sbOut, IRExpr* guard, */
/*                           int num_vals); */
/* IRExpr* runNewShadowTemp(IRSB* sbOut, int num_vals); */
/* IRExpr* runMkShadowTempG(IRSB* sbOut, IRExpr* guard, */
/*                          int num_vals, FloatType valPrecision, */
/*                          IRExpr* valExpr); */
/* IRExpr* runMkShadowTemp(IRSB* sbOut, */
/*                         int num_vals, FloatType valPecision, */
/*                         IRExpr* valExpr); */
/* IRExpr* runMkShadowValue(IRSB* sbOut, */
/*                          FloatType type, */
/*                          IRExpr* doubleExpr); */
/* IRExpr* runMkShadowValueG(IRSB* sbOut, IRExpr* guard, */
/*                           FloatType type, */
/*                           IRExpr* doubleExpr); */
#endif
