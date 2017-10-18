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
  Ss_Shadowed,
  Ss_Unshadowed,
} ShadowStatus;

typedef struct {
  ValueType vtype;
  ShadowStatus shadowed;
} TypeInfo;

// Meet and join operations for the type lattice
// Cheat sheet: join -> union, meet -> intersect
ValueType typeJoin(ValueType type1, ValueType type2);
ValueType typeMeet(ValueType type1, ValueType type2);

typedef enum {
  Ft_Unknown,
  Ft_NonFloat,
  Ft_Unshadowed,
  Ft_Single,
  Ft_Double
} FloatType;

typedef struct _FloatTypeEntry {
  FloatType type;
  int instrIndexSet;
} FloatTypeEntry;
typedef struct _TSFloatTypeEntry {
  struct _TSFloatTypeEntry* next;
  FloatType type;
  int instrIndexSet;
} TSFloatTypeEntry;

extern VgHashTable* memContext;

void initTypeState(void);
void resetTypeState(void);
void addClearMemTypes(void);
void inferTypes(IRSB* sbIn);

FloatType argPrecision(IROp op_code);
FloatType resultPrecision(IROp op_code);

int isFloatType(IRType type);
int isFloat(IRTypeEnv* env, IRTemp temp);

void ppFloatType(FloatType type);

void setTempType(int tempIdx, int instrIdx, FloatType type);
Bool tempIsTyped(int idx, int instrIdx);
FloatType tempType(int idx, int instrIdx);
FloatType tempEventualType(int idx);
Bool hasStaticShadow(IRExpr* expr, int instrIdx);
Bool hasStaticShadowEventually(IRExpr* expr);
Bool canHaveShadow(IRTypeEnv* tyenv, IRExpr* expr);
Bool canBeFloat(IRTypeEnv* typeEnv, IRExpr* expr);
Bool canStoreShadow(IRTypeEnv* typeEnv, IRExpr* expr);

void setTSType(int idx, int instrIdx, FloatType type);
FloatType tsType(Int tsAddr, int instrIdx);
FloatType inferTSType64(Int tsAddr, int instrIdx);
Bool tsAddrCanHaveShadow(Int tsAddr, int instrIdx);
Bool tsHasStaticShadow(Int tsAddr, int instrIdx);

FloatType inferMemType(ULong addr, int size);
void addClearMemType(void);
void setMemType(ULong addr, FloatType type);
FloatType getMemType(ULong addr);
FloatType lookupMemType(ULong addr);
Bool memAddrCanHaveShadow(ULong memAddr);
Bool memAddrHasStaticShadow(ULong memAddr);
// Keep in mind this block len is in chunks (aka 4-byte units)
Bool memBlockCanHaveShadow(ULong blockStart, int block_len);

int valueSize(IRSB* sbOut, int idx);
int numTempValues(IRSB* sbOut, int idx);
int exprSize(IRTypeEnv* tyenv, IRExpr* expr);
int typeSize(IRType type);
int loadConversionSize(IRLoadGOp conversion);
#endif
