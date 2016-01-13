#ifndef _HG_HIPREC_OPS
#define _HG_HIPREC_OPS

#include "mpfr.h"

// These are just a bunch of composite mpfr operations to mirror some
// of the weird operations supported by the floating point unit. These
// operations will pretty much never do the smart thing, at least for
// now, so they may be the source of weird things down the line.

int hiprec_yl2x(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round);
int hiprec_yl2xp1(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round);
int hiprec_scale(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round);

#endif
