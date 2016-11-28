/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          conversions.h ---*/
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

#include "conversions.h"
#include "../value-shadowstate/value-shadowstate.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"

ShadowTemp* zeroHi96ofV128(ShadowTemp* input){
  ShadowTemp* result = newShadowTemp(4);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* zeroHi64ofV128(ShadowTemp* input){
  ShadowTemp* result = newShadowTemp(2);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* v128to32(ShadowTemp* input){
  tl_assert(input->num_vals == 4);
  ShadowTemp* result = newShadowTemp(1);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* v128to64(ShadowTemp* input){
  VG_(printf)("************V128to64**************");
  tl_assert(input);
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = newShadowTemp(1);
  tl_assert(input->values);
  tl_assert(input->values[0]);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* v128Hito64(ShadowTemp* input){
  VG_(printf)("******v128Hito64******");
  tl_assert(input);
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = newShadowTemp(1);
  tl_assert(input->values);
  tl_assert(input->values[1]);
  result->values[0] = input->values[1];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* setV128lo32(ShadowTemp* topThree, ShadowTemp* bottomOne){
  tl_assert(topThree->num_vals == 4);
  tl_assert(bottomOne->num_vals == 1);
  ShadowTemp* result = copyShadowTemp(topThree);
  result->values[0] = bottomOne->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* setV128lo64(ShadowTemp* topOne, ShadowTemp* bottomOne){
  tl_assert(topOne->num_vals == 2);
  tl_assert(bottomOne->num_vals == 1);
  ShadowTemp* result = copyShadowTemp(topOne);
  result->values[0] = bottomOne->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* f128Loto64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = newShadowTemp(1);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* f128Hito64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = newShadowTemp(1);
  result->values[0] = input->values[1];
  ownShadowValue(result->values[0]);
  return result;
}
ShadowTemp* i64HLtoV128(ShadowTemp* hi, ShadowTemp* low){
  VG_(printf)("******i64HLtoV128******");
  ShadowTemp* result = newShadowTemp(2);
  tl_assert(hi);
  tl_assert(low);
  tl_assert(hi->values);
  tl_assert(low->values);
  result->values[0] = hi->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = low->values[0];
  ownShadowValue(result->values[1]);
  return result;
}
ShadowTemp* f64HLtoF128(ShadowTemp* hi, ShadowTemp* low){
  ShadowTemp* result = newShadowTemp(2);
  result->values[0] = hi->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = low->values[0];
  ownShadowValue(result->values[1]);
  return result;
}
