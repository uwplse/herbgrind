/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                 real.c ---*/
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

#include "real.h"

#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"

Real mkReal(double bytes){
  Real result = VG_(malloc)("real", sizeof(struct _RealStruct));
  #ifdef USE_MPFR
  mpfr_init2(result->mpfr_val, precision);
  mpfr_set_d(result->mpfr_val, bytes, MPFR_RNDN);
  #else
  mpf_init2(result->mpf_val, precision);
  mpf_set_d(result->mpf_val, bytes);
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
  #ifdef USE_MPFR
  return mpfr_get_d(real->mpfr_val, MPFR_RNDN);
  #else
  return mpf_get_d(real->mpf_val);
  #endif
}

Real copyReal(Real real){
  Real result = VG_(malloc)("real", sizeof(struct _RealStruct));
  #ifdef USE_MPFR
  mpfr_init2(result->mpfr_val, precision);
  mpfr_set(result->mpfr_val, real->mpfr_val, MPFR_RNDN);
  #else
  mpf_init2(result->mpf_val, precision);
  mpf_set(result->mpf_val, real->mpf_val);
  #endif
  return result;
}
