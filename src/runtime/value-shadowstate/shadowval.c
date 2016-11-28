/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            shadowval.c ---*/
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

#include "shadowval.h"
#include "exprs.h"
#include "real.h"
#include "value-shadowstate.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_hashtable.h"
#include "pub_tool_libcbase.h"

VgHashTable* doubleShadowValueCache;
VgHashTable* singleShadowValueCache;

typedef struct _cacheEntry {
  struct _cacheEntry* next;
  UWord key;
  ShadowValue* shadowValue;
} CacheEntry;

void initShadowValueSystem(void){
  doubleShadowValueCache = VG_(HT_construct)("double shadow value cache");
  singleShadowValueCache = VG_(HT_construct)("single shadow value cache");
}

VG_REGPARM(1) ShadowTemp* newShadowTemp(UWord num_vals){
  ShadowTemp* newShadowTemp =
    VG_(calloc)("shadow temp", 1, sizeof(ShadowTemp));
  newShadowTemp->num_vals = num_vals;
  newShadowTemp->values =
    VG_(calloc)("shadow temp", num_vals,
                sizeof(ShadowValue*));
  return newShadowTemp;
}
void freeShadowTemp(ShadowTemp* temp){
  VG_(free)(temp->values);
  VG_(free)(temp);
}
void changeSingleValueType(ShadowTemp* temp, FloatType type){
  temp->values[0]->type = type;
}
UWord hashDouble(double val){
  UWord result;
  VG_(memcpy)(&result, &val, sizeof(UWord));
  return result;
}
ShadowValue* newShadowValue(FloatType type, double value){
  UWord key = hashDouble(value);
  CacheEntry* entry = VG_(HT_lookup)(type == Ft_Single ?
                                     singleShadowValueCache :
                                     doubleShadowValueCache,
                                     key);
  if (entry != NULL)
    return entry->shadowValue;
  entry = VG_(malloc)("shadow value cache entry", sizeof(CacheEntry));
  entry->key = key;

  ShadowValue* result = VG_(malloc)("shadow value", sizeof(ShadowValue));
  result->ref_count = 1;
  result->type = type;
  result->real = mkReal(value);
  result->expr = mkLeafExpr(value);
  result->influences = NULL;
  entry->shadowValue = result;
  return result;
}
void freeShadowValue(ShadowValue* val){
  freeReal(val->real);
  VG_(free)(val);
}

ShadowValue* copyShadowValue(ShadowValue* val){
  ShadowValue* result = VG_(malloc)("shadow value", sizeof(ShadowValue));
  result->type = val->type;
  result->ref_count = 1;
  result->real = copyReal(val->real);
  result->expr = val->expr;
  ownExpr(result->expr);
  result->influences = val->influences;
  return result;
}
