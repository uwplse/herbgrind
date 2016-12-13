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

VG_REGPARM(2) void dynamicCleanup(int nentries, TempDebtEntry* entries){
  for(int i = 0; i < nentries; ++i){
    freeShadowTemp(shadowTemps[entries[i].temp]);
    shadowTemps[entries[i].temp] = NULL;
  }
}

void freeShadowTemp(ShadowTemp* temp){
  stack_push(freedTemps[temp->num_vals - 1], (void*)temp);
}

ShadowTemp* mkShadowTemp(UWord num_vals){
  if (stack_empty(freedTemps[num_vals - 1])){
    return newShadowTemp(num_vals);
  } else {
    return (void*)stack_pop(freedTemps[num_vals - 1]);
  }
}

void freeShadowValue(ShadowValue* val){
  if (val->influences != NULL){
    VG_(deleteXA)(val->influences);
    val->influences = NULL;
  }
  stack_push(freedVals, (void*)val);
}

ShadowValue* mkShadowValue(FloatType type, double value){
  ShadowValue* result;
  if (stack_empty(freedVals)){
    result = newShadowValue(type, value);
  } else {
    result = (void*)stack_pop(freedVals);
    setReal(result->real, value);
    result->type = type;
  }
  result->ref_count = 1;
  return result;
}

VG_REGPARM(1) void disownShadowTemp(ShadowTemp* temp){
  /* if (temp == NULL) return; */
  /* for(int i = 0; i < temp->num_vals; ++i){ */
  /*   if (temp->values[i] != NULL){ */
  /*     disownShadowValue(temp->values[i]); */
  /*   } */
  /* } */
  freeShadowTemp(temp);
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
void disownShadowValue(ShadowValue* val){
  if (val == NULL) return;
  (val->ref_count)--;
  if (val->ref_count < 1){
    freeShadowValue(val);
  }
}
void ownShadowValue(ShadowValue* val){
  if (val == NULL) return;
  (val->ref_count)++;
}
