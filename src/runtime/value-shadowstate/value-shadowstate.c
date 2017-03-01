/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie    value-shadowstate.c ---*/
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

#include "value-shadowstate.h"

#include "pub_tool_hashtable.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_mallocfree.h"
#include "../shadowop/mathreplace.h"

#include "../../options.h"
#include "../../helper/debug.h"

ShadowTemp* shadowTemps[MAX_TEMPS];
ShadowValue* shadowThreadState[MAX_THREADS][MAX_REGISTERS];
VgHashTable* shadowMemory = NULL;

Stack* freedTemps[MAX_TEMP_SHADOWS];
Stack* freedVals;
Stack* memEntries;

void initValueShadowState(void){
  for(int i = 0; i < MAX_TEMP_SHADOWS; ++i){
    freedTemps[i] = mkStack();
  }
  freedVals = mkStack();
  memEntries = mkStack();
  shadowMemory = VG_(HT_construct)("shadow memory");
  callToOpInfoMap = VG_(HT_construct)("call map");
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
      if (print_value_moves){
        VG_(printf)("Cleaning up value %p (old rc %lu) "
                    "from temp %p at end of block.\n",
                    temp->values[j], temp->values[j]->ref_count,
                    temp);
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
    shadowThreadState[VG_(get_running_tid)()][tsDest] = val;
    shadowThreadState[VG_(get_running_tid)()][tsDest + sizeof(float)] =
      NULL;
    ownShadowValue(val);
    if (print_value_moves){
      if (getTS(tsDest) != NULL || val != NULL){
        VG_(printf)("dynamicPut64: Setting thread state %d to %p\n",
                    tsDest, val);
      }
      if (val != NULL){
        VG_(printf)(" (type ");
        ppFloatType(val->type);
        VG_(printf)(")\n");
      }
      if (getTS(tsDest + sizeof(float)) != NULL){
        VG_(printf)("dynamicPut64: Overwriting TS(%lu) with NULL, "
                    "due to double write at TS(%d)\n",
                    tsDest + sizeof(float),
                    tsDest);
      }
    }
  } else {
    for(int i = 0; i < 2; ++i){
      int dest_addr = tsDest + i * sizeof(float);
      tl_assert(st->values[i]->type == Ft_Single);
      ShadowValue* val = st->values[i];
      shadowThreadState[VG_(get_running_tid)()][dest_addr] = val;
      ownShadowValue(val);
      if (print_value_moves){
        if (getTS(tsDest) != NULL || val != NULL){
          VG_(printf)("dynamicPut64: Setting thread state %d to %p\n",
                      tsDest, val);
        }
      }
      if (val != NULL){
        VG_(printf)(" (type ");
        ppFloatType(val->type);
        VG_(printf)(")\n");
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
    if (print_value_moves){
      if (getTS(dest_byte) != NULL || val != NULL){
        VG_(printf)("dynamicPut: Setting thread state %d to %p",
                    dest_byte, val);
        if (val != NULL){
          VG_(printf)(" (type %d)\n", val->type);
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
      if (print_value_moves && getTS(second_byte)){
        VG_(printf)("Setting thread state %d to 0x0",
                    second_byte);
      }
    }
    ownShadowValue(val);
  }
}
VG_REGPARM(1) ShadowTemp* dynamicGet64(Int tsSrc, UWord tsBytes){
  ShadowValue* firstValue = getTS(tsSrc);
  if (firstValue == NULL){
    ShadowValue* secondValue = getTS(tsSrc + sizeof(float));
    if (secondValue == NULL || secondValue->type != Ft_Single){
      return NULL;
    }
    ShadowTemp* temp = mkShadowTemp(2);
    float firstValueBytes;
    VG_(memcpy)(&firstValueBytes, &tsBytes, sizeof(float));
    firstValue = mkShadowValue(Ft_Single, firstValueBytes);
    if (print_value_moves){
      VG_(printf)("Making value %p as part of dynamicGet64\n",
                  firstValue);
    }
    temp->values[0] = firstValue;
    temp->values[1] = secondValue;
    ownShadowValue(secondValue);
    if (print_value_moves){
      VG_(printf)("Owning %p (new rc %lu) as part of dynamic get.\n",
                  secondValue, secondValue->ref_count);
    }
    return temp;
  } if (firstValue->type == Ft_Double){
    ShadowTemp* temp = mkShadowTemp(1);
    temp->values[0] = firstValue;
    ownShadowValue(firstValue);
    if (print_value_moves){
      VG_(printf)("Owning %p (new rc %lu) as part of dynamic get.\n",
                  firstValue, firstValue->ref_count);
    }
    return temp;
  } else {
    ShadowValue* secondValue = getTS(tsSrc + sizeof(float));
    if (secondValue == NULL){
      float secondValueBytes;
      VG_(memcpy)(&secondValueBytes, (&tsBytes) + sizeof(float),
                  sizeof(float));
      secondValue = mkShadowValue(Ft_Single, secondValueBytes);
      if (print_value_moves){
        VG_(printf)("Making value %p as part of dynamicGet64\n",
                    secondValue);
      }
    } else {
      ownShadowValue(secondValue);
      if (print_value_moves){
        VG_(printf)("Owning %p (new rc %lu) as part of dynamic get.\n",
                    secondValue, secondValue->ref_count);
      }
    }
    ownShadowValue(firstValue);
    if (print_value_moves){
      VG_(printf)("Owning %p (new rc %lu) as part of dynamic get.\n",
                  firstValue, firstValue->ref_count);
    }
    tl_assert(secondValue->type == Ft_Single);
    ShadowTemp* temp = mkShadowTemp(2);
    temp->values[0] = firstValue;
    temp->values[1] = secondValue;
    return temp;
  }
}
VG_REGPARM(3) ShadowTemp* dynamicGet128(Int tsSrc,
                                        UWord bytes1,
                                        UWord bytes2){
  ShadowTemp* firstHalf = dynamicGet64(tsSrc, bytes1);
  ShadowTemp* secondHalf = dynamicGet64(tsSrc + sizeof(double), bytes2);
  if (firstHalf == NULL && secondHalf == NULL){
    return NULL;
  } else if (firstHalf == NULL){
    switch(secondHalf->num_vals){
    case 1:
      {
        if (print_types){
          VG_(printf)("Inferred type Doublex2 because "
                      "second half had a single double value.\n");
        }
        double value;
        VG_(memcpy)(&bytes1, &value, sizeof(double));
        firstHalf = mkShadowTempOneDouble(value);
      }
      break;
    case 2:
      {
        if (print_types){
          VG_(printf)("Inferred type Singlex4 because "
                      "second half had two single values.\n");
        }
        firstHalf = mkShadowTempTwoSingles(bytes1);
      }
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
      {
        double value;
        VG_(memcpy)(&bytes2, &value, sizeof(double));
        secondHalf = mkShadowTempOneDouble(value);
        if (print_types){
          VG_(printf)("Inferred type Doublex2 because "
                      "first half had a single double value.\n");
        }
      }
      break;
    case 2:
      {
        if (print_types){
          VG_(printf)("Inferred type Singlex4 because "
                      "second half had two single values.\n");
        }
        secondHalf = mkShadowTempTwoSingles(bytes2);
      }
      break;
    default:
      tl_assert(0);
      return NULL;
    }
  } else {
    if (print_types){
      if (firstHalf->num_vals == 1){
        VG_(printf)("Inferred type Doublex2 because "
                    "first half had a single double value.\n");
      } else {
        VG_(printf)("Inferred type Singlex4 because "
                    "second half had two single values.\n");
      }
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

VG_REGPARM(3) void setMemShadowTemp(UWord memDest,
                                    UWord size,
                                    ShadowTemp* st){
  for(int i = 0; i < size; ++i){
    UWord addr = memDest + i * sizeof(float);
    removeMemShadow(addr);
    if (st != NULL &&
        !(st->values[i/2]->type == Ft_Double && i % 2 == 1)){
      addMemShadow(addr, st->values[i/2]);
    }
  }
}
VG_REGPARM(2) ShadowTemp* getMemShadowTemp(UWord memSrc,
                                           UWord size){
  ShadowTemp* newTemp = mkShadowTemp(size);
  Bool allNull = True;
  for(int i = 0; i < size; ++i){
    UWord addr = memSrc + i * sizeof(float);
    newTemp->values[i] = getMemShadow(addr);
    if (newTemp->values[i] != NULL){
      allNull = False;
      ownShadowValue(newTemp->values[i]);
    }
  }
  if (allNull){
    freeShadowTemp(newTemp);
    return NULL;
  }
  return newTemp;
}
VG_REGPARM(1) ShadowValue* getMemShadow(UWord memSrc){
  ShadowMemEntry* entry = VG_(HT_lookup)(shadowMemory, memSrc);
  if (entry == NULL) return NULL;
  else return entry->val;
}
void removeMemShadow(UWord addr){
  ShadowMemEntry* oldEntry = VG_(HT_remove)(shadowMemory, addr);
  if (oldEntry == NULL) return;
  disownShadowValue(oldEntry->val);
  stack_push(memEntries, (void*)oldEntry);
}
void addMemShadow(UWord addr, ShadowValue* val){
  ShadowMemEntry* newEntry;
  if (stack_empty(memEntries)){
    newEntry = VG_(malloc)("memEntry", sizeof(ShadowMemEntry));
  } else {
    newEntry = (void*)stack_pop(memEntries);
  }
  newEntry->addr = addr;
  newEntry->val = val;
  ownShadowValue(val);
  VG_(HT_add_node)(shadowMemory, val);
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
inline
void stack_push_fast(Stack* s, StackNode* item_node){
  item_node->next = s->head;
  s->head = item_node;
}
void freeShadowValue(ShadowValue* val){
  if (val->influences != NULL){
    VG_(deleteXA)(val->influences);
    val->influences = NULL;
  }
  stack_push_fast(freedVals, (void*)val);
}

inline
StackNode* stack_pop_fast(Stack* s){
  StackNode* oldHead = s->head;
  s->head = oldHead->next;
  return oldHead;
}
ShadowValue* copyShadowValue(ShadowValue* val){
  ShadowValue* copy = mkShadowValueBare(val->type);
  copyReal(val->real, copy->real);
  return copy;
}
inline
ShadowValue* mkShadowValueBare(FloatType type){
  ShadowValue* result;
  if (stack_empty(freedVals)){
    result = newShadowValue(type);
    if (print_value_moves || print_allocs){
      VG_(printf)("Alloced new shadow value %p\n", result);
    }
  } else {
    result = (void*)stack_pop_fast(freedVals);
    result->type = type;
  }
  result->ref_count = 1;
  return result;
}
inline
ShadowValue* mkShadowValue(FloatType type, double value){
  ShadowValue* result = mkShadowValueBare(type);
  setReal(result->real, value);
  return result;
}

VG_REGPARM(1) ShadowTemp* copyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = mkShadowTemp(temp->num_vals);
  for(int i = 0; i < temp->num_vals; ++i){
    ownShadowValue(temp->values[i]);
    result->values[i] = temp->values[i];
    if (print_value_moves){
      VG_(printf)("Copying %p to new temp %p\n", temp->values[i],
                  result);
    }
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* deepCopyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = mkShadowTemp(temp->num_vals);
  for(int i = 0; i < temp->num_vals; ++i){
    result->values[i] = copyShadowValue(temp->values[i]);
    if (print_value_moves){
      VG_(printf)("Copying shadow value %p from temp %p to shadow value %p at temp %p\n",
                  temp->values[i], temp, result->values[i], result);
    }
  }
  return result;
}
inline
void disownShadowTemp(ShadowTemp* temp){
  for(int i = 0; i < temp->num_vals; ++i){
    if (print_value_moves){
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
    if (print_value_moves){
      VG_(printf)("Disowned last reference to %p! Freeing...\n", val);
    }
  } else {
    if (print_value_moves){
      VG_(printf)("Disowning %p, new ref count %lu\n", val, val->ref_count);
    }
  }
}
void ownShadowValue(ShadowValue* val){
  if (val == NULL) return;
  (val->ref_count)++;
  if (print_value_moves){
    VG_(printf)("Owning %p, new ref count %lu\n", val, val->ref_count);
  }
}

VG_REGPARM(1) ShadowTemp* mkShadowTempOneDouble(double value){
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = mkShadowValue(Ft_Double, value);
  if (print_value_moves){
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
  if (print_value_moves){
    VG_(printf)("Making values %p and %p as part of two double temp %p\n",
                result->values[0], result->values[1], result);
  }
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempOneSingle(double value){
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = mkShadowValue(Ft_Single, value);
  if (print_value_moves){
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
  if (print_value_moves){
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
  if (print_value_moves){
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
