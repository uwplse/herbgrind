#include "hg_evaluate.h"
#include "hg_runtime.h"
#include "../include/hg_options.h"
#include "hg_op_tracker.h"

void evaluateOpError(ShadowValue* shadowVal, double actualVal,
                     Op_Info* opinfo){
  // We're going to do the log in mpfr since that way we don't have to
  // worry about pulling in the normal math library, which is
  // non-trivial in a valgrind tool. But, we can't get ulps from an
  // mpfr value, so we have to first bring both values into doubles,
  // get their ulps, move the ulps into MPFR, get the +1log (which is
  // the number of bits), then finally convert that to double.
  unsigned long long ulpsError;
  double bitsError, shadowValD;
  mpfr_t ulpsErrorM, bitsErrorM;

  shadowValD = mpfr_get_d(shadowVal->value, MPFR_RNDN);

  ulpsError = ulpd(shadowValD, actualVal);
  // To calculate bits error, we take the log2 of the ulps error +
  // 1. This means that 0 ulps (same value) has log2(1) = 0 bits of
  // error, 1 ulp (values that are as close as they can be but still
  // different) has log2(2) = 1 bit of error, and we scale
  // logarithmically from there.
  mpfr_init2(ulpsErrorM, precision);
  mpfr_set_ui(ulpsErrorM, (unsigned long int)(ulpsError + 1), MPFR_RNDN);
  mpfr_init2(bitsErrorM, 64);
  mpfr_log2(bitsErrorM, ulpsErrorM, MPFR_RNDN);
  bitsError = mpfr_get_d(bitsErrorM, MPFR_RNDN);

  // Update the persistent op record
  if (bitsError > opinfo->evalinfo.max_error){
    // This tests whether we didnt want to track it before, but do
    // now. If that's the case, we'll start tracking it.
    if (opinfo->evalinfo.max_error < error_threshold &&
        bitsError > error_threshold){
      startTrackingOp(opinfo);
    }
    // Update the max error, since the error of this operation
    // instance was greater than any error this operation has seen before.
    opinfo->evalinfo.max_error = bitsError;
  }
  opinfo->evalinfo.total_error += bitsError;
  opinfo->evalinfo.num_calls++;

  // For printing
  if (print_errors_long || print_errors){
    if (print_errors_long){
      char* shadowValstr;
      mpfr_exp_t shadowValexpt;

      shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, 0, shadowVal->value, MPFR_RNDN);

      VG_(printf)("The shadowed val is %se%ld, and the actual (computed) val is %f.\n",
                  shadowValstr, shadowValexpt, actualVal);
      mpfr_free_str(shadowValstr);
    }
    else if (print_errors){
      VG_(printf)("The shadowed val is %f, and the actual (computed) val is %f.\n",
                  shadowValD, actualVal);
    }
  
    VG_(printf)("The bits error of that operation was: %f.\n", bitsError);
    VG_(printf)("(Operation at %lX)\n", opinfo->debuginfo.op_addr);
    if (opinfo->debuginfo.src_filename != NULL)
      VG_(printf)("%s at %s:%u in %s\n",
                  opinfo->debuginfo.plain_opname,
                  opinfo->debuginfo.src_filename,
                  opinfo->debuginfo.src_line,
                  opinfo->debuginfo.fnname);
  }
}

void evaluateOpError_helper(ShadowValue* shadowVal, UWord* valbytes, LocType bytestype, int el_index, Op_Info* opinfo){
  switch(bytestype){
  case Lt_Float:
  case Lt_Floatx2:
  case Lt_Floatx4:
  case Lt_Floatx8:
    evaluateOpError(shadowVal, ((float*)valbytes)[el_index], opinfo);
    break;
  case Lt_Double:
  case Lt_Doublex2:
  case Lt_Doublex4:
    evaluateOpError(shadowVal, ((double*)valbytes)[el_index], opinfo);
    break;
  default:
    VG_(dmsg)("Hey, those are some big floats! We can't handle those floats!");
    break;
  }
}

unsigned long long ulpd(double x, double y) {
  if (x == 0) x = 0; // -0 == 0
  if (y == 0) y = 0; // -0 == 0

  if (x != x && y != y) return 0;
  if (x != x) return LLONG_MIN; // Maximum error
  if (y != y) return LLONG_MIN; // Maximum error

  long long xx = *((long long*) &x);
  xx = xx < 0 ? LLONG_MIN - xx : xx;

  long long yy = *((long long*) &y);
  yy = yy < 0 ? LLONG_MIN - yy : yy;

  return xx >= yy ? xx - yy : yy - xx;
}
