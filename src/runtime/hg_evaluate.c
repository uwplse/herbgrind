
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          hg_evaluate.c ---*/
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

#include "hg_evaluate.h"
#include "hg_runtime.h"
#include "../types/hg_stemtea.h"
#include "../include/hg_options.h"
#include "hg_op_tracker.h"
#include <math.h>

#include "pub_tool_libcassert.h"

void evaluateOpError(ShadowValue* shadowVal, double actualVal,
                     Op_Info* opinfo, double localResult,
                     Bool force){
  // We're going to do the log in mpfr since that way we don't have to
  // worry about pulling in the normal math library, which is
  // non-trivial in a valgrind tool. But, we can't get ulps from an
  // mpfr value, so we have to first bring both values into doubles,
  // get their ulps, move the ulps into MPFR, get the +1log (which is
  // the number of bits), then finally convert that to double.
  unsigned long long ulpsError, ulpsLocal;
  double bitsError, bitsLocal, shadowValD;
  mpfr_t ulpsErrorM, bitsErrorM, ulpsLocalM, bitsLocalM;

  if (!running) return;

  shadowValD = mpfr_get_d(shadowVal->value, MPFR_RNDN);

  ulpsError = ulpd(shadowValD, actualVal);
  ulpsLocal = ulpd(shadowValD, localResult);
  // To calculate bits error, we take the log2 of the ulps error +
  // 1. This means that 0 ulps (same value) has log2(1) = 0 bits of
  // error, 1 ulp (values that are as close as they can be but still
  // different) has log2(2) = 1 bit of error, and we scale
  // logarithmically from there.
  mpfr_inits2(precision, ulpsErrorM, ulpsLocalM, NULL);
  mpfr_set_ui(ulpsErrorM, (unsigned long int)(ulpsError + 1), MPFR_RNDN);
  mpfr_set_ui(ulpsLocalM, (unsigned long int)(ulpsLocal + 1), MPFR_RNDN);
  mpfr_inits2(64, bitsErrorM, bitsLocalM, NULL);
  mpfr_log2(bitsErrorM, ulpsErrorM, MPFR_RNDN);
  mpfr_log2(bitsLocalM, ulpsLocalM, MPFR_RNDN);
  bitsError = mpfr_get_d(bitsErrorM, MPFR_RNDN);
  bitsLocal = mpfr_get_d(bitsLocalM, MPFR_RNDN);

  mpfr_clears(ulpsErrorM, bitsErrorM, ulpsLocalM, bitsLocalM, NULL);

  // Update the persistent op record
  if (bitsError > opinfo->evalinfo.max_error){
    // Update the max error, since the error of this operation
    // instance was greater than any error this operation has seen before.
    opinfo->evalinfo.max_error = bitsError;
  }
  if (bitsLocal > opinfo->evalinfo.max_local){
    // This tests whether we didnt want to track it before, but do
    // now. If that's the case, we'll start tracking it.
    if (opinfo->evalinfo.max_local < error_threshold &&
        bitsLocal >= error_threshold){
      startTrackingOp(opinfo);
    }
    opinfo->evalinfo.max_local = bitsLocal;
  } else if (force && !isTrackingOp(opinfo)){
    startTrackingOp(opinfo);
  }
  if (report_exprs && (bitsLocal >= error_threshold || force)){
    // If the opfinfo doesn't have an tea assigned yet, give it a strict
    // translation of the tea assigned to this shadow value. If it does,
    // generalize the tea sufficiently to match the tea of the shadow
    // val.
    updateTea(opinfo, shadowVal->stem);
  }
  opinfo->evalinfo.total_error += bitsError;
  opinfo->evalinfo.total_local += bitsLocal;
  opinfo->evalinfo.num_calls++;

  // For printing
  if (print_errors_long || print_errors){
    if (print_errors_long){
      char* shadowValstr;
      mpfr_exp_t shadowValexpt;

      shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len, shadowVal->value, MPFR_RNDN);

      VG_(printf)("The shadowed val is %se%ld, and the actual (computed) val is %f.\n",
                  shadowValstr, shadowValexpt, actualVal);
      mpfr_free_str(shadowValstr);
    }
    else if (print_errors){
      VG_(printf)("The shadowed val is %f, and the actual (computed) val is %f.\n",
                  shadowValD, actualVal);
    }
  
    VG_(printf)("The bits error of that operation was: %f (%llu ulps).\n", bitsError, ulpsError);
    VG_(printf)("(Operation at %lX)\n", opinfo->debuginfo.op_addr);
    if (opinfo->debuginfo.src_filename != NULL)
      VG_(printf)("%s at %s:%u in %s\n",
                  opinfo->debuginfo.plain_opname,
                  opinfo->debuginfo.src_filename,
                  opinfo->debuginfo.src_line,
                  opinfo->debuginfo.fnname);
  }
}

void evaluateOpError_helper(ShadowValue* shadowVal, LocType bytestype, int el_index, Op_Info* opinfo, double localResult){
  switch(bytestype){
  case Lt_Float:
  case Lt_Floatx2:
  case Lt_Floatx4:
  case Lt_Floatx8:
    evaluateOpError(shadowVal, ((float*)(opinfo->dest_value))[el_index], opinfo, localResult, False);
    break;
  case Lt_Double:
  case Lt_Doublex2:
  case Lt_Doublex4:
    evaluateOpError(shadowVal, ((double*)(opinfo->dest_value))[el_index], opinfo, localResult, False);
    break;
  default:
    VG_(dmsg)("Hey, those are some big floats! We can't handle those floats!");
    tl_assert(0);
    break;
  }
}

unsigned long long ulpd(double x, double y) {
  if (x == 0) x = 0; // -0 == 0
  if (y == 0) y = 0; // -0 == 0

  if (x != x && y != y) return 0;
  if (x != x) return ULLONG_MAX - 1; // Maximum error
  if (y != y) return ULLONG_MAX - 1; // Maximum error

  long long xx = *((long long*) &x);
  xx = xx < 0 ? LLONG_MIN - xx : xx;

  long long yy = *((long long*) &y);
  yy = yy < 0 ? LLONG_MIN - yy : yy;

  return xx >= yy ? xx - yy : yy - xx;
}

void updateRegimes(double** regimes_data, SizeT arity, ...){
  va_list args;
  va_start(args, arity);
  for (int i = 0; i < arity; ++i){
    double coord = va_arg(args, double);
    if (coord != coord || coord == INFINITY || coord == -INFINITY){
      continue;
    }
    unsigned long long bucket_size = (ULLONG_MAX - 1) / max_num_regimes;
    int bucket = (*((long long*) &coord)) / bucket_size;
    if (regimes_data[i][bucket] != regimes_data[i][bucket]){
      regimes_data[i][bucket] = coord;
    }
    if (regimes_data[i][bucket + 1] != regimes_data[i][bucket + 1]){
      regimes_data[i][bucket + 1] = coord;
    }
  }
  va_end(args);
}
