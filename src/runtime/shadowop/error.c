/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie                error.c ---*/
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

#include "error.h"
#include "../value-shadowstate/shadowval.h"
#include "../../helper/runtime-util.h"

#include "mpfr.h"
#include "pub_tool_libcprint.h"
#include <math.h>

double updateError(ErrorAggregate* eagg,
                   Real realVal, double computedVal){
  if (no_reals) return 0.0;
  double shadowRounded = getDouble(realVal);
  ULong ulpsError = ulpd(shadowRounded, computedVal);

  double bitsError = log2(ulpsError + 1);
  if (bitsError > eagg->max_error){
    eagg->max_error = bitsError;
  }
  eagg->total_error += bitsError;
  eagg->num_evals += 1;


  // Debug printing code

  if (print_errors_long || print_errors){
    if (print_errors_long){
      VG_(printf)("The shadow value is ");
      printReal(realVal);
    } else {
      if (shadowRounded != shadowRounded){
        VG_(printf)("The rounded shadow value is NaN");
      } else {
        VG_(printf)("The rounded shadow value is ");
        ppFloat(shadowRounded);
      }
    }
    if (computedVal != computedVal){
      VG_(printf)(", but NaN was computed.\n");
    } else {
      VG_(printf)(", but ");
      ppFloat(computedVal);
      VG_(printf)(" was computed.\n");
    }
    VG_(printf)("%f bits error (%llu ulps)\n",
                bitsError, ulpsError);
  }
  return bitsError;
}

ULong ulpd(double x, double y){
  if (x == 0) x = 0; // -0 == 0
  if (y == 0) y = 0; // -0 == 0

  /* if (x != x && y != y) return 0; */
  if (x != x) return ULLONG_MAX - 1; // Maximum error
  if (y != y) return ULLONG_MAX - 1; // Maximum error

  long long xx = *((long long*) &x);
  xx = xx < 0 ? LLONG_MIN - xx : xx;

  long long yy = *((long long*) &y);
  yy = yy < 0 ? LLONG_MIN - yy : yy;

  return xx >= yy ? xx - yy : yy - xx;
}
