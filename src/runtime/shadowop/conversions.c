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
  if (input->values[0] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(4));
  result->values[0] = input->values[0];
  tl_assert(input->values[0]->type == Vt_Single);
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
  if (input->values[0] == NULL && input->values[1] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(4));
  result->values[0] = input->values[0];
  result->values[1] = input->values[1];
  ownShadowValue(result->values[0]);
  ownShadowValue(result->values[1]);
  result->values[2] = NULL;
  result->values[3] = NULL;
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in zeroHi64ofV128\n",
                result->values[0],
                result->values[0]->ref_count);
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in zeroHi64ofV128\n",
                result->values[1],
                result->values[1]->ref_count);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* v128to32(ShadowTemp* input){
  tl_assert(INT(input->num_blocks) == 4);
  if (input->values[0] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(1));
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
  tl_assert(INT(input->num_blocks) == 4);
  if (input->values[0] == NULL && input->values[1] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(2));
  result->values[0] = input->values[0];
  result->values[1] = input->values[1];
  ownShadowValue(result->values[0]);
  ownShadowValue(result->values[1]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in v128to64\n",
                result->values[0],
                result->values[0]->ref_count);
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in v128to64\n",
                result->values[1],
                result->values[1]->ref_count);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* v128Hito64(ShadowTemp* input){
  tl_assert(INT(input->num_blocks) == 4);
  if (input->values[0] == NULL && input->values[1] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(2));
  result->values[0] = input->values[2];
  result->values[1] = input->values[3];
  ownShadowValue(result->values[0]);
  ownShadowValue(result->values[1]);
  if (PRINT_VALUE_MOVES){
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in v128Hito64\n",
                result->values[0],
                result->values[0]->ref_count);
    VG_(printf)("Owning value %p (new ref count %lu) "
                "copied in v128Hito64\n",
                result->values[0],
                result->values[0]->ref_count);
  }
  return result;
}
VG_REGPARM(1)
ShadowTemp* f128Loto64(ShadowTemp* input){
  tl_assert2(0, "This operation is currently not supported.\n");
  return NULL;
}
VG_REGPARM(1)
ShadowTemp* f128Hito64(ShadowTemp* input){
  tl_assert2(0, "This operation is currently not supported.\n");
  return NULL;
}
VG_REGPARM(2)
ShadowTemp* setV128lo32(ShadowTemp* topThree, ShadowTemp* bottomOne){
  tl_assert2(INT(topThree->num_blocks) == 4,
             "Wrong number of blocks! Expected 4, got %d\n",
             INT(topThree->num_blocks));
  tl_assert(INT(bottomOne->num_blocks) == 1);
  if (bottomOne->values[0] == NULL &&
      topThree->values[1] == NULL &&
      topThree->values[2] == NULL &&
      topThree->values[3] == NULL){
    return NULL;
  }
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
  tl_assert(INT(top->num_blocks) == 4);
  tl_assert(INT(bottom->num_blocks) == 2);
  if (bottom->values[0] == NULL &&
      bottom->values[1] == NULL &&
      top->values[2] == NULL &&
      top->values[3] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(4));
  for(int i = 0; i < 2; ++i){
    result->values[i] = bottom->values[i];
    ownShadowValue(result->values[i]);
  }
  for(int i = 2; i < 4; ++i){
    result->values[i] = top->values[i];
    ownShadowValue(result->values[i]);
  }
  return result;
}
VG_REGPARM(3)
ShadowTemp* setV128lo64Dynamic2(ShadowTemp* top,
                                IRTemp bottomIdx, UWord bottomVal){
  tl_assert(INT(top->num_blocks) == 4);
  if (top->values[2] == NULL && top->values[3] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(4));
  if (top->values[2] != NULL && top->values[2]->type == Vt_Double){
    double val = *(double*)(void*)&bottomVal;
    result->values[0] = mkShadowValue(Vt_Double, val);
    result->values[1] = NULL;
    result->values[2] = top->values[2];
    result->values[3] = top->values[3]; // NULL
    ownShadowValue(top->values[2]);
    if (PRINT_TEMP_MOVES){
      VG_(printf)("Made %p with two doubles because %p has two doubles.\n",
                  result, top);
    }
  } else if ((top->values[2] != NULL && top->values[2]->type == Vt_Single) ||
             (top->values[3] != NULL && top->values[3]->type == Vt_Single)){
    float floatValues[2];
    VG_(memcpy)(floatValues, &bottomVal, sizeof(floatValues));
    result->values[0] = mkShadowValue(Vt_Single, floatValues[0]);
    result->values[1] = mkShadowValue(Vt_Single, floatValues[1]);
    result->values[2] = top->values[2];
    result->values[3] = top->values[3];
    ownShadowValue(top->values[2]);
    ownShadowValue(top->values[3]);
    if (PRINT_TEMP_MOVES){
      VG_(printf)("Made %p with four singles because "
                  "%p has four singles.\n",
                  result, top);
    }
  } else {
    tl_assert(0);
  }
  return result;
}

