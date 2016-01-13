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
int hiprec_2xm1(mpfr_t result, mpfr_t arg, mpfr_rnd_t round);
int hiprec_recpexp(mpfr_t result, mpfr_t arg, mpfr_rnd_t round);

// Two argument
int hiprec_yl2x(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round);
int hiprec_yl2xp1(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round);
int hiprec_scale(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round);

// Three argument
int hiprec_fma(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_t arg3, mpfr_rnd_t round);
int hiprec_fms(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_t arg3, mpfr_rnd_t round);
#endif
