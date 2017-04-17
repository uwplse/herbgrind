/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie                 real.c ---*/
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

#include "real.h"

#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"

Real mkReal(void){
  Real result = VG_(malloc)("real", sizeof(struct _RealStruct));
  #ifdef USE_MPFR
  mpfr_init2(result->mpfr_val, precision);
  #else
  mpf_init2(result->mpf_val, precision);
  #endif
  return result;
}
void setReal(Real r, double bytes){
  #ifdef USE_MPFR
  mpfr_set_d(r->mpfr_val, bytes, MPFR_RNDN);
  #else
  mpf_set_d(r->mpf_val, bytes);
  #endif
}
void freeReal(Real real){
  #ifdef USE_MPFR
  mpfr_clear(real->mpfr_val);
  #else
  mpf_clear(real->mpf_val);
  #endif
  VG_(free)(real);
}

double getDouble(Real real){
  if (no_reals) return 0.0;
  #ifdef USE_MPFR
  return mpfr_get_d(real->mpfr_val, MPFR_RNDN);
  #else
  return mpf_get_d(real->mpf_val);
  #endif
}

void copyReal(Real src, Real dest){
  #ifdef USE_MPFR
  mpfr_set(dest->mpfr_val, src->mpfr_val, MPFR_RNDN);
  #else
  mpf_set(dest->mpf_val, src->mpf_val);
  #endif
}

void printReal(Real real){
  #ifdef USE_MPFR
  char* shadowValStr;
  mpfr_exp_t shadowValExpt;

  shadowValStr = mpfr_get_str(NULL, &shadowValExpt, 10, longprint_len, real->mpfr_val, MPFR_RNDN);
  VG_(printf)("%c.%se%ld", shadowValStr[0], shadowValStr+1, shadowValExpt-1);
  mpfr_free_str(shadowValStr);
  #else
  tl_assert2(0, "Can't print GMP vals!\n");
  #endif
}
