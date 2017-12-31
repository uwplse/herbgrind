/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie           floattypes.h ---*/
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

#ifndef _FLOATTYPES_H
#define _FLOATTYPES_H

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_hashtable.h"

#define MAX_TEMPS 1000
#define MAX_REGISTERS 1000

typedef enum {
  Vt_Unknown,
  Vt_NonFloat,
  Vt_UnknownFloat,
  Vt_Double,
  Vt_Single,
} ValueType;

typedef enum {
  Ss_Unknown,
  Ss_Shadowed,
  Ss_Unshadowed,
} ShadowStatus;

typedef struct _TSTypeEntry {
  struct _TSTypeEntry* next;
  ValueType type;
  int instrIndexSet;
} TSTypeEntry;

extern ShadowStatus tempShadowStatus[MAX_TEMPS];
extern ShadowStatus tsShadowStatus[MAX_TEMPS];

// Meet and join operations for the type lattice
// Cheat sheet: join -> union, meet -> intersect
ValueType typeJoin(ValueType type1, ValueType type2);
ValueType typeMeet(ValueType type1, ValueType type2);
const char* typeName(ValueType type);

ValueType constType(const IRConst* constant);

void initTypeState(void);
void resetTypeState(void);
void addClearMemTypes(void);
void inferTypes(IRSB* sbIn);

ValueType argPrecision(IROp op_code);
ValueType conversionArgPrecision(IROp op_code, int argIndex);
ValueType resultPrecision(IROp op_code);

int isFloatType(ValueType type);
int isFloatIRType(IRType type);
int isFloat(IRTypeEnv* env, IRTemp temp);

void ppValueType(ValueType type);

// Returns true if anything was changed, false otherwise
Bool refineTempType(int tempIdx, ValueType type);

ValueType tempType(int idx);
ValueType exprType(IRExpr* expr);
Bool tempIsTyped(int idx, int instrIdx);
Bool canBeFloat(IRTypeEnv* typeEnv, IRExpr* expr);
Bool staticallyFloat(IRExpr* expr);
Bool staticallyShadowed(IRExpr* expr);
Bool canStoreShadow(IRTypeEnv* typeEnv, IRExpr* expr);

// Returns true if anything was changed, false otherwise
Bool setTSType(int idx, int instrIdx, ValueType type);
Bool refineTSType(int tempIdx, int instrIdx, ValueType type);

ValueType tsType(Int tsAddr, int instrIdx);
ValueType inferTSBlockType(int tsAddr, int instrIdx, int size);
Bool tsAddrCanHaveShadow(Int tsAddr, int instrIdx);
Bool tsHasStaticShadow(Int tsAddr, int instrIdx);

int valueSize(IRSB* sbOut, int idx);
int numTempValues(IRSB* sbOut, int idx);
int exprSize(IRTypeEnv* tyenv, IRExpr* expr);
int typeSize(IRType type);
int loadConversionSize(IRLoadGOp conversion);
void printTypeState(void);
#endif
