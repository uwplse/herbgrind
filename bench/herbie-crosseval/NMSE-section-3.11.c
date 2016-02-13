#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE section 3.11";

double f_if(float x) {
        float r4352 = x;
        float r4353 = exp(r4352);
        float r4354 = 1.0;
        float r4355 = r4353 - r4354;
        float r4356 = r4353 / r4355;
        return r4356;
}

double f_id(double x) {
        double r4357 = x;
        double r4358 = exp(r4357);
        double r4359 = 1.0;
        double r4360 = r4358 - r4359;
        double r4361 = r4358 / r4360;
        return r4361;
}


double f_of(float x) {
        float r4362 = 1.0;
        float r4363 = x;
        float r4364 = -r4363;
        float r4365 = exp(r4364);
        float r4366 = r4362 - r4365;
        float r4367 = r4362 / r4366;
        return r4367;
}

double f_od(double x) {
        double r4368 = 1.0;
        double r4369 = x;
        double r4370 = -r4369;
        double r4371 = exp(r4370);
        double r4372 = r4368 - r4371;
        double r4373 = r4368 / r4372;
        return r4373;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4374, r4375, r4376, r4377, r4378;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4374);
        mpfr_init(r4375);
        mpfr_init_set_str(r4376, "1", 10, MPFR_RNDN);
        mpfr_init(r4377);
        mpfr_init(r4378);
}

double f_im(double x) {
        mpfr_set_d(r4374, x, MPFR_RNDN);
        mpfr_exp(r4375, r4374, MPFR_RNDN);
        ;
        mpfr_sub(r4377, r4375, r4376, MPFR_RNDN);
        mpfr_div(r4378, r4375, r4377, MPFR_RNDN);
        return mpfr_get_d(r4378, MPFR_RNDN);
}

static mpfr_t r4379, r4380, r4381, r4382, r4383, r4384;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4379, "1", 10, MPFR_RNDN);
        mpfr_init(r4380);
        mpfr_init(r4381);
        mpfr_init(r4382);
        mpfr_init(r4383);
        mpfr_init(r4384);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r4380, x, MPFR_RNDN);
        mpfr_neg(r4381, r4380, MPFR_RNDN);
        mpfr_exp(r4382, r4381, MPFR_RNDN);
        mpfr_sub(r4383, r4379, r4382, MPFR_RNDN);
        mpfr_div(r4384, r4379, r4383, MPFR_RNDN);
        return mpfr_get_d(r4384, MPFR_RNDN);
}

static mpfr_t r4385, r4386, r4387, r4388, r4389, r4390;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4385, "1", 10, MPFR_RNDN);
        mpfr_init(r4386);
        mpfr_init(r4387);
        mpfr_init(r4388);
        mpfr_init(r4389);
        mpfr_init(r4390);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r4386, x, MPFR_RNDN);
        mpfr_neg(r4387, r4386, MPFR_RNDN);
        mpfr_exp(r4388, r4387, MPFR_RNDN);
        mpfr_sub(r4389, r4385, r4388, MPFR_RNDN);
        mpfr_div(r4390, r4385, r4389, MPFR_RNDN);
        return mpfr_get_d(r4390, MPFR_RNDN);
}

