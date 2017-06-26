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

#include "../../options.h"
#include "../../helper/debug.h"

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
    for(int j = 0; j < temp->num_vals; ++j){
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
  return shadowThreadState[VG_(get_running_tid)()][idx];
}
VG_REGPARM(2) void dynamicPut64(Int tsDest, ShadowTemp* st){
  tl_assert(st->num_vals == 1 ||
            st->num_vals == 2);
  if (st->num_vals == 1){
    tl_assert(st->values[0]->type == Ft_Double);
    ShadowValue* val = st->values[0];
    if (PRINT_VALUE_MOVES){
      if (getTS(tsDest) != NULL || val != NULL){
        VG_(printf)("dynamicPut64: Setting thread state %d to %p (old ref count %lu",
                    tsDest, val, val->ref_count);
      }
      if (val != NULL){
        VG_(printf)(") (type ");
        ppFloatType(val->type);
      }
      VG_(printf)(")\n");
      if (getTS(tsDest + sizeof(float)) != NULL){
        VG_(printf)("dynamicPut64: Overwriting TS(%lu) with NULL, "
                    "due to double write at TS(%d)\n",
                    tsDest + sizeof(float),
                    tsDest);
      }
    }
    shadowThreadState[VG_(get_running_tid)()][tsDest] = val;
    shadowThreadState[VG_(get_running_tid)()][tsDest + sizeof(float)] =
      NULL;
    ownShadowValue(val);
  } else {
    for(int i = 0; i < 2; ++i){
      int dest_addr = tsDest + i * sizeof(float);
      tl_assert(st->values[i]->type == Ft_Single);
      ShadowValue* val = st->values[i];
      shadowThreadState[VG_(get_running_tid)()][dest_addr] = val;
      ownShadowValue(val);
      if (PRINT_VALUE_MOVES){
        if (getTS(tsDest) != NULL || val != NULL){
          VG_(printf)("dynamicPut64: Setting thread state %d to %p (new ref count %lu)\n",
                      tsDest, val, val->ref_count);
        }
        if (val != NULL){
          VG_(printf)(" (type ");
          ppFloatType(val->type);
          VG_(printf)(")\n");
        }
      }
    }
  }
}
VG_REGPARM(2) void dynamicPut128(Int tsDest, ShadowTemp* st){
  for(int i = 0; i < st->num_vals; ++i){
    ShadowValue* val = st->values[i];
    int size = val->type == Ft_Single ? sizeof(float) : sizeof(double);
    int dest_byte = tsDest + (i * size);
    shadowThreadState[VG_(get_running_tid)()][dest_byte] =
      val;
    if (PRINT_VALUE_MOVES){
      if (getTS(dest_byte) != NULL || val != NULL){
        VG_(printf)("dynamicPut: Setting thread state %d to %p",
                    dest_byte, val);
        if (val != NULL){
          VG_(printf)(" (type %d) (new ref count %lu)\n", val->type, val->ref_count);
        } else {
          VG_(printf)("\n");
        }
      }
    }
    if (val->type == Ft_Double){
      int second_byte = dest_byte + sizeof(float);
      shadowThreadState[VG_(get_running_tid)()]
        [second_byte] =
        NULL;
      if (PRINT_VALUE_MOVES && getTS(second_byte)){
        VG_(printf)("Setting thread state %d to 0x0",
                    second_byte);
      }
    }
    ownShadowValue(val);
  }
}
VG_REGPARM(2) ShadowTemp* dynamicGet64(Int tsSrc, UWord tsBytes){
  return dynamicGet(tsSrc, &tsBytes, 8);
}

VG_REGPARM(3) ShadowTemp* dynamicGet128(Int tsSrc,
                                        UWord bytes1,
                                        UWord bytes2){
  UWord bytes[] = {bytes1, bytes2};
  return dynamicGet(tsSrc, bytes, 16);
}

