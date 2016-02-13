#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.3.2";

double f_if(float x, float eps) {
        float r4055 = x;
        float r4056 = eps;
        float r4057 = r4055 + r4056;
        float r4058 = tan(r4057);
        float r4059 = tan(r4055);
        float r4060 = r4058 - r4059;
        return r4060;
}

double f_id(double x, double eps) {
        double r4061 = x;
        double r4062 = eps;
        double r4063 = r4061 + r4062;
        double r4064 = tan(r4063);
        double r4065 = tan(r4061);
        double r4066 = r4064 - r4065;
        return r4066;
}


double f_of(float x, float eps) {
        float r4067 = eps;
        float r4068 = sin(r4067);
        float r4069 = x;
        float r4070 = cos(r4069);
        float r4071 = r4069 + r4067;
        float r4072 = cos(r4071);
        float r4073 = r4070 * r4072;
        float r4074 = r4068 / r4073;
        return r4074;
}

double f_od(double x, double eps) {
        double r4075 = eps;
        double r4076 = sin(r4075);
        double r4077 = x;
        double r4078 = cos(r4077);
        double r4079 = r4077 + r4075;
        double r4080 = cos(r4079);
        double r4081 = r4078 * r4080;
        double r4082 = r4076 / r4081;
        return r4082;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4083, r4084, r4085, r4086, r4087, r4088;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4083);
        mpfr_init(r4084);
        mpfr_init(r4085);
        mpfr_init(r4086);
        mpfr_init(r4087);
        mpfr_init(r4088);
}

double f_im(double x, double eps) {
        mpfr_set_d(r4083, x, MPFR_RNDN);
        mpfr_set_d(r4084, eps, MPFR_RNDN);
        mpfr_add(r4085, r4083, r4084, MPFR_RNDN);
        mpfr_tan(r4086, r4085, MPFR_RNDN);
        mpfr_tan(r4087, r4083, MPFR_RNDN);
        mpfr_sub(r4088, r4086, r4087, MPFR_RNDN);
        return mpfr_get_d(r4088, MPFR_RNDN);
}

static mpfr_t r4089, r4090, r4091, r4092, r4093, r4094, r4095, r4096;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4089);
        mpfr_init(r4090);
        mpfr_init(r4091);
        mpfr_init(r4092);
        mpfr_init(r4093);
        mpfr_init(r4094);
        mpfr_init(r4095);
        mpfr_init(r4096);
}

double f_fm(double x, double eps) {
        mpfr_set_d(r4089, eps, MPFR_RNDN);
        mpfr_sin(r4090, r4089, MPFR_RNDN);
        mpfr_set_d(r4091, x, MPFR_RNDN);
        mpfr_cos(r4092, r4091, MPFR_RNDN);
        mpfr_add(r4093, r4091, r4089, MPFR_RNDN);
        mpfr_cos(r4094, r4093, MPFR_RNDN);
        mpfr_mul(r4095, r4092, r4094, MPFR_RNDN);
        mpfr_div(r4096, r4090, r4095, MPFR_RNDN);
        return mpfr_get_d(r4096, MPFR_RNDN);
}

static mpfr_t r4097, r4098, r4099, r4100, r4101, r4102, r4103, r4104;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4097);
        mpfr_init(r4098);
        mpfr_init(r4099);
        mpfr_init(r4100);
        mpfr_init(r4101);
        mpfr_init(r4102);
        mpfr_init(r4103);
        mpfr_init(r4104);
}

double f_dm(double x, double eps) {
        mpfr_set_d(r4097, eps, MPFR_RNDN);
        mpfr_sin(r4098, r4097, MPFR_RNDN);
        mpfr_set_d(r4099, x, MPFR_RNDN);
        mpfr_cos(r4100, r4099, MPFR_RNDN);
        mpfr_add(r4101, r4099, r4097, MPFR_RNDN);
        mpfr_cos(r4102, r4101, MPFR_RNDN);
        mpfr_mul(r4103, r4100, r4102, MPFR_RNDN);
        mpfr_div(r4104, r4098, r4103, MPFR_RNDN);
        return mpfr_get_d(r4104, MPFR_RNDN);
}

