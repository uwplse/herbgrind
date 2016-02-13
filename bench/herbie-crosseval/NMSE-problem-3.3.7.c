#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.3.7";

double f_if(float x) {
        float r4303 = x;
        float r4304 = exp(r4303);
        float r4305 = 2.0;
        float r4306 = r4304 - r4305;
        float r4307 = -r4303;
        float r4308 = exp(r4307);
        float r4309 = r4306 + r4308;
        return r4309;
}

double f_id(double x) {
        double r4310 = x;
        double r4311 = exp(r4310);
        double r4312 = 2.0;
        double r4313 = r4311 - r4312;
        double r4314 = -r4310;
        double r4315 = exp(r4314);
        double r4316 = r4313 + r4315;
        return r4316;
}


double f_of(float x) {
        float r4317 = 4.0;
        float r4318 = x;
        float r4319 = 2.0;
        float r4320 = r4318 / r4319;
        float r4321 = sinh(r4320);
        float r4322 = r4321 * r4321;
        float r4323 = r4317 * r4322;
        return r4323;
}

double f_od(double x) {
        double r4324 = 4.0;
        double r4325 = x;
        double r4326 = 2.0;
        double r4327 = r4325 / r4326;
        double r4328 = sinh(r4327);
        double r4329 = r4328 * r4328;
        double r4330 = r4324 * r4329;
        return r4330;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4331, r4332, r4333, r4334, r4335, r4336, r4337;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4331);
        mpfr_init(r4332);
        mpfr_init_set_str(r4333, "2", 10, MPFR_RNDN);
        mpfr_init(r4334);
        mpfr_init(r4335);
        mpfr_init(r4336);
        mpfr_init(r4337);
}

double f_im(double x) {
        mpfr_set_d(r4331, x, MPFR_RNDN);
        mpfr_exp(r4332, r4331, MPFR_RNDN);
        ;
        mpfr_sub(r4334, r4332, r4333, MPFR_RNDN);
        mpfr_neg(r4335, r4331, MPFR_RNDN);
        mpfr_exp(r4336, r4335, MPFR_RNDN);
        mpfr_add(r4337, r4334, r4336, MPFR_RNDN);
        return mpfr_get_d(r4337, MPFR_RNDN);
}

static mpfr_t r4338, r4339, r4340, r4341, r4342, r4343, r4344;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4338, "4", 10, MPFR_RNDN);
        mpfr_init(r4339);
        mpfr_init_set_str(r4340, "2", 10, MPFR_RNDN);
        mpfr_init(r4341);
        mpfr_init(r4342);
        mpfr_init(r4343);
        mpfr_init(r4344);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r4339, x, MPFR_RNDN);
        ;
        mpfr_div(r4341, r4339, r4340, MPFR_RNDN);
        mpfr_sinh(r4342, r4341, MPFR_RNDN);
        mpfr_mul(r4343, r4342, r4342, MPFR_RNDN);
        mpfr_mul(r4344, r4338, r4343, MPFR_RNDN);
        return mpfr_get_d(r4344, MPFR_RNDN);
}

static mpfr_t r4345, r4346, r4347, r4348, r4349, r4350, r4351;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4345, "4", 10, MPFR_RNDN);
        mpfr_init(r4346);
        mpfr_init_set_str(r4347, "2", 10, MPFR_RNDN);
        mpfr_init(r4348);
        mpfr_init(r4349);
        mpfr_init(r4350);
        mpfr_init(r4351);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r4346, x, MPFR_RNDN);
        ;
        mpfr_div(r4348, r4346, r4347, MPFR_RNDN);
        mpfr_sinh(r4349, r4348, MPFR_RNDN);
        mpfr_mul(r4350, r4349, r4349, MPFR_RNDN);
        mpfr_mul(r4351, r4345, r4350, MPFR_RNDN);
        return mpfr_get_d(r4351, MPFR_RNDN);
}