VG_REGPARM(2) ShadowTemp* dynamicGet256(Int tsSrc,
                                        Word256* bytes){
  return dynamicGet(tsSrc, bytes, 32);
}
ShadowTemp* dynamicGet(Int tsSrc, void* bytes, int size){
  if (size == 4){
    ShadowTemp* result = mkShadowTemp(1);
    ShadowValue* val = getTS(tsSrc);
    if (val == NULL) return NULL;
    tl_assert(val->type == Ft_Single);
    result->values[0] = val;
    ownShadowValue(result->values[0]);
    return result;
  } else if (size == 8 && getTS(tsSrc) != NULL && getTS(tsSrc)->type == Ft_Double){
    ShadowTemp* result = mkShadowTemp(1);
    result->values[0] = getTS(tsSrc);
    ownShadowValue(result->values[0]);
    return result;
  }
  ShadowTemp* halves[2];
  for(int i = 0; i < 2; ++i){
    halves[i] = dynamicGet(tsSrc + (size / 2) * i, ((float*)bytes) + (i * size / 8), size / 2);
  }
  if (halves[0] == NULL &&
      halves[1] == NULL){
    return NULL;
  } else {
    if (halves[0] == NULL){
      halves[0] = mkShadowTempValues(bytes, halves[1]->num_vals,
                                     halves[1]->values[0]->type);
    } else if (halves[1] == NULL){
      halves[1] = mkShadowTempValues((void*)(((float*)bytes) + size / 8),
                                     halves[0]->num_vals,
                                     halves[0]->values[0]->type);
    }
    ShadowTemp* result = mkShadowTemp(halves[1]->num_vals * 2);
    for(int i = 0 ; i < halves[1]->num_vals; ++i){
      result->values[i] = halves[0]->values[i];
      result->values[i + halves[1]->num_vals] =
        halves[1]->values[i];
    }
    for(int i = 0; i < 2; ++i){
      freeShadowTemp(halves[i]);
    }
    return result;
  }
}
VG_REGPARM(2) ShadowTemp* dynamicLoad(Addr memSrc, int size){
  switch(size){
  case 1:
    return dynamicLoad32(memSrc);
  case 2:
    return dynamicLoad64(memSrc);
  case 4:
    return dynamicLoad128(memSrc);
  case 8:
    return dynamicLoad256(memSrc);
  default:
    tl_assert(0);
    return NULL;
  }
}
ShadowTemp* dynamicLoad32(UWord memSrc){
  ShadowValue* val = getMemShadow(memSrc);
  if (val != NULL){
    ShadowTemp* newTemp = mkShadowTemp(1);
    newTemp->values[0] = val;
    ownShadowValue(val);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Owning shadow value %p (new rc %lu) as part of dynamic load 32.\n",
                  val, val->ref_count);
    }
    return newTemp;
  } else {
    return NULL;
  }
}
ShadowTemp* dynamicLoad64(Addr memSrc){
  ShadowValue* firstValue = getMemShadow(memSrc);
  if (firstValue == NULL){
    ShadowValue* secondValue = getMemShadow(memSrc + sizeof(float));
    if (secondValue == NULL || secondValue->type != Ft_Single){
      return NULL;
    }
    ShadowTemp* temp = mkShadowTemp(2);
    firstValue = mkShadowValue(Ft_Single, *(float*)(void*)memSrc);
    temp->values[0] = firstValue;
    temp->values[1] = secondValue;
    ownShadowValue(secondValue);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Owning shadow value %p (new rc %lu) as part of dynamic load 64.\n",
                  secondValue, secondValue->ref_count);
    }
    return temp;
  } else if (firstValue->type == Ft_Double){
    ShadowTemp* temp = mkShadowTemp(1);
    temp->values[0] = firstValue;
    ownShadowValue(firstValue);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Owning shadow value %p (new rc %lu) as part of dynamic load 64.\n",
                  firstValue, firstValue->ref_count);
    }
    return temp;
  } else {
    ShadowValue* secondValue = getMemShadow(memSrc + sizeof(float));
    if (secondValue == NULL || secondValue->type != Ft_Single){
      secondValue = mkShadowValue(Ft_Single, *(float*)(void*)(memSrc + sizeof(float)));
    } else {
      ownShadowValue(secondValue);
    }
    tl_assert(secondValue->type == Ft_Single);
    ShadowTemp* temp = mkShadowTemp(2);
    ownShadowValue(firstValue);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Owning shadow value(s) %p (new rc %lu)",
                  firstValue, firstValue->ref_count);
      if (secondValue != NULL){
        VG_(printf)(" and %p (new rc %lu)",
                    secondValue, secondValue->ref_count);
      }
      VG_(printf)(" as part of dynamic load 64.\n");
    }
    temp->values[0] = firstValue;
    temp->values[1] = secondValue;
    return temp;
  }
}
ShadowTemp* dynamicLoad128(UWord memSrc){
  ShadowTemp* firstHalf = dynamicLoad64(memSrc);
  ShadowTemp* secondHalf = dynamicLoad64(memSrc + sizeof(double));
  if (firstHalf == NULL && secondHalf == NULL){
    return NULL;
  } else if (firstHalf == NULL){
    switch(secondHalf->num_vals){
    case 1:
      firstHalf = mkShadowTempOneDouble(*(double*)(void*)memSrc);
      break;
    case 2:
      firstHalf = mkShadowTempTwoSingles(*(UWord*)(void*)memSrc);
      break;
    default:
      tl_assert(0);
      return NULL;
    }
  } else if (secondHalf == NULL ||
             secondHalf->num_vals != firstHalf->num_vals){
    if (secondHalf != NULL){
      disownShadowTemp(secondHalf);
    }
    switch(firstHalf->num_vals){
    case 1:
      secondHalf = mkShadowTempOneDouble(*(double*)(void*)memSrc);
      break;
    case 2: {
      secondHalf = mkShadowTempTwoSingles(*(UWord*)(void*)memSrc);
    }
    default:
      tl_assert(0);
      return NULL;
    }
  }
  ShadowTemp* result;
  if (firstHalf->num_vals == 1 && secondHalf->num_vals == 1){
    result = mkShadowTemp(2);
    result->values[0] = firstHalf->values[0];
    result->values[1] = secondHalf->values[0];
  } else {
    tl_assert(firstHalf->num_vals == 2 &&
              secondHalf->num_vals == 2);
    result = mkShadowTemp(4);
    result->values[0] = firstHalf->values[0];
    result->values[1] = firstHalf->values[1];
    result->values[2] = secondHalf->values[0];
    result->values[3] = secondHalf->values[1];
  }
  freeShadowTemp(firstHalf);
  freeShadowTemp(secondHalf);
  return result;
}
ShadowTemp* dynamicLoad256(UWord memSrc){
  ShadowTemp* firstHalf = dynamicLoad128(memSrc);
  ShadowTemp* secondHalf = dynamicLoad128(memSrc + sizeof(double) * 2);
  if (firstHalf == NULL && secondHalf == NULL){
    return NULL;
  } else if (firstHalf == NULL){
    switch(secondHalf->num_vals){
    case 2:
      firstHalf = mkShadowTempOneDouble(*(double*)(void*)memSrc);
      break;
    case 4:
      firstHalf = mkShadowTempTwoSingles(*(UWord*)(void*)memSrc);
      break;
    default:
      tl_assert(0);
      return NULL;
    }
  } else if (secondHalf == NULL ||
             secondHalf->num_vals != firstHalf->num_vals){
    if (secondHalf != NULL){
      disownShadowTemp(secondHalf);
    }
    switch(firstHalf->num_vals){
    case 2:
      secondHalf = mkShadowTempOneDouble(*(double*)(void*)memSrc);
      break;
    case 4:
      secondHalf = mkShadowTempTwoSingles(*(UWord*)(void*)memSrc);
    default:
      tl_assert(0);
      return NULL;
    }
  }
  ShadowTemp* result;
  if (firstHalf->num_vals == 2 && secondHalf->num_vals == 2){
    result = mkShadowTemp(4);
    result->values[0] = firstHalf->values[0];
    result->values[1] = firstHalf->values[1];
    result->values[2] = secondHalf->values[0];
    result->values[3] = secondHalf->values[1];
  } else {
    tl_assert(firstHalf->num_vals == 4 &&
              secondHalf->num_vals == 4);
    result = mkShadowTemp(8);
    result->values[0] = firstHalf->values[0];
    result->values[1] = firstHalf->values[1];
    result->values[2] = firstHalf->values[2];
    result->values[3] = firstHalf->values[3];
    result->values[4] = secondHalf->values[0];
    result->values[5] = secondHalf->values[1];
    result->values[6] = secondHalf->values[2];
    result->values[7] = secondHalf->values[3];
  }
  freeShadowTemp(firstHalf);
  freeShadowTemp(secondHalf);
  return result;
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
      if (st->values[i/2]->type == Ft_Double){
        if (i % 2 == 0){
          addMemShadow(addr, st->values[i/2]);
        }
      } else {
        addMemShadow(addr, st->values[i]);
      }
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
  stack_push(freedTemps[temp->num_vals - 1], (void*)temp);
}

