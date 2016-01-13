#include "hg_hiprec_ops.h"

int hiprec_yl2x(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round){
  mpfr_log2(result, arg2, round);
  return mpfr_mul(result, arg1, result, round);
}
int hiprec_yl2xp1(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round){
  mpfr_add_d(result, arg2, 1.0, round);
  mpfr_log2(result, result, round);
  return mpfr_mul(result, arg1, result, round);
}
int hiprec_scale(mpfr_t result, mpfr_t arg1, mpfr_t arg2, mpfr_rnd_t round){
  mpfr_trunc(result, arg2);
  mpfr_exp2(result, result, round);
  return mpfr_mul(result, arg1, result, round);
}
