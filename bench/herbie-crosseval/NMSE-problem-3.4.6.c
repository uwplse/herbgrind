#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.4.6";

double f_if(float x, float n) {
        float r5369 = x;
        float r5370 = 1.0;
        float r5371 = r5369 + r5370;
        float r5372 = n;
        float r5373 = r5370 / r5372;
        float r5374 = pow(r5371, r5373);
        float r5375 = pow(r5369, r5373);
        float r5376 = r5374 - r5375;
        return r5376;
}

double f_id(double x, double n) {
        double r5377 = x;
        double r5378 = 1.0;
        double r5379 = r5377 + r5378;
        double r5380 = n;
        double r5381 = r5378 / r5380;
        double r5382 = pow(r5379, r5381);
        double r5383 = pow(r5377, r5381);
        double r5384 = r5382 - r5383;
        return r5384;
}


double f_of(float x, float n) {
        float r5385 = x;
        float r5386 = 1.0;
        float r5387 = r5385 + r5386;
        float r5388 = n;
        float r5389 = r5386 / r5388;
        float r5390 = pow(r5387, r5389);
        float r5391 = pow(r5385, r5389);
        float r5392 = r5390 - r5391;
        return r5392;
}

double f_od(double x, double n) {
        double r5393 = x;
        double r5394 = 1.0;
        double r5395 = r5393 + r5394;
        double r5396 = n;
        double r5397 = r5394 / r5396;
        double r5398 = pow(r5395, r5397);
        double r5399 = pow(r5393, r5397);
        double r5400 = r5398 - r5399;
        return r5400;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r5401, r5402, r5403, r5404, r5405, r5406, r5407, r5408;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5401);
        mpfr_init_set_str(r5402, "1", 10, MPFR_RNDN);
        mpfr_init(r5403);
        mpfr_init(r5404);
        mpfr_init(r5405);
        mpfr_init(r5406);
        mpfr_init(r5407);
        mpfr_init(r5408);
}

double f_im(double x, double n) {
        mpfr_set_d(r5401, x, MPFR_RNDN);
        ;
        mpfr_add(r5403, r5401, r5402, MPFR_RNDN);
        mpfr_set_d(r5404, n, MPFR_RNDN);
        mpfr_div(r5405, r5402, r5404, MPFR_RNDN);
        mpfr_pow(r5406, r5403, r5405, MPFR_RNDN);
        mpfr_pow(r5407, r5401, r5405, MPFR_RNDN);
        mpfr_sub(r5408, r5406, r5407, MPFR_RNDN);
        return mpfr_get_d(r5408, MPFR_RNDN);
}

static mpfr_t r5409, r5410, r5411, r5412, r5413, r5414, r5415, r5416;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5409);
        mpfr_init_set_str(r5410, "1", 10, MPFR_RNDN);
        mpfr_init(r5411);
        mpfr_init(r5412);
        mpfr_init(r5413);
        mpfr_init(r5414);
        mpfr_init(r5415);
        mpfr_init(r5416);
}

double f_fm(double x, double n) {
        mpfr_set_d(r5409, x, MPFR_RNDN);
        ;
        mpfr_add(r5411, r5409, r5410, MPFR_RNDN);
        mpfr_set_d(r5412, n, MPFR_RNDN);
        mpfr_div(r5413, r5410, r5412, MPFR_RNDN);
        mpfr_pow(r5414, r5411, r5413, MPFR_RNDN);
        mpfr_pow(r5415, r5409, r5413, MPFR_RNDN);
        mpfr_sub(r5416, r5414, r5415, MPFR_RNDN);
        return mpfr_get_d(r5416, MPFR_RNDN);
}

static mpfr_t r5417, r5418, r5419, r5420, r5421, r5422, r5423, r5424;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5417);
        mpfr_init_set_str(r5418, "1", 10, MPFR_RNDN);
        mpfr_init(r5419);
        mpfr_init(r5420);
        mpfr_init(r5421);
        mpfr_init(r5422);
        mpfr_init(r5423);
        mpfr_init(r5424);
}

double f_dm(double x, double n) {
        mpfr_set_d(r5417, x, MPFR_RNDN);
        ;
        mpfr_add(r5419, r5417, r5418, MPFR_RNDN);
        mpfr_set_d(r5420, n, MPFR_RNDN);
        mpfr_div(r5421, r5418, r5420, MPFR_RNDN);
        mpfr_pow(r5422, r5419, r5421, MPFR_RNDN);
        mpfr_pow(r5423, r5417, r5421, MPFR_RNDN);
        mpfr_sub(r5424, r5422, r5423, MPFR_RNDN);
        return mpfr_get_d(r5424, MPFR_RNDN);
}

