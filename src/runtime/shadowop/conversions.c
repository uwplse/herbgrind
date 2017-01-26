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
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcbase.h"
#include "../value-shadowstate/value-shadowstate.h"

VG_REGPARM(1)
ShadowTemp* zeroHi96ofV128(ShadowTemp* input){
  ShadowTemp* result = mkShadowTemp(4);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  for(int i = 1; i < 4; ++i){
    result->values[i] = mkShadowValue(Ft_Single, 0.0);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* zeroHi64ofV128(ShadowTemp* input){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = mkShadowValue(Ft_Single, 0.0);
  return result;
}
VG_REGPARM(1)
ShadowTemp* v128to32(ShadowTemp* input){
  tl_assert(input->num_vals == 4);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
VG_REGPARM(1)
ShadowTemp* v128to64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
VG_REGPARM(1)
ShadowTemp* v128Hito64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[1];
  ownShadowValue(result->values[0]);
  return result;
}
VG_REGPARM(1)
ShadowTemp* f128Loto64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
VG_REGPARM(1)
ShadowTemp* f128Hito64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[1];
  ownShadowValue(result->values[0]);
  return result;
}
VG_REGPARM(2)
ShadowTemp* setV128lo32(ShadowTemp* topThree, ShadowTemp* bottomOne){
  tl_assert2(topThree->num_vals == 4,
             "Wrong number of values! Expected 4, got %d\n",
             topThree->num_vals);
  tl_assert(bottomOne->num_vals == 1);
  ShadowTemp* result = copyShadowTemp(topThree);
  disownShadowValue(result->values[0]);
  result->values[0] = bottomOne->values[0];
  ownShadowValue(result->values[0]);
  return result;
}
inline
VG_REGPARM(2)
ShadowTemp* setV128lo64(ShadowTemp* top, ShadowTemp* bottom){
  tl_assert2(top->num_vals == bottom->num_vals * 2,
             "Wrong number of values! First argument %p has %d values, "
             "and second argument %p has %d values.\n",
             top, top->num_vals, bottom, bottom->num_vals);
  ShadowTemp* result = copyShadowTemp(top);
  for (int i = 0; i < bottom->num_vals; ++i){
    result->values[i] = bottom->values[i];
    ownShadowValue(result->values[i]);
  }
  return result;
}
VG_REGPARM(3)
ShadowTemp* setV128lo64Dynamic2(ShadowTemp* top,
                                IRTemp bottomIdx, UWord bottomVal){
  ShadowTemp* bottom;
  if (top->num_vals == 2){
    double val;
    VG_(memcpy)(&val, &bottomVal, sizeof(double));
    bottom = mkShadowTempOneDouble(val);
  } else {
    bottom = mkShadowTempTwoSingles(bottomVal);
  }
  if (print_moves){
    VG_(printf)("Made %p for conversion dynamic 2.\n", bottom);
  }
  if (bottomIdx != IRTemp_INVALID){
    shadowTemps[bottomIdx] = bottom;
  }
  ShadowTemp* result = setV128lo64(top, bottom);
  if (bottomIdx == IRTemp_INVALID){
    disownShadowTemp(bottom);
  }
  return result;
}
VG_REGPARM(3)
ShadowTemp* setV128lo64Dynamic1(ShadowTemp* bottom,
                                IRTemp topIdx, UWord* topVal){
  ShadowTemp* top;
  if (bottom->num_vals == 1){
    top = mkShadowTempTwoDoubles((double*)topVal);
  } else {
    top = mkShadowTempFourSingles((float*)topVal);
  }
  if (topIdx != IRTemp_INVALID){
    shadowTemps[topIdx] = top;
  }
  ShadowTemp* result = setV128lo64(top, bottom);
  if (topIdx == IRTemp_INVALID){
    disownShadowTemp(top);
  }
  return result;
}
VG_REGPARM(2)
ShadowTemp* i64HLtoV128(ShadowTemp* hi, ShadowTemp* low){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] = hi->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = low->values[0];
  ownShadowValue(result->values[1]);
  return result;
}
VG_REGPARM(2)
ShadowTemp* f64HLtoF128(ShadowTemp* hi, ShadowTemp* low){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] = hi->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = low->values[0];
  ownShadowValue(result->values[1]);
  return result;
}
VG_REGPARM(2)
ShadowTemp* i64UtoV128(ShadowTemp* t){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] = t->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = mkShadowValue(Ft_Double, 0.0);
  return result;
}

VG_REGPARM(2)
ShadowTemp* i32UtoV128(ShadowTemp* t){
  ShadowTemp* result = mkShadowTemp(4);
  result->values[0] = t->values[0];
  ownShadowValue(result->values[0]);
  for (int i = 1; i < 4; ++i){
    result->values[i] = mkShadowValue(Ft_Single, 0.0);
  }
  if (print_value_moves){
    VG_(printf)("Copying shadow value %p to %p, "
                "and making values %p, %p, and %p "
                "as part of i32UtoV128\n",
                result->values[0], result,
                result->values[1], result->values[2], result->values[3]);
  }
  return result;
}
