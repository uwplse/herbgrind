
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie        hg_hiprec_ops.h ---*/
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

#ifndef _HG_HIPREC_OPS
#define _HG_HIPREC_OPS

#include "mpfr.h"

// These are just a bunch of composite mpfr operations to mirror some
// of the weird operations supported by the floating point unit. These
// operations will pretty much never do the smart thing, at least for
// now, so they may be the source of weird things down the line. mpfr
// operations return some sort of error code I think, here we're just
// returning the code from the last operation, and ignoring the
// others. There's no way that could possibly go wrong, right? right
// guys?

// One argument
int hiprec_2xm1(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round);
int hiprec_recpexp(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round);
int hiprec_recip(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round);
int hiprec_logb(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round);
int hiprec_lgamma(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round);

// Two argument
int hiprec_yl2x(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round);
int hiprec_yl2xp1(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round);
int hiprec_scale(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round);
int hiprec_recipstep(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round);
int hiprec_rsqrtstep(mpfr_t result, mpfr_srcptr arg, mpfr_srcptr arg2, mpfr_rnd_t round);

// Three argument
int hiprec_fma(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_srcptr arg3, mpfr_rnd_t round);
int hiprec_fms(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_srcptr arg3, mpfr_rnd_t round);
#endif
