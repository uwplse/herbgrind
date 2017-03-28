/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie               realop.h ---*/
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

#ifndef _REALOP_H
#define _REALOP_H

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "../value-shadowstate/shadowval.h"
#include "../../options.h"

#ifdef USE_MPFR
#include "mpfr.h"
#define RVAL mpfr_val
#define CALL1(f, result, arg) mpfr_##f(result, arg, MPFR_RNDN)
#define CALL2(f, result, arg1, arg2) \
  mpfr_##f(result, arg1, arg2, MPFR_RNDN)
#define CALL3(f, result, arg1, arg2, arg3) \
  mpfr_##f(result, arg1, arg2, arg3, MPFR_RNDN)
#define DEF1(f) \
  int mpfr_##f(mpfr_t res, mpfr_srcptr arg, mpfr_rnd_t round)
#define DEF2(f) \
  int mpfr_##f(mpfr_t res, \
               mpfr_srcptr arg1, mpfr_srcptr arg2, \
               mpfr_rnd_t round)
#define DEF3(f) \
  int mpfr_##f(mpfr_t res, \
               mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_srcptr arg3,   \
               mpfr_rnd_t round)
#define GETD(v)                                 \
  mpfr_get_d(v, MPFR_RNDN)
#define RET return
#else
#include "gmp.h"
#define RVAL mpf_val
#define CALL1(f, result, arg) mpf_##f(result, arg)
#define CALL2(f, result, arg1, arg2) \
  mpf_##f(result, arg1, arg2)
#define CALL3(f, result, arg1, arg2, arg3) \
  mpf_##f(result, arg1, arg2, arg3)
#define DEF1(f) \
  void mpf_##f(mpf_t res, mpf_t arg)
#define DEF2(f) \
  void mpf_##f(mpf_t res, mpf_t arg1, mpf_t arg2)
#define DEF3(f) \
  void mpf_##f(mpf_t res, mpf_t arg1, mpf_t arg2, mpf_t arg3)
#define GETD(v) \
  mpf_get_d(v)
#define RET
#endif

void execRealOp(IROp op_code, Real* result, ShadowValue** args);
DEF1(recip);
DEF2(recip_step);
DEF2(recip_sqrt_step);

#ifdef USE_MPFR
DEF1(2xm1);
DEF1(recp_exp);
DEF2(yl2x);
DEF2(yl2xp);
DEF2(scale);
DEF1(logb);
DEF1(lgamma2);
#else
DEF3(fma);
DEF3(fms);
DEF1(rec_sqrt);
DEF2(max);
DEF2(min);
#endif

#endif
