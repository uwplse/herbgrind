#include "hg_hiprec_ops.h"

int hiprec_2xm1(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round){
  mpfr_exp2(result, arg, round);
  return mpfr_sub_d(result, result, 1.0, round);
}
int hiprec_recpexp(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round){
  mpfr_neg(result, arg, round);
  return mpfr_exp(result, result, round);
}
int hiprec_recip(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round){
  return mpfr_d_div(result, 1.0, arg, round);
}
int hiprec_logb(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round){
  mpfr_set_ui(result, mpfr_get_exp(arg), round);
  return 0;
}
int hiprec_lgamma(mpfr_t result, mpfr_srcptr arg, mpfr_rnd_t round){
  int _x;
  return mpfr_lgamma(result, &_x, arg, round);
}

int hiprec_yl2x(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round){
  mpfr_log2(result, arg2, round);
  return mpfr_mul(result, arg1, result, round);
}
int hiprec_yl2xp1(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round){
  mpfr_add_d(result, arg2, 1.0, round);
  mpfr_log2(result, result, round);
  return mpfr_mul(result, arg1, result, round);
}
int hiprec_scale(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round){
  mpfr_trunc(result, arg2);
  mpfr_exp2(result, result, round);
  return mpfr_mul(result, arg1, result, round);
}
int hiprec_recipstep(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round){
  mpfr_mul(result, arg1, arg2, round);
  return mpfr_d_sub(result, 2.0, result, round);
}
int hiprec_rsqrtstep(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_rnd_t round){
  mpfr_mul(result, arg1, arg2, round);
  mpfr_d_sub(result, 3.0, result, round);
  return mpfr_div_d(result, result, 2.0, round);
}

int hiprec_fma(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_srcptr arg3, mpfr_rnd_t round){
  mpfr_mul(result, arg1, arg2, round);
  return mpfr_add(result, result, arg3, round);
}

int hiprec_fms(mpfr_t result, mpfr_srcptr arg1, mpfr_srcptr arg2, mpfr_srcptr arg3, mpfr_rnd_t round){
  mpfr_mul(result, arg1, arg2, round);
  return mpfr_sub(result, result, arg3, round);
}
