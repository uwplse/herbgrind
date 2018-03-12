/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie    value-shadowstate.c ---*/
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

#include "value-shadowstate.h"

#include "pub_tool_hashtable.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_mallocfree.h"

#include "../shadowop/influence-op.h"

#include "../../options.h"
#include "../../helper/debug.h"
#include "../../helper/runtime-util.h"

#include <math.h>

ArgUnion computedArgs;

ResultUnion computedResult;

ShadowTemp* shadowTemps[MAX_TEMPS];
ShadowValue* shadowThreadState[MAX_THREADS][MAX_REGISTERS];
TableValueEntry* shadowMemTable[LARGE_PRIME];

Stack* freedTemps[MAX_TEMP_SHADOWS];
Stack* freedVals;
Stack* tableEntries;

Word256 getBytes;
inline TableValueEntry* mkTableEntry(void);

void initValueShadowState(void){
  for(int i = 0; i < MAX_TEMP_SHADOWS; ++i){
    freedTemps[i] = mkStack();
  }
  freedVals = mkStack();
  tableEntries = mkStack();
  valueCacheSingle = VG_(HT_construct)("value cache single-precision");
  valueCacheDouble = VG_(HT_construct)("value cache double precision");
  initExprAllocator();
}

VG_REGPARM(2) void dynamicCleanup(int nentries, IRTemp* entries){
  Bool hasEntriesToCleanup = False;
  if (print_temp_moves){
    for(int i = 0; i < nentries; ++i){
      ShadowTemp* temp = shadowTemps[entries[i]];
      if (temp == NULL) continue;
      if (!hasEntriesToCleanup){
        VG_(printf)("Freeing temp(s) %p", temp);
        hasEntriesToCleanup = True;
      } else {
        VG_(printf)(", %p", temp);
      }
    }
    if (hasEntriesToCleanup){
      VG_(printf)("\n");
    }
  }
  for(int i = 0; i < nentries; ++i){
    ShadowTemp* temp = shadowTemps[entries[i]];
    if (temp == NULL) continue;
    for(int j = 0; j < INT(temp->num_blocks);
        j += temp->values[j]->type == Vt_Double ? 2 : 1){
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Cleaning up value %p (old rc %lu) "
                    "from temp %p in %d at end of block.\n",
                    temp->values[j], temp->values[j]->ref_count,
                    temp, entries[i]);
      }
      disownShadowValue(temp->values[j]);
    }
    freeShadowTemp(temp);
    shadowTemps[entries[i]] = NULL;
  }
}
inline
ShadowValue* getTS(Int idx){
  ShadowValue* result = shadowThreadState[VG_(get_running_tid)()][idx];
  tl_assert2(result == NULL || result->ref_count > 0,
             "Freed value %p left over at TS(%d)",
             result, idx);
  return result;
}
VG_REGPARM(2) ShadowTemp* dynamicLoad(Addr memSrc, FloatBlocks numBlocks){
  ShadowTemp* newTemp = mkShadowTemp(numBlocks);
  for(int i = 0; i < INT(numBlocks); ++i){
    newTemp->values[i] = getMemShadow(memSrc + i * sizeof(float));
    ownShadowValue(newTemp->values[i]);
  }
  return newTemp;
}
VG_REGPARM(1) ShadowValue* getMemShadow(Addr64 addr){
  int key = addr % LARGE_PRIME;
  for(TableValueEntry* node = shadowMemTable[key];
      node != NULL; node = node->next){
    if (node->addr == addr){
      return node->val;
    }
  }
  return NULL;
}
VG_REGPARM(3) void setMemShadowTemp(Addr64 memDest,
                                    UWord size,
                                    ShadowTemp* st){
  for(int i = 0; i < size; ++i){
    UWord addr = memDest + i * sizeof(float);
    removeMemShadow(addr);
    if (st != NULL){
      if (i % 2 == 1 && st->values[i - 1]->type == Vt_Double){
        continue;
      }
      addMemShadow(addr, st->values[i]);
    }
  }
}
void removeMemShadow(Addr64 addr){
  int key = addr % LARGE_PRIME;
  TableValueEntry* prevEntry = NULL;
  for(TableValueEntry* node = shadowMemTable[key];
      node != NULL; node = node->next){
    if (node->addr == addr){
      if (prevEntry == NULL){
        shadowMemTable[key] = node->next;
      } else {
        prevEntry->next = node->next;
      }

      if (PRINT_VALUE_MOVES){
        VG_(printf)("Clearing %llX", addr);
        if (node->val != NULL){
          VG_(printf)(", which disowns %p (old rc %lu)", node->val, node->val->ref_count);
        }
        VG_(printf)("\n");
      }
      disownShadowValue(node->val);
      stack_push(tableEntries, (void*)node);
      break;
    }
    prevEntry = node;
  }
}
VG_REGPARM(0) TableValueEntry* newTableValueEntry(void){
  return VG_(malloc)("tableEntry", sizeof(TableValueEntry));
}
inline TableValueEntry* mkTableEntry(void){
  TableValueEntry* newEntry;
  if (stack_empty(tableEntries)){
    newEntry = VG_(malloc)("tableEntry", sizeof(TableValueEntry));
  } else {
    newEntry = (void*)stack_pop(tableEntries);
  }
  return newEntry;
}
void addMemShadow(Addr64 addr, ShadowValue* val){
  TableValueEntry* newEntry = mkTableEntry();
  newEntry->addr = addr;
  newEntry->val = val;
  ownShadowValue(val);

  int key = addr % LARGE_PRIME;
  newEntry->next = shadowMemTable[key];
  shadowMemTable[key] = newEntry;
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Setting %llX to %p", addr, val);
    if (val != NULL){
      VG_(printf)(" (new rc %lu)", val->ref_count);
    }
    VG_(printf)("\n");
  }
}
void freeShadowTemp(ShadowTemp* temp){
  stack_push(freedTemps[INT(temp->num_blocks) - 1], (void*)temp);
}