inline
ShadowTemp* mkShadowTemp(UWord num_vals){
  ShadowTemp* result;
  if (stack_empty(freedTemps[num_vals - 1])){
    result = newShadowTemp(num_vals);
    if (print_temp_moves || print_allocs){
      VG_(printf)("Making fresh shadow temp %p with values %p\n",
                  result, result->values);
    }
  } else {
    result = (void*)stack_pop(freedTemps[num_vals - 1]);
  }
  return result;
}
void freeShadowValue(ShadowValue* val){
  while(val->influences != NULL){
    (void)lpop(InfluenceList)(&(val->influences));
  }
  if (!no_exprs){
    disownConcExpr(val->expr);
  }
  double value = getDouble(val->real);
  TableValueEntry* entry =
    VG_(HT_remove)(val->type == Ft_Single ? valueCacheSingle : valueCacheDouble,
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
  return copy;
}
inline
ShadowValue* mkShadowValueBare(FloatType type){
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
VgHashTable* valueCacheSingle;
VgHashTable* valueCacheDouble;
inline
ShadowValue* mkShadowValue(FloatType type, double value){
  TableValueEntry* existingEntry =
    VG_(HT_lookup)(type == Ft_Single ? valueCacheSingle : valueCacheDouble,
                   *(UWord*)&value);
  ShadowValue* result = NULL;
  if (existingEntry == NULL){
    result = mkShadowValueBare(type);
    if (!no_reals){
      setReal(result->real, value);
    }
    if (!no_exprs){
      result->expr = mkLeafConcExpr(value);
    }
    TableValueEntry* newEntry = mkTableEntry();
    newEntry->val = result;
    newEntry->addr = *(UWord*)&value;
    VG_(HT_add_node)(type == Ft_Single ? valueCacheSingle : valueCacheDouble,
                     newEntry);
  } else {
    result = existingEntry->val;
    ownShadowValue(result);
  }
  return result;
}

VG_REGPARM(1) ShadowTemp* copyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = mkShadowTemp(temp->num_vals);
  for(int i = 0; i < temp->num_vals; ++i){
    ownShadowValue(temp->values[i]);
    result->values[i] = temp->values[i];
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Copying %p (new rc %lu) to new temp %p\n", temp->values[i], temp->values[i]->ref_count,
                  result);
    }
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* deepCopyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = mkShadowTemp(temp->num_vals);
  for(int i = 0; i < temp->num_vals; ++i){
    result->values[i] = copyShadowValue(temp->values[i]);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Copying shadow value %p from temp %p to shadow value %p at temp %p\n",
                  temp->values[i], temp, result->values[i], result);
    }
  }
  return result;
}
inline
void disownShadowTemp(ShadowTemp* temp){
  for(int i = 0; i < temp->num_vals; ++i){
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Disowning %p (rc %lu) as part of disowning temp %p\n",
                  temp->values[i], temp->values[i]->ref_count, temp);
    }
    disownShadowValue(temp->values[i]);
    temp->values[i] = NULL;
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
}

