#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.4.5";

double f_if(float x) {
        float r5327 = x;
        float r5328 = sin(r5327);
        float r5329 = r5327 - r5328;
        float r5330 = tan(r5327);
        float r5331 = r5327 - r5330;
        float r5332 = r5329 / r5331;
        return r5332;
}

double f_id(double x) {
        double r5333 = x;
        double r5334 = sin(r5333);
        double r5335 = r5333 - r5334;
        double r5336 = tan(r5333);
        double r5337 = r5333 - r5336;
        double r5338 = r5335 / r5337;
        return r5338;
}


double f_of(float x) {
        float r5339 = x;
        float r5340 = sin(r5339);
        float r5341 = r5339 - r5340;
        float r5342 = tan(r5339);
        float r5343 = r5339 - r5342;
        float r5344 = r5341 / r5343;
        return r5344;
}

double f_od(double x) {
        double r5345 = x;
        double r5346 = sin(r5345);
        double r5347 = r5345 - r5346;
        double r5348 = tan(r5345);
        double r5349 = r5345 - r5348;
        double r5350 = r5347 / r5349;
        return r5350;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r5351, r5352, r5353, r5354, r5355, r5356;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5351);
        mpfr_init(r5352);
        mpfr_init(r5353);
        mpfr_init(r5354);
        mpfr_init(r5355);
        mpfr_init(r5356);
}

double f_im(double x) {
        mpfr_set_d(r5351, x, MPFR_RNDN);
        mpfr_sin(r5352, r5351, MPFR_RNDN);
        mpfr_sub(r5353, r5351, r5352, MPFR_RNDN);
        mpfr_tan(r5354, r5351, MPFR_RNDN);
        mpfr_sub(r5355, r5351, r5354, MPFR_RNDN);
        mpfr_div(r5356, r5353, r5355, MPFR_RNDN);
        return mpfr_get_d(r5356, MPFR_RNDN);
}

static mpfr_t r5357, r5358, r5359, r5360, r5361, r5362;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5357);
        mpfr_init(r5358);
        mpfr_init(r5359);
        mpfr_init(r5360);
        mpfr_init(r5361);
        mpfr_init(r5362);
}

double f_fm(double x) {
        mpfr_set_d(r5357, x, MPFR_RNDN);
        mpfr_sin(r5358, r5357, MPFR_RNDN);
        mpfr_sub(r5359, r5357, r5358, MPFR_RNDN);
        mpfr_tan(r5360, r5357, MPFR_RNDN);
        mpfr_sub(r5361, r5357, r5360, MPFR_RNDN);
        mpfr_div(r5362, r5359, r5361, MPFR_RNDN);
        return mpfr_get_d(r5362, MPFR_RNDN);
}

static mpfr_t r5363, r5364, r5365, r5366, r5367, r5368;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5363);
        mpfr_init(r5364);
        mpfr_init(r5365);
        mpfr_init(r5366);
        mpfr_init(r5367);
        mpfr_init(r5368);
}

double f_dm(double x) {
        mpfr_set_d(r5363, x, MPFR_RNDN);
        mpfr_sin(r5364, r5363, MPFR_RNDN);
        mpfr_sub(r5365, r5363, r5364, MPFR_RNDN);
        mpfr_tan(r5366, r5363, MPFR_RNDN);
        mpfr_sub(r5367, r5363, r5366, MPFR_RNDN);
        mpfr_div(r5368, r5365, r5367, MPFR_RNDN);
        return mpfr_get_d(r5368, MPFR_RNDN);
}

