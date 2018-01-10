/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie          conversions.h ---*/
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
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in zeroHi96ofV128\n",
                result->values[0], result->values[0]->ref_count);
  }
  for(int i = 1; i < 4; ++i){
    result->values[i] = mkShadowValue(Vt_Single, 0.0);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Making shadow value %p as part of zeroHi96ofV128.\n",
                  result->values[i]);
    }
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* zeroHi64ofV128(ShadowTemp* input){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in zeroHi64ofV128\n",
                result->values[0],
                result->values[0]->ref_count);
  }
  result->values[1] = mkShadowValue(Vt_Single, 0.0);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making shadow value %p as part of zeroHi64ofV128.\n",
                result->values[1]);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* v128to32(ShadowTemp* input){
  tl_assert(input->num_vals == 4);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in v128to32\n",
                result->values[0],
                result->values[0]->ref_count);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* v128to64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in v128to64\n",
                result->values[0],
                result->values[0]->ref_count);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* v128Hito64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[1];
  ownShadowValue(result->values[0]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in v128Hito64\n",
                result->values[0],
                result->values[0]->ref_count);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* f128Loto64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[0];
  ownShadowValue(result->values[0]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in f128Loto64\n",
                result->values[0],
                result->values[0]->ref_count);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* f128Hito64(ShadowTemp* input){
  tl_assert(input->num_vals == 2);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = input->values[1];
  ownShadowValue(result->values[0]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in f128Hito64\n",
                result->values[0],
                result->values[0]->ref_count);
  }
  return result;
}
VG_REGPARM(2)
ShadowTemp* setV128lo32(ShadowTemp* topThree, ShadowTemp* bottomOne){
  tl_assert2(topThree->num_vals == 4,
             "Wrong number of values! Expected 4, got %d\n",
             topThree->num_vals);
  tl_assert(bottomOne->num_vals == 1);
  ShadowTemp* result = copyShadowTemp(topThree);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Disowning extreniously copied value %p (old rc %lu)\n",
                result->values[0], result->values[0]->ref_count);
  }
  disownShadowValue(result->values[0]);
  result->values[0] = bottomOne->values[0];
  ownShadowValue(result->values[0]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in setV128lo32\n",
                result->values[0],
                result->values[0]->ref_count);
  }
  return result;
}
inline
VG_REGPARM(2)
ShadowTemp* setV128lo64(ShadowTemp* top, ShadowTemp* bottom){
  /* tl_assert2(top->num_vals == bottom->num_vals * 2, */
  /*            "Wrong number of values! First argument %p has %d values, " */
  /*            "and second argument %p has %d values.\n", */
  /*            top, top->num_vals, bottom, bottom->num_vals); */
  if (top->num_vals == bottom->num_vals * 2){
    ShadowTemp* result = copyShadowTemp(top);
    if (print_types){
      VG_(printf)("Inferred result of setV128lo64 to have %d values, because top and bottom match.\n",
                  top->num_vals);
    }
    for (int i = 0; i < bottom->num_vals; ++i){
      result->values[i] = bottom->values[i];
      ownShadowValue(result->values[i]);
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Owning value %p (new ref count %lu) "
                    "copied in setV128lo64\n",
                    result->values[i],
                    result->values[i]->ref_count);
      }
    }
    return result;
  } else {
    // Mixed reads are a thing, and hopefully mean the program is only
    // going to look at the bottom value, because otherwise all sane
    // semantics are fucked.
    if (top->num_vals == 4 && bottom->num_vals == 1){
      if (print_types){
        VG_(printf)("Inferred result of setV128lo64 to have %d values, "
                    "because it's a mixed read and the top has %d values.\n",
                    top->num_vals, top->num_vals);
      }
      ShadowTemp* result = mkShadowTemp(2);
      result->values[0] = bottom->values[0];
      ownShadowValue(result->values[0]);
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Owning value %p (new ref count %lu) "
                    "copied in setV128lo64\n",
                    result->values[0],
                    result->values[0]->ref_count);
      }
      float v3 = getDouble(top->values[2]->real);
      float v4 = getDouble(top->values[3]->real);
      // #suuuuupersketch
      double combined;
      VG_(memcpy)(&v3, &combined, sizeof(float));
      VG_(memcpy)(&v4, (&combined) + sizeof(float), sizeof(float));
      result->values[1] = mkShadowValue(Vt_Double, combined);
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Made shadow value %p for reinterpreted bits of second half of V128\n",
                    result->values[1]);
      }
      return result;
    } else if (top->num_vals == 2 && bottom->num_vals == 2){
      if (print_types){
        VG_(printf)("Inferred result of setV128lo64 to have %d values, "
                    "because it's a mixed read and the tope has %d values.\n",
                    top->num_vals, top->num_vals);
      }
      ShadowTemp* result = mkShadowTemp(4);
      result->values[0] = bottom->values[0];
      result->values[1] = bottom->values[1];
      ownShadowValue(result->values[0]);
      ownShadowValue(result->values[1]);
      if (PRINT_VALUE_MOVES){
        VG_(printf)("Owning values %p (new ref count %lu) "
                    "and %p (new ref count %lu) "
                    "copied in setV128lo64\n",
                    result->values[0],
                    result->values[0]->ref_count,
                    result->values[1],
                    result->values[1]->ref_count);
      }
      double combined = getDouble(top->values[1]->real);
      float f3, f4;
      VG_(memcpy)(&combined, &f3, sizeof(float));
      VG_(memcpy)((&combined) + sizeof(float), &f4, sizeof(float));
      result->values[2] = mkShadowValue(Vt_Single, f3);
      result->values[3] = mkShadowValue(Vt_Single, f4);
      return result;
    } else {
      tl_assert(0);
    }
  }
}
VG_REGPARM(3)
ShadowTemp* setV128lo64Dynamic2(ShadowTemp* top,
                                IRTemp bottomIdx, UWord bottomVal){
  ShadowTemp* bottom;
  if (top->num_vals == 2){
    if (print_types){
      VG_(printf)("Inferred result of setV128lo64 to have %d values, because top has that many values.\n",
                  top->num_vals);
    }
    double val;
    VG_(memcpy)(&val, &bottomVal, sizeof(double));
    bottom = mkShadowTempOneDouble(val);
    if (print_temp_moves){
      VG_(printf)("Made %p with one double because %p has two doubles.\n",
                  bottom, top);
    }
  } else {
    if (print_types){
      VG_(printf)("Inferred result of setV128lo64 to have %d values, because top has that many values.\n",
                  top->num_vals);
    }
    bottom = mkShadowTempTwoSingles(bottomVal);
    if (print_temp_moves){
      VG_(printf)("Made %p with two singles because "
                  "%p has four singles.\n",
                  bottom, top);
    }
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
    if (print_types){
      VG_(printf)("Inferred result of setV128lo64 to have %d values, because bottom has that many values.\n",
                  bottom->num_vals);
    }
    top = mkShadowTempTwoDoubles((double*)topVal);
    if (print_temp_moves){
      VG_(printf)("Made %p with two doubles because %p has one double.\n",
                  top, bottom);
    }
  } else {
    if (print_types){
      VG_(printf)("Inferred result of setV128lo64 to have %d values, because bottom has that many values.\n",
                  bottom->num_vals);
    }
    top = mkShadowTempFourSingles((float*)topVal);
    if (print_temp_moves){
      VG_(printf)("Made %p with four singles because "
                  "%p has two singles.\n",
                  top, bottom);
    }
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
ShadowTemp* i64HLtoV128NoFirstShadow(UWord hi, ShadowTemp* lo){
  ShadowTemp* hiShadow;
  if (lo->num_vals == 2){
    hiShadow = mkShadowTempTwoSingles(hi);
  } else {
    hiShadow = mkShadowTempOneDouble(*(double*)&hi);
  }
  ShadowTemp* result = i64HLtoV128(hiShadow, lo);
  return result;
}
VG_REGPARM(2)
ShadowTemp* i64HLtoV128NoSecondShadow(ShadowTemp* hi, UWord lo){
  ShadowTemp* loShadow;
  if (hi->num_vals == 2){
    loShadow = mkShadowTempTwoSingles(lo);
  } else {
    loShadow = mkShadowTempOneDouble(*(double*)&lo);
  }
  ShadowTemp* result = i64HLtoV128(hi, loShadow);
  return result;
}
VG_REGPARM(2)
ShadowTemp* i64HLtoV128(ShadowTemp* hi, ShadowTemp* lo){
  if (hi->values[0]->type == Vt_Double){
    tl_assert2(hi->values[0]->type == Vt_Double,
               "Type is instead %d", hi->values[0]->type);
    tl_assert(lo->values[0]->type == Vt_Double);
    ShadowTemp* result = mkShadowTemp(2);
    result->values[0] = hi->values[0];
    ownShadowValue(result->values[0]);
    result->values[1] = lo->values[0];
    ownShadowValue(result->values[1]);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Owning values %p (rc %lu) and %p (rc %lu), "
                  "copied in i64HLtoV128\n",
                  result->values[0], result->values[0]->ref_count,
                  result->values[1], result->values[1]->ref_count);
    }
    return result;
  } else {
    tl_assert(hi->values[0]->type == Vt_Single);
    tl_assert(hi->values[1]->type == Vt_Single);
    tl_assert(lo->values[0]->type == Vt_Single);
    tl_assert(lo->values[1]->type == Vt_Single);
    ShadowTemp* result = mkShadowTemp(4);
    result->values[0] = hi->values[0];
    ownShadowValue(result->values[0]);
    result->values[1] = hi->values[1];
    ownShadowValue(result->values[1]);
    result->values[2] = lo->values[0];
    ownShadowValue(result->values[2]);
    result->values[3] = lo->values[1];
    ownShadowValue(result->values[3]);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Owning values %p (rc %lu), %p (rc %lu), "
                  "%p (rc %lu), and %p (rc %lu), "
                  "copied in i64HLtoV128",
                  result->values[0], result->values[0]->ref_count,
                  result->values[1], result->values[1]->ref_count,
                  result->values[2], result->values[2]->ref_count,
                  result->values[3], result->values[3]->ref_count);
    }
    return result;
  }
}
VG_REGPARM(2)
ShadowTemp* f64HLtoF128(ShadowTemp* hi, ShadowTemp* low){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] = hi->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = low->values[0];
  ownShadowValue(result->values[1]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning values %p (rc %lu) and %p (rc %lu), "
                "copied in f64HL to F128\n",
                result->values[0], result->values[0]->ref_count,
                result->values[1], result->values[1]->ref_count);
  }
  return result;
}
VG_REGPARM(2)
ShadowTemp* i64UtoV128(ShadowTemp* t){
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] = t->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = mkShadowValue(Vt_Double, 0.0);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Making shadow value %p and owning %p (rc %lu) "
                "as part of i64UtoV128\n",
                result->values[1],
                result->values[0], result->values[0]->ref_count);
  }
  return result;
}

