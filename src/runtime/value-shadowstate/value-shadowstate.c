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

// Consider making this two-entrypoint function into a single
// entrypoint, as it might speed things up.

VG_REGPARM(1) void disownShadowTemp(ShadowTemp* temp){
  for(int i = 0; i < temp->num_vals; ++i){
    disownShadowValue(temp->values[i]);
  }
  freeShadowTemp(temp);
}
VG_REGPARM(1) ShadowTemp* copyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = newShadowTemp(temp->num_vals);
  for(int i = 0; i < temp->num_vals; ++i){
    ownShadowValue(temp->values[i]);
    result->values[i] = temp->values[i];
  }
  return result;
}
ShadowTemp* deepCopyShadowTemp(ShadowTemp* temp){
  ShadowTemp* result = newShadowTemp(temp->num_vals);
  for(int i = 0; i < temp->num_vals; ++i){
    result->values[i] = copyShadowValue(temp->values[i]);
  }
  return result;
}
void disownShadowValue(ShadowValue* val){
  (val->ref_count)--;
  if (val->ref_count < 1){
    disownExpr(val->expr);
    freeShadowValue(val);
  }
}
void ownShadowValue(ShadowValue* val){
  (val->ref_count)++;
}

void disownExpr(ConcExpr* expr){
  (expr->ref_count)--;
  if (expr->ref_count < 1){
    if (expr->type == Node_Branch){
      for(int i = 0; i < expr->branch.nargs; ++i){
        disownExpr(expr->branch.args[i]);
      }
    }
    freeExpr(expr);
  }
}

void ownExpr(ConcExpr* expr){
  (expr->ref_count)++;
}
