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
#include "pub_tool_mallocfree.h"

ShadowTemp* newShadowTemp(int num_vals){
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
ShadowValue* newShadowValue(void){
  ShadowValue* result = VG_(malloc)("shadow value", sizeof(ShadowValue));
  result->ref_count = 1;
  result->type = Ft_Double;
  return result;
}
ShadowValue* newShadowValueF(void){
  ShadowValue* result = VG_(malloc)("shadow value", sizeof(ShadowValue));
  result->ref_count = 1;
  result->type = Ft_Single;
  return result;
}
void freeShadowValue(ShadowValue* val){
  freeReal(val->real);
  VG_(free)(val);
}
