/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie            shadowval.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017Alex Sanchez-Stern

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

#include "pub_tool_mallocfree.h"
#include "pub_tool_hashtable.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"

VG_REGPARM(1) ShadowTemp* newShadowTemp(UWord num_vals){
  ShadowTemp* newShadowTemp =
    VG_(perm_malloc)(sizeof(ShadowTemp), vg_alignof(ShadowTemp));
  newShadowTemp->num_vals = num_vals;
  tl_assert(num_vals > 0);
  newShadowTemp->values =
    VG_(perm_malloc)(num_vals * sizeof(ShadowValue*), vg_alignof(ShadowValue*));
  return newShadowTemp;
}
void changeSingleValueType(ShadowTemp* temp, ValueType type){
  if (temp->values[0] != NULL){
    temp->values[0]->type = type;
  }
}
UWord hashDouble(double val){
  UWord result;
  VG_(memcpy)(&result, &val, sizeof(UWord));
  return result;
}
inline
ShadowValue* newShadowValue(ValueType type){
  ShadowValue* result =
    VG_(perm_malloc)(sizeof(ShadowValue), vg_alignof(ShadowValue));
  result->type = type;
  result->ref_count = 1;
  if (!no_reals){
    result->real = mkReal();
  }
  return result;
}
void updateRanges(RangeRecord* records, double* args, int nargs){
  for (int i = 0; i < nargs; ++i){
    updateRangeRecord(records + i, args[i]);
  }
}

VG_REGPARM(2) void assertValValid(const char* label, ShadowValue* val){
  tl_assert2(val->real != NULL, "%s: value is %p", label, val);
}
VG_REGPARM(2) void assertTempValid(const char* label, ShadowTemp* temp){
  for(int i = 0; i < temp->num_vals; ++i){
    tl_assert2(temp->values[i] != NULL,
               "%s: Value %d of temp %p is NULL!",
               label, i, temp);
    assertValValid(label, temp->values[i]);
  }
}
VG_REGPARM(3) void assertValType(const char* label, ShadowValue* val, ValueType type){
  tl_assert2(val->type == type,
             "%s: Expected type %d for %p, got %d\n",
             label, type, val, val->type);
}
VG_REGPARM(3) void assertTempType(const char* label, ShadowTemp* temp, ValueType type){
  for(int i = 0; i < temp->num_vals; ++i){
    tl_assert2(temp->values[i]->type == type,
               "%s: Expected type %d for %p, got %d\n",
               label, type, temp->values[i], temp->values[i]->type);
  }
}
VG_REGPARM(3) void assertNumVals(const char* label, ShadowTemp* temp,
                                 int num_vals){
  tl_assert2(temp->num_vals == num_vals,
             "%s: Expected %d vals in %p, got %d\n",
             label, num_vals, temp, temp->num_vals);
}
VG_REGPARM(3) void assertNumValsNot(const char* label,
                                    ShadowTemp* temp,
                                    int num_vals){
  tl_assert2(temp->num_vals != num_vals,
             "%s: Expected not %d vals in %p, got %d\n",
             label, num_vals, temp, temp->num_vals);
}

VG_REGPARM(3) void assertDynamicSize(const char* label,
                                     ShadowTemp* temp,
                                     int num_halfwords){
  if (temp == NULL) return;
  if (temp->values[0]->type == Vt_Single){
    tl_assert2(temp->num_vals == num_halfwords,
               "%s: Expected %d vals in %p (of type Single), got %d",
               label, num_halfwords, temp, temp->num_vals);
    for(int i = 1; i < temp->num_vals; ++i){
      tl_assert2(temp->values[i]->type == Vt_Single,
                 "%s: Value %d in %p is not a Single, but value 0 is!",
                 label, i, temp);
    }
  } else {
    tl_assert2(temp->values[0]->type == Vt_Double,
               "%s: Invalid type %d!",temp->values[0]->type);
    tl_assert2(temp->num_vals == num_halfwords / 2,
               "%s: Expected %d vals in %p (of type Double), got %d",
               label, num_halfwords / 2, temp, temp->num_vals);
    for(int i = 1; i < temp->num_vals; ++i){
      tl_assert2(temp->values[i]->type == Vt_Double,
                 "%s: Value %d in %p is not a Double, but value 0 is!",
                 label, i, temp);
    }
  }
}