VG_REGPARM(3)
ShadowTemp* setV128lo64Dynamic1(ShadowTemp* bottom,
                                IRTemp topIdx, UWord* topVal){
  tl_assert(INT(bottom->num_blocks) == 2);
  if (bottom->values[0] == NULL && bottom->values[1] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(4));
  if (bottom->values[0] != NULL && bottom->values[0]->type == Vt_Double){
    double val = *(double*)(void*)&(topVal[1]);
    result->values[0] = bottom->values[0];
    result->values[1] = bottom->values[1]; // NULL
    result->values[2] = mkShadowValue(Vt_Double, val);
    result->values[3] = NULL;
    ownShadowValue(bottom->values[0]);
    if (PRINT_TEMP_MOVES){
      VG_(printf)("Made %p with two doubles because %p has one double.\n",
                  result, bottom);
    }
  } else if ((bottom->values[0] != NULL && bottom->values[0]->type == Vt_Single) ||
             (bottom->values[1] != NULL && bottom->values[1]->type == Vt_Single)){
    float floatValues[2];
    VG_(memcpy)(floatValues, &topVal, sizeof(floatValues));
    result->values[0] = bottom->values[0];
    result->values[1] = bottom->values[1];
    result->values[2] = mkShadowValue(Vt_Single, floatValues[0]);
    result->values[3] = mkShadowValue(Vt_Single, floatValues[1]);
    ownShadowValue(bottom->values[0]);
    ownShadowValue(bottom->values[1]);
    if (PRINT_TEMP_MOVES){
      VG_(printf)("Made %p with four singles because "
                  "%p has two singles.\n",
                  result, bottom);
    }
  } else {
    tl_assert(0);
  }
  return result;
}
VG_REGPARM(2)
ShadowTemp* i64HLtoV128NoFirstShadow(UWord hi, ShadowTemp* lo){
  ShadowTemp* hiShadow;
  if (lo->values[0] == NULL && lo->values[1] == NULL){
    return NULL;
  }
  if (lo->values[0]->type == Vt_Double){
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
  if (hi->values[2] == NULL && hi->values[3] == NULL){
    return NULL;
  }
  if (hi->values[0]->type == Vt_Double){
    loShadow = mkShadowTempTwoSingles(lo);
  } else {
    loShadow = mkShadowTempOneDouble(*(double*)&lo);
  }
  ShadowTemp* result = i64HLtoV128(hi, loShadow);
  return result;
}
VG_REGPARM(2)
ShadowTemp* i64HLtoV128(ShadowTemp* hi, ShadowTemp* lo){
  if (hi->values[0] == NULL && hi->values[1] == NULL &&
      lo->values[0] == NULL && lo->values[1] == NULL){
    return NULL;
  }
  if ((hi->values[0] != NULL && hi->values[0]->type == Vt_Double) ||
      (lo->values[0] != NULL && lo->values[0]->type == Vt_Double)){
    tl_assert((hi->values[0] == NULL || hi->values[0]->type == Vt_Double) &&
              (lo->values[0] == NULL || lo->values[0]->type == Vt_Double));
    ShadowTemp* result = mkShadowTemp(FB(4));
    result->values[0] = hi->values[0];
    result->values[1] = NULL;
    result->values[2] = lo->values[0];
    result->values[3] = NULL;
    ownShadowValue(result->values[0]);
    ownShadowValue(result->values[2]);
    if (PRINT_VALUE_MOVES){
      VG_(printf)("Owning values %p (rc %lu) and %p (rc %lu), "
                  "copied in i64HLtoV128\n",
                  result->values[0], result->values[0]->ref_count,
                  result->values[1], result->values[1]->ref_count);
    }
    return result;
  } else if ((hi->values[0] != NULL && hi->values[0]->type == Vt_Single) ||
             (hi->values[1] != NULL && hi->values[1]->type == Vt_Single) ||
             (lo->values[0] != NULL && lo->values[0]->type == Vt_Single) ||
             (lo->values[1] != NULL && lo->values[1]->type == Vt_Single)){
    tl_assert(hi->values[0] == NULL || hi->values[0]->type == Vt_Single);
    tl_assert(hi->values[1] == NULL || hi->values[1]->type == Vt_Single);
    tl_assert(lo->values[0] == NULL || lo->values[0]->type == Vt_Single);
    tl_assert(lo->values[0] == NULL || lo->values[1]->type == Vt_Single);
    ShadowTemp* result = mkShadowTemp(FB(4));
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
  } else {
    tl_assert(0);
    return NULL;
  }
}
VG_REGPARM(2)
ShadowTemp* f64HLtoF128(ShadowTemp* hi, ShadowTemp* lo){
  if (hi->values[0] == NULL && hi->values[1] == NULL &&
      lo->values[0] == NULL && lo->values[1] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(4));
  result->values[0] = hi->values[0];
  result->values[1] = hi->values[1];
  result->values[2] = lo->values[0];
  result->values[3] = lo->values[1];
  ownShadowValue(result->values[0]);
  ownShadowValue(result->values[1]);
  ownShadowValue(result->values[2]);
  ownShadowValue(result->values[3]);
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
  if (t->values[0] == NULL && t->values[1] == NULL){
    return NULL;
  }
  ShadowTemp* result = mkShadowTemp(FB(4));
  result->values[0] = t->values[0];
  result->values[1] = t->values[1];
  result->values[2] = NULL;
  result->values[3] = NULL;
  ownShadowValue(result->values[0]);
  ownShadowValue(result->values[1]);
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
  tl_assert(t->values[0] != NULL);
  ShadowTemp* result = mkShadowTemp(FB(4));
  result->values[0] = t->values[0];
  ownShadowValue(result->values[0]);
  for (int i = 1; i < 4; ++i){
    result->values[i] = NULL;
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
  tl_assert(INT(t->num_blocks) == 1);
  tl_assert2(t->values[0] != NULL, "Temp %p is missing a value!\n", t);
  tl_assert(t->values[0]->type == Vt_Single);
  ShadowTemp* result = mkShadowTemp(FB(2));
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
  tl_assert(INT(t->num_blocks) == 2);
  tl_assert(t->values[0] != NULL);
  ShadowTemp* result = mkShadowTemp(FB(1));
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