inline
ShadowTemp* mkShadowTemp(FloatBlocks num_blocks){
  ShadowTemp* result;
  if (stack_empty(freedTemps[INT(num_blocks) - 1])){
    result = newShadowTemp(num_blocks);
    if (print_temp_moves || print_allocs){
      VG_(printf)("Making fresh shadow temp %p with values %p\n",
                  result, result->values);
    }
  } else {
    result = (void*)stack_pop(freedTemps[INT(num_blocks) - 1]);
  }
  return result;
}
void freeShadowValue(ShadowValue* val){
  while(val->influences != NULL){
    (void)lpop(InfluenceList)(&(val->influences));
  }
  if (!no_exprs){
    if (print_expr_refs){
      VG_(printf)("Disowning expression %p as part of freeing val %p\n",
                  val->expr, val);
    }
    disownConcExpr(val->expr);
  }
  double value = getDouble(val->real);
  if (value == 0.0) value = 0.0;
  if (isNaN(val->real)) value = NAN;
  TableValueEntry* entry =
    VG_(HT_remove)(val->type == Vt_Single ? valueCacheSingle : valueCacheDouble,
                   *(UWord*)&value);
  if (entry != NULL){
    stack_push(tableEntries, (void*)entry);
  }
  stack_push_fast(freedVals, (void*)val);
}

ShadowValue* copyShadowValue(ShadowValue* val){
  ShadowValue* copy = mkShadowValueBare(val->type);
  if (!no_reals){
    copyReal(val->real, copy->real);
  }
  copy->expr = val->expr;
  if (!no_exprs){
    recursivelyOwnConcExpr(copy->expr,
                           max_expr_block_depth * 2);
  }
  if (!no_influences){
    copy->influences = cloneInfluences(val->influences);
  }
  return copy;
}
inline
ShadowValue* mkShadowValueBare(ValueType type){
  ShadowValue* result;
  if (stack_empty_fast(freedVals)){
    result = newShadowValue(type);
    if (PRINT_VALUE_MOVES || print_allocs){
      VG_(printf)("Alloced new shadow value %p\n", result);
    }
  } else {
    result = (void*)stack_pop_fast(freedVals);
    result->type = type;
  }
  result->ref_count = 1;
  return result;
}

VG_REGPARM(2) ShadowValue* mkShadowValue_wrapper(ValueType type, UWord value){
  return mkShadowValue(type, *(double*)(void*)&value);
}
VgHashTable* valueCacheSingle;
VgHashTable* valueCacheDouble;
inline
ShadowValue* mkShadowValue(ValueType type, double value){
  if (value == 0.0) value = 0.0;
  if (value != value) value = NAN;
  UWord key = *(UWord*)&value;
  TableValueEntry* existingEntry =
    VG_(HT_lookup)(type == Vt_Single ? valueCacheSingle : valueCacheDouble, key);
  ShadowValue* result = NULL;
  if (existingEntry == NULL || no_reals){
    result = mkShadowValueBare(type);
    if (!no_reals){
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Setting shadow value %p to initial value of ", result);
        ppFloat(value);
        VG_(printf)("\n");
      }
      setReal(result->real, value);
    }
    if (!no_exprs){
      result->expr = mkLeafConcExpr(value);
    }
    ValueCacheEntry* newEntry = (void*)mkTableEntry();
    newEntry->val = result;
    newEntry->key = key;
    VG_(HT_add_node)(type == Vt_Single ? valueCacheSingle : valueCacheDouble,
                     newEntry);
  } else {
    result = existingEntry->val;
    ownShadowValue(result);
  }
  return result;
}

