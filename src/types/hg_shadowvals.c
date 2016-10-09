
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie        hg_shadowvals.c ---*/
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

#include "hg_shadowvals.h"
#include "hg_stemtea.h"
#include "../include/hg_macros.h"
#include "../include/hg_options.h"

// Some basic valgrind stuff
#include "pub_tool_tooliface.h"
#include "pub_tool_mallocfree.h"

// Pull in this header file so that we can call the valgrind version
// of printf and dmsg.
#include "pub_tool_libcprint.h"

ShadowLocation* mkShadowLocation(LocType type){
  ShadowLocation* location = mkShadowLocation_bare(type);
  for (SizeT i = 0; i < capacity(type); ++i){
    location->values[i] = mkShadowValue();
  }
  return location;
}
ShadowLocation* mkShadowLocation_bare(LocType type){
  ShadowLocation* location;
  ALLOC(location, "hg.shadow_location.1", 1, sizeof(ShadowLocation));
  ALLOC(location->values, "hg.shadow_values", capacity(type), sizeof(ShadowValue*));
  location->type = type;
  return location;
}

void freeSL(ShadowLocation* sl){
  for (int i = 0; i < capacity(sl->type); ++i)
    if (sl->values[i] != NULL)
      disownSV(sl->values[i]);
  VG_(free)(sl->values);
  VG_(free)(sl);
}

void copySL(ShadowLocation* src, ShadowLocation** dest){
  if (src == (*dest)) return;
  if ((*dest) != NULL){
    freeSL(*dest);
  }
  if (src == NULL){
    (*dest) = NULL;
  } else {
    (*dest) = mkShadowLocation_bare(src->type);
    for (int i = 0; i < capacity(src->type); ++i){
      copySV(src->values[i], &((*dest)->values[i]));
    }
  }
}

void printShadowLoc(ShadowLocation* sl){
  if (sl == NULL)
    VG_(printf)("NULL");
  else {
    VG_(printf)("[");
    for (SizeT i = 0; i < capacity(sl->type); ++i){
      printShadowVal(sl->values[i]);
      if (i < capacity(sl->type) - 1){
        VG_(printf)(", ");
      }
    }
    VG_(printf)("]");
  }
}

void printShadowVal(ShadowValue* sv){
  if (sv == NULL)
    VG_(printf)("None");
  else
    printMPFRVal(sv->value);
}

void printMPFRVal(mpfr_t val){
  mpfr_exp_t shadowValexpt;
  char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                    val, MPFR_RNDN);
  VG_(printf)("%c.%se%ld", shadowValstr[0], shadowValstr + 1, shadowValexpt - 1);
  mpfr_free_str(shadowValstr);
}

SizeT capacity(LocType bytestype){
  tl_assert2(bytestype >= 0 && bytestype <= 8, "Bad location type %u", bytestype);
  switch(bytestype){
  case Lt_Float:
  case Lt_Double:
  case Lt_DoubleDouble:
  case Lt_DoubleDoubleDouble:
    return 1;
  case Lt_Floatx2:
  case Lt_Doublex2:
    return 2;
  case Lt_Floatx4:
  case Lt_Doublex4:
    return 4;
  case Lt_Floatx8:
    return 8;
  default:
    tl_assert(0);
    return 0;
  }
}
SizeT el_size(LocType bytestype){
  switch(bytestype){
  case Lt_Float:
  case Lt_Floatx2:
  case Lt_Floatx4:
  case Lt_Floatx8:
    return sizeof(float);
  case Lt_Double:
  case Lt_Doublex2:
  case Lt_Doublex4:
    return sizeof(double);
  case Lt_DoubleDouble:
    return sizeof(double) * 2;
  case Lt_DoubleDoubleDouble:
    return sizeof(double) * 4;
  }
  return 0;
}

ShadowValue* mkShadowValue(void){
  ShadowValue* result;
  ALLOC(result, "hg.shadow_val", 1, sizeof(ShadowValue));
  if (report_exprs){
    ALLOC(result->stem, "hg.shadow_stem", 1, sizeof(StemNode));
  }
  mpfr_init2(result->value, precision);
  result->ref_count = 0;
  if (print_moves)
    VG_(printf)("Making shadow value %p\n", result);
  return result;
}

void copySV(ShadowValue* src, ShadowValue** dest){
  if (src != NULL){
    addRef(src);
  }
  if ((*dest) != NULL) {
    disownSV(*dest);
  }
  (*dest) = src;
}

void disownSV(ShadowValue* sv){
  (sv->ref_count) --;
  if (print_counts){
    VG_(printf)("Decreasing count of %p to %lu\n", sv, sv->ref_count);
  }
  tl_assert(sv->ref_count >= 0);
  if (sv->ref_count < 1){
    if (print_moves)
      VG_(printf)("Cleaning up shadow value %p\n", sv);
    mpfr_clear(sv->value);
    if (report_exprs){
      tl_assert(sv == sv->stem->ref);
      cleanupStemNode(sv->stem);
    }
    VG_(free)(sv);
  }
}
void addRef(ShadowValue* val){
  (val->ref_count)++;
  if (print_counts){
    VG_(printf)("Increasing count of %p to %lu\n", val, val->ref_count);
  }
}
