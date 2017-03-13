/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie    value-shadowstate.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
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

#include "../../helper/stack.h"

#define MAX_THREADS 16

#define LARGE_PRIME 1973

typedef struct _shadowMemEntry {
  struct _shadowMemEntry* next;
  UWord addr;
  ShadowValue* val;
} ShadowMemEntry;

extern ShadowTemp* shadowTemps[MAX_TEMPS];
extern ShadowValue* shadowThreadState[MAX_THREADS][MAX_REGISTERS];
extern ShadowMemEntry* shadowMemTable[LARGE_PRIME];

#define MAX_TEMP_SHADOWS 4

extern Stack* freedTemps[MAX_TEMP_SHADOWS];
extern Stack* freedVals;

void initValueShadowState(void);
VG_REGPARM(2) void dynamicCleanup(int nentries, IRTemp* entries);
VG_REGPARM(2) void dynamicPut64(Int tsDest, ShadowTemp* st);
VG_REGPARM(2) void dynamicPut128(Int tsDest, ShadowTemp* st);
VG_REGPARM(2) ShadowTemp* dynamicGet64(Int tsSrc,
                                       UWord tsBytes);
VG_REGPARM(3) ShadowTemp* dynamicGet128(Int tsSrc,
                                        UWord bytes1, UWord bytes2);
VG_REGPARM(2) ShadowTemp* dynamicLoad32(UWord memSrc);
VG_REGPARM(2) ShadowTemp* dynamicLoad64(UWord memSrc, UWord memBytes);
VG_REGPARM(3) ShadowTemp* dynamicLoad128(UWord memSrc,
                                         UWord bytes1, UWord bytes2);
VG_REGPARM(3) void setMemShadowTemp(Addr64 memDest, UWord size,
                                    ShadowTemp* st);
VG_REGPARM(1) ShadowValue* getMemShadow(Addr64 memSrc);
void removeMemShadow(Addr64 addr);
void addMemShadow(Addr64 addr, ShadowValue* val);

VG_REGPARM(1) void disownShadowTempNonNull(ShadowTemp* temp);
VG_REGPARM(1) void disownShadowTemp(ShadowTemp* temp);
VG_REGPARM(1) ShadowTemp* copyShadowTemp(ShadowTemp* temp);
VG_REGPARM(1) ShadowTemp* deepCopyShadowTemp(ShadowTemp* temp);

ShadowTemp* mkShadowTemp(UWord num_vals);
void freeShadowTemp(ShadowTemp* temp);
void disownShadowTemp(ShadowTemp* temp);
VG_REGPARM(1) void disownShadowTempNonNullDynamic(IRTemp idx);
VG_REGPARM(1) void disownShadowTempDynamic(IRTemp idx);
void disownShadowValue(ShadowValue* val);
void ownShadowValue(ShadowValue* val);
void freeShadowValue(ShadowValue* val);
ShadowValue* copyShadowValue(ShadowValue* val);
ShadowValue* mkShadowValueBare(FloatType type);
ShadowValue* mkShadowValue(FloatType type, double value);

VG_REGPARM(1) ShadowTemp* mkShadowTempOneDouble(double value);
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoDoubles(double* values);
VG_REGPARM(1) ShadowTemp* mkShadowTempOneSingle(double value);
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoSingles(UWord values);
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSingles(float* values);
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSinglesG(UWord guard, float* values);
ShadowValue* getTS(Int idx);
VG_REGPARM(2) void printStoreValue(const char* dest_label, ShadowValue* val);
void printStoreValueF(ShadowValue* val, const char* format, ...);

#endif
