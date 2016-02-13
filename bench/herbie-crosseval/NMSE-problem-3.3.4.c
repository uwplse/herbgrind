#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.3.4";

double f_if(float x) {
        float r4163 = x;
        float r4164 = 1.0;
        float r4165 = r4163 + r4164;
        float r4166 = 3.0;
        float r4167 = r4164 / r4166;
        float r4168 = pow(r4165, r4167);
        float r4169 = pow(r4163, r4167);
        float r4170 = r4168 - r4169;
        return r4170;
}

double f_id(double x) {
        double r4171 = x;
        double r4172 = 1.0;
        double r4173 = r4171 + r4172;
        double r4174 = 3.0;
        double r4175 = r4172 / r4174;
        double r4176 = pow(r4173, r4175);
        double r4177 = pow(r4171, r4175);
        double r4178 = r4176 - r4177;
        return r4178;
}


double f_of(float x) {
        float r4179 = x;
        float r4180 = 1.0;
        float r4181 = r4179 + r4180;
        float r4182 = 3.0;
        float r4183 = r4180 / r4182;
        float r4184 = pow(r4181, r4183);
        float r4185 = pow(r4179, r4183);
        float r4186 = r4184 - r4185;
        return r4186;
}

double f_od(double x) {
        double r4187 = x;
        double r4188 = 1.0;
        double r4189 = r4187 + r4188;
        double r4190 = 3.0;
        double r4191 = r4188 / r4190;
        double r4192 = pow(r4189, r4191);
        double r4193 = pow(r4187, r4191);
        double r4194 = r4192 - r4193;
        return r4194;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4195, r4196, r4197, r4198, r4199, r4200, r4201, r4202;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4195);
        mpfr_init_set_str(r4196, "1", 10, MPFR_RNDN);
        mpfr_init(r4197);
        mpfr_init_set_str(r4198, "3", 10, MPFR_RNDN);
        mpfr_init(r4199);
        mpfr_init(r4200);
        mpfr_init(r4201);
        mpfr_init(r4202);
}

double f_im(double x) {
        mpfr_set_d(r4195, x, MPFR_RNDN);
        ;
        mpfr_add(r4197, r4195, r4196, MPFR_RNDN);
        ;
        mpfr_div(r4199, r4196, r4198, MPFR_RNDN);
        mpfr_pow(r4200, r4197, r4199, MPFR_RNDN);
        mpfr_pow(r4201, r4195, r4199, MPFR_RNDN);
        mpfr_sub(r4202, r4200, r4201, MPFR_RNDN);
        return mpfr_get_d(r4202, MPFR_RNDN);
}

static mpfr_t r4203, r4204, r4205, r4206, r4207, r4208, r4209, r4210;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4203);
        mpfr_init_set_str(r4204, "1", 10, MPFR_RNDN);
        mpfr_init(r4205);
        mpfr_init_set_str(r4206, "3", 10, MPFR_RNDN);
        mpfr_init(r4207);
        mpfr_init(r4208);
        mpfr_init(r4209);
        mpfr_init(r4210);
}

double f_fm(double x) {
        mpfr_set_d(r4203, x, MPFR_RNDN);
        ;
        mpfr_add(r4205, r4203, r4204, MPFR_RNDN);
        ;
        mpfr_div(r4207, r4204, r4206, MPFR_RNDN);
        mpfr_pow(r4208, r4205, r4207, MPFR_RNDN);
        mpfr_pow(r4209, r4203, r4207, MPFR_RNDN);
        mpfr_sub(r4210, r4208, r4209, MPFR_RNDN);
        return mpfr_get_d(r4210, MPFR_RNDN);
}

static mpfr_t r4211, r4212, r4213, r4214, r4215, r4216, r4217, r4218;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4211);
        mpfr_init_set_str(r4212, "1", 10, MPFR_RNDN);
        mpfr_init(r4213);
        mpfr_init_set_str(r4214, "3", 10, MPFR_RNDN);
        mpfr_init(r4215);
        mpfr_init(r4216);
        mpfr_init(r4217);
        mpfr_init(r4218);
}

double f_dm(double x) {
        mpfr_set_d(r4211, x, MPFR_RNDN);
        ;
        mpfr_add(r4213, r4211, r4212, MPFR_RNDN);
        ;
        mpfr_div(r4215, r4212, r4214, MPFR_RNDN);
        mpfr_pow(r4216, r4213, r4215, MPFR_RNDN);
        mpfr_pow(r4217, r4211, r4215, MPFR_RNDN);
        mpfr_sub(r4218, r4216, r4217, MPFR_RNDN);
        return mpfr_get_d(r4218, MPFR_RNDN);
}