VG_REGPARM(1) ShadowTemp* mkShadowTempOneDouble(double value){
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = mkShadowValue(Ft_Double, value);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making value %p as one double temp %p\n",
                result->values[0], result);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoDoubles(double* values){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] =
    mkShadowValue(Ft_Double, values[0]);
  result->values[1] =
    mkShadowValue(Ft_Double, values[1]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making values %p and %p as part of two double temp %p\n",
                result->values[0], result->values[1], result);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempOneSingle(double value){
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = mkShadowValue(Ft_Single, value);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making value %p as part of one single temp %p\n",
                result->values[0], result);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoSingles(UWord values){
  ShadowTemp* result = mkShadowTemp(2);
  float floatValues[2];
  VG_(memcpy)(floatValues, &values, sizeof(floatValues));
  result->values[0] = mkShadowValue(Ft_Single, floatValues[0]);
  result->values[1] = mkShadowValue(Ft_Single, floatValues[1]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making values %p and %p "
                "as part of two singles temp %p\n",
                result->values[0], result->values[1], result);
  }
  return result;
}
inline
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSingles(float* values){
  ShadowTemp* result = mkShadowTemp(4);
  result->values[0] =
    mkShadowValue(Ft_Single, values[0]);
  result->values[1] =
    mkShadowValue(Ft_Single, values[1]);
  result->values[2] =
    mkShadowValue(Ft_Single, values[2]);
  result->values[3] =
    mkShadowValue(Ft_Single, values[3]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making values %p, %p, %p, and %p "
                "as part of four single temp %p\n",
                result->values[0], result->values[1],
                result->values[2], result->values[3],
                result);
  }
  return result;
}
inline ShadowTemp* mkShadowTempValues(void* bytes, int num_values, FloatType type){
  ShadowTemp* result = mkShadowTemp(num_values);
  for(int i = 0; i < num_values; ++i){
    result->values[i] =
      mkShadowValue(type,
                    type == Ft_Single ? ((float*)bytes)[i] : ((double*)bytes)[i]);
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
