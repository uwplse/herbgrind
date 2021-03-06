/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie    value-shadowstate.h ---*/
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

// Here are the data structures we set up to hold shadow values. They
// take three forms:
//
// * Values that are being worked on currently are held in temps by
//   VEX, so we have an array of shadow values to match each temp
//   register, up to a limit set in the .h file.
//
// * Values that persist between blocks (I think this is how it
//   works), are held in a per thread data structure by VEX, so we set
//   up another array for every thread to hold those, also up to a
//   limit set in the .h file.
//
// * Finally, values might be written to memory, and then read out
//   later at some arbitrary point. For these, we'll maintain a hash
//   table that maps addresses to shadow values, so we don't have to
//   maintain a vast array of shadow values for all of memory.

#ifndef _VALUE_SHADOWSTATE_H
#define _VALUE_SHADOWSTATE_H

#include "shadowval.h"
#include "exprs.h"
#include "pub_tool_tooliface.h"

#include "pub_tool_libcprint.h"

#include "../../helper/stack.h"

#define MAX_THREADS 16

#define LARGE_PRIME 1572869

typedef struct _tableValueEntry {
  struct _tableValueEntry* next;
  UWord addr;
  ShadowValue* val;
} TableValueEntry;

typedef struct _valueCacheEntry {
  struct _valueCacheEntry* next;
  UWord key;
  ShadowValue* val;
} ValueCacheEntry;

typedef union {
  float argValuesF[4][8];
  double argValues[4][4];
} ArgUnion;

typedef union {
  float f[8];
  double d[4];
} ResultUnion;

extern ArgUnion computedArgs;

extern ResultUnion computedResult;

extern ShadowTemp* shadowTemps[MAX_TEMPS];
extern ShadowValue* shadowThreadState[MAX_THREADS][MAX_REGISTERS];
extern TableValueEntry* shadowMemTable[LARGE_PRIME];

extern Stack* freedTemps[MAX_TEMP_BLOCKS];
extern Stack* freedVals;
extern Stack* tableEntries;
extern VgHashTable* valueCacheSingle;
extern VgHashTable* valueCacheDouble;

typedef struct _Word256 {
  UWord bytes[4];
} Word256;
extern Word256 getBytes;

extern int blockStateDirty;

void initValueShadowState(void);
VG_REGPARM(2) void dynamicCleanup(int nentries, IRTemp* entries);
VG_REGPARM(2) void dynamicPut(Int tsDest, ShadowTemp* st);
VG_REGPARM(2) ShadowTemp* dynamicGet64(Int tsSrc,
                                       UWord tsBytes);
VG_REGPARM(3) ShadowTemp* dynamicGet128(Int tsSrc, UWord bytes1, UWord bytes2);
VG_REGPARM(2) ShadowTemp* dynamicGet256(Int tsSrc, Word256* bytes);
ShadowTemp* dynamicGet(Int tsSrc, void* bytes, int size);
VG_REGPARM(2) ShadowTemp* dynamicLoad(Addr memSrc, FloatBlocks size);
VG_REGPARM(0) TableValueEntry* newTableValueEntry(void);
VG_REGPARM(3) void setMemShadowTemp(Addr64 memDest, UWord size,
                                    ShadowTemp* st);
VG_REGPARM(1) ShadowValue* getMemShadow(Addr64 memSrc);
void removeMemShadow(Addr64 addr);
void addMemShadow(Addr64 addr, ShadowValue* val);

VG_REGPARM(1) void disownShadowTempNonNull(ShadowTemp* temp);
VG_REGPARM(1) void disownShadowTemp(ShadowTemp* temp);
VG_REGPARM(1) ShadowTemp* copyShadowTemp(ShadowTemp* temp);
VG_REGPARM(1) ShadowTemp* deepCopyShadowTemp(ShadowTemp* temp);
VG_REGPARM(1) ShadowValue* toSingle(ShadowValue* v);
VG_REGPARM(1) ShadowValue* toDouble(ShadowValue* v);

ShadowTemp* mkShadowTemp(FloatBlocks num_blocks);
void freeShadowTemp(ShadowTemp* temp);
void disownShadowTemp(ShadowTemp* temp);
VG_REGPARM(1) void disownShadowTempNonNullDynamic(IRTemp idx);
VG_REGPARM(1) void disownShadowTempDynamic(IRTemp idx);
void disownShadowValue(ShadowValue* val);
void ownShadowValue(ShadowValue* val);
void freeShadowValue(ShadowValue* val);
ShadowValue* copyShadowValue(ShadowValue* val);
ShadowValue* mkShadowValueBare(ValueType type);
ShadowValue* mkShadowValue(ValueType type, double value);
VG_REGPARM(2) ShadowValue* mkShadowValue_wrapper(ValueType type, UWord value);

VG_REGPARM(2) ShadowTemp* mkShadowTempDummy(FloatBlocks numBlocks, ValueType valueTypes);
VG_REGPARM(1) ShadowTemp* mkShadowTempOneDouble(UWord value);
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoDoubles(UWord* values);
VG_REGPARM(1) ShadowTemp* mkShadowTempOneSingle(UWord value);
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoSingles(UWord values);
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSingles(float* values);
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSinglesG(UWord guard, float* values);
ShadowValue* getTS(Int idx);
VG_REGPARM(2) void printStoreValue(const char* dest_label, ShadowValue* val);
void printStoreValueF(ShadowValue* val, const char* format, ...);

inline ShadowValue* mkShadowValueBare_fast(ValueType type);
inline ShadowValue* mkShadowValue_fast(ValueType type, double value);
inline void freeShadowValue_fast(ShadowValue* val);
inline void freeShadowTemp_fast(ShadowTemp* temp);
inline void disownNonNullShadowValue(ShadowValue* val);
inline void ownNonNullShadowValue(ShadowValue* val);
inline void disownShadowTemp_fast(ShadowTemp* temp);

__attribute__((always_inline))
inline
ShadowValue* mkShadowValueBare_fast(ValueType type){
  ShadowValue* result;
  if (stack_empty_fast(freedVals)){
    result = newShadowValue(type);
  } else {
    result = (void*)stack_pop_fast(freedVals);
    result->type = type;
  }
  result->ref_count = 1;
  return result;
}
__attribute__((always_inline))
inline
ShadowValue* mkShadowValue_fast(ValueType type, double value){
  ShadowValue* result = mkShadowValueBare(type);
  setReal_fast(result->real, value);
  return result;
}
__attribute__((always_inline))
inline
void freeShadowValue_fast(ShadowValue* val){
  stack_push_fast(freedVals, (void*)val);
}
__attribute__((always_inline))
inline
void freeShadowTemp_fast(ShadowTemp* temp){
  stack_push_fast(freedTemps[INT(temp->num_blocks)- 1], (void*)temp);
}
__attribute__((always_inline))
inline
void disownNonNullShadowValue(ShadowValue* val){
  val->ref_count--;
  if (val->ref_count == 0){
    freeShadowValue(val);
  }
}
__attribute__((always_inline))
inline
void ownNonNullShadowValue(ShadowValue* val){
  (val->ref_count)++;
}
__attribute__((always_inline))
inline
void disownShadowTemp_fast(ShadowTemp* temp){
  for(int i = 0; i < INT(temp->num_blocks); ++i){
    disownShadowValue(temp->values[i]);
  }
  freeShadowTemp_fast(temp);
}
#endif