VG_REGPARM(2)
ShadowTemp* i32UtoV128(ShadowTemp* t){
  ShadowTemp* result = mkShadowTemp(4);
  result->values[0] = t->values[0];
  ownShadowValue(result->values[0]);
  for (int i = 1; i < 4; ++i){
    result->values[i] = mkShadowValue(Vt_Single, 0.0);
  }
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Copying shadow value %p to %p, "
                "and making values %p, %p, and %p "
                "as part of i32UtoV128\n",
                result->values[0], result,
                result->values[1], result->values[2], result->values[3]);
  }
  return result;
}

VG_REGPARM(2)
ShadowTemp* i32Uto64(ShadowTemp* t){
  tl_assert(t);
  tl_assert(t->num_vals == 1);
  tl_assert(t->values[0] != NULL);
  tl_assert(t->values[0]->type == Vt_Single);
  ShadowTemp* result = mkShadowTemp(2);
  result->values[0] = t->values[0];
  ownShadowValue(result->values[0]);
  result->values[1] = mkShadowValue(Vt_Single, 0.0);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Copying shadow value %p to %p, "
                "and making value %p "
                "as part of i32Uto64\n",
                result->values[0], result,
                result->values[1]);
  }
  return result;
}

VG_REGPARM(2)
ShadowTemp* i64to32(ShadowTemp* t){
  tl_assert(t);
  tl_assert(t->num_vals == 2);
  tl_assert(t->values[0] != NULL);
  ShadowTemp* result = mkShadowTemp(1);
  result->values[0] = t->values[0];
  tl_assert(t->values[0]);
  ownShadowValue(result->values[0]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Copying shadow value %p to %p, "
                "as part of 64to32\n",
                result->values[0], result);
  }
  return result;
}
