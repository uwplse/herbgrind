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

ShadowTemp* shadowTemps[MAX_TEMPS];
ShadowValue* shadowThreadState[MAX_THREADS][MAX_REGISTERS];
VgHashTable* shadowMemory = NULL;

Stack* freedTemps[MAX_TEMP_SHADOWS];
Stack* freedVals;

void initValueShadowState(void){
  for(int i = 0; i < MAX_TEMP_SHADOWS; ++i){
    freedTemps[i] = mkStack();
  }
  freedVals = mkStack();
}

VG_REGPARM(2) void dynamicCleanup(int nentries, IRTemp* entries){
  for(int i = 0; i < nentries; ++i){
    ShadowTemp* temp = shadowTemps[entries[i]];
    if (temp == NULL) continue;
    for(int j = 0; j < temp->num_vals; ++j){
      if (temp->values[j] != NULL){
        disownShadowValue(temp->values[j]);
      }
    }
    freeShadowTemp(shadowTemps[entries[i]]);
    shadowTemps[entries[i]] = NULL;
  }
}

void freeShadowTemp(ShadowTemp* temp){
  stack_push(freedTemps[temp->num_vals - 1], (void*)temp);
}

inline
ShadowTemp* mkShadowTemp(UWord num_vals){
  if (stack_empty(freedTemps[num_vals - 1])){
    return newShadowTemp(num_vals);
  } else {
    return (void*)stack_pop(freedTemps[num_vals - 1]);
  }
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

inline
ShadowValue* mkShadowValueBare(FloatType type){
  ShadowValue* result;
  if (stack_empty(freedVals)){
    result = newShadowValue(type);
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
    if (temp->values[i] != NULL){
      ownShadowValue(temp->values[i]);
      result->values[i] = temp->values[i];
    }
  }
  return result;
}
ShadowTemp* deepCopyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = mkShadowTemp(temp->num_vals);
  for(int i = 0; i < temp->num_vals; ++i){
    if (temp->values[i] != NULL){
      result->values[i] = copyShadowValue(temp->values[i]);
    }
  }
  return result;
}
inline
void disownShadowTemp(ShadowTemp* temp){
  for(int i = 0; i < temp->num_vals; ++i){
    disownShadowValue(temp->values[i]);
  }
  freeShadowTemp(temp);
}
VG_REGPARM(1) void disownShadowTempNonNullDynamic(IRTemp idx){
  disownShadowTemp(shadowTemps[idx]);
}
VG_REGPARM(1) void disownShadowTempDynamic(IRTemp idx){
  if (shadowTemps[idx] != NULL){
    disownShadowTemp(shadowTemps[idx]);
  }
}
void disownShadowValue(ShadowValue* val){
  if (val == NULL) return;
  if (val->ref_count < 2){
    freeShadowValue(val);
  } else {
    (val->ref_count)--;
  }
}
void ownShadowValue(ShadowValue* val){
  if (val == NULL) return;
  (val->ref_count)++;
}

VG_REGPARM(1) ShadowTemp* mkShadowTempOneDouble(double value){
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = mkShadowValue(Ft_Double, value);
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoDoubles(double* values){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] =
    mkShadowValue(Ft_Double, values[0]);
  result->values[1] =
    mkShadowValue(Ft_Double, values[1]);
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempOneSingle(double value){
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = mkShadowValue(Ft_Single, value);
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoSingles(UWord values){
  ShadowTemp* result = mkShadowTemp(2);
  float floatValues[2];
  VG_(memcpy)(floatValues, &values, sizeof(floatValues));
  result->values[0] = mkShadowValue(Ft_Single, floatValues[0]);
  result->values[1] = mkShadowValue(Ft_Single, floatValues[1]);
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
  return result;
}
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSinglesG(UWord guard, float* values){
  if (!guard) return NULL;
  return mkShadowTempFourSingles(values);
}