VG_REGPARM(1) ShadowTemp* copyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = mkShadowTemp(temp->num_blocks);
  for(int i = 0; i < INT(temp->num_blocks); ++i){
    if (i % 2 == 1 && temp->values[i-1]->type == Vt_Double){
      result->values[i] = NULL;
    } else {
      ownShadowValue(temp->values[i]);
      result->values[i] = temp->values[i];
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Copying %p (new rc %lu) from %p to new temp %p\n",
                    temp->values[i], temp->values[i]->ref_count, temp,
                    result);
      }
    }
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* deepCopyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = mkShadowTemp(temp->num_blocks);
  for(int i = 0; i < INT(temp->num_blocks); ++i){
    if (i % 2 == 1 && temp->values[i-1]->type == Vt_Double){
      result->values[i] = NULL;
    } else {
      result->values[i] = copyShadowValue(temp->values[i]);
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Copying shadow value %p from temp %p "
                    "to shadow value %p at temp %p\n",
                    temp->values[i], temp, result->values[i], result);
      }
    }
  }
  return result;
}
inline
void disownShadowTemp(ShadowTemp* temp){
  for(int i = 0; i < INT(temp->num_blocks); ++i){
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Disowning %p (rc %lu) as part of disowning temp %p\n",
                  temp->values[i], temp->values[i]->ref_count, temp);
    }
    if (i % 2 == 0 || temp->values[i-1]->type != Vt_Double){
      disownShadowValue(temp->values[i]);
      temp->values[i] = NULL;
    }
  }
  freeShadowTemp(temp);
}
VG_REGPARM(1) void disownShadowTempNonNullDynamic(IRTemp idx){
  disownShadowTemp(shadowTemps[idx]);
  shadowTemps[idx] = NULL;
}
VG_REGPARM(1) void disownShadowTempDynamic(IRTemp idx){
  if (shadowTemps[idx] != NULL){
    disownShadowTemp(shadowTemps[idx]);
    shadowTemps[idx] = NULL;
  }
}
void disownShadowValue(ShadowValue* val){
  if (val == NULL) return;
  tl_assert2(val->ref_count > 0,
             "Trying to disown %p, but it's ref count is already %lu!\n",
             val, val->ref_count);
  val->ref_count--;
  if (val->ref_count == 0){
    freeShadowValue(val);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Disowned last reference to %p! Freeing...\n", val);
    }
  }
}
void ownShadowValue(ShadowValue* val){
  if (val == NULL) return;
  (val->ref_count)++;
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning shadow value %p (new ref count %lu)\n", val, val->ref_count);
  }
}

VG_REGPARM(1) ShadowTemp* mkShadowTempOneDouble(double value){
  ShadowTemp* result = mkShadowTemp(FB(2));
  result->values[0] = mkShadowValue(Vt_Double, value);
  result->values[1] = NULL;
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making value %p as one double temp %p\n",
                result->values[0], result);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoDoubles(double* values){
  ShadowTemp* result = mkShadowTemp(FB(4));
  result->values[0] =
    mkShadowValue(Vt_Double, values[0]);
  result->values[1] = NULL;
  result->values[2] =
    mkShadowValue(Vt_Double, values[1]);
  result->values[3] = NULL;
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making values %p and %p as part of two double temp %p\n",
                result->values[0], result->values[1], result);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempOneSingle(double value){
  ShadowTemp* result = mkShadowTemp(FB(1));
  result->values[0] = mkShadowValue(Vt_Single, value);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making value %p as part of one single temp %p\n",
                result->values[0], result);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoSingles(UWord values){
  ShadowTemp* result = mkShadowTemp(FB(2));
  float floatValues[2];
  VG_(memcpy)(floatValues, &values, sizeof(floatValues));
  result->values[0] = mkShadowValue(Vt_Single, floatValues[0]);
  result->values[1] = mkShadowValue(Vt_Single, floatValues[1]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making values %p and %p "
                "as part of two singles temp %p\n",
                result->values[0], result->values[1], result);
  }
  return result;
}
inline
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSingles(float* values){
  ShadowTemp* result = mkShadowTemp(FB(4));
  result->values[0] =
    mkShadowValue(Vt_Single, values[0]);
  result->values[1] =
    mkShadowValue(Vt_Single, values[1]);
  result->values[2] =
    mkShadowValue(Vt_Single, values[2]);
  result->values[3] =
    mkShadowValue(Vt_Single, values[3]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making values %p, %p, %p, and %p "
                "as part of four single temp %p\n",
                result->values[0], result->values[1],
                result->values[2], result->values[3],
                result);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSinglesG(UWord guard, float* values){
  if (!guard) return NULL;
  return mkShadowTempFourSingles(values);
}

VG_REGPARM(2) void printStoreValue(const char* dest_label, ShadowValue* val){
  VG_(printf)("%s = %p(%lu)\n", dest_label, val, val->ref_count);
}

#define FSTRINGSIZE 1024
char buffer[FSTRINGSIZE];
void printStoreValueF(ShadowValue* val, const char* format, ...){
  if (val == NULL) return;
  va_list vl;
  va_start(vl, format);
  VG_(vsnprintf)(buffer, FSTRINGSIZE, format, vl);
  va_end(vl);
  printStoreValue(buffer, val);
}
