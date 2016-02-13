#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.3.3";

double f_if(float x) {
        float r4105 = 1.0;
        float r4106 = x;
        float r4107 = r4106 + r4105;
        float r4108 = r4105 / r4107;
        float r4109 = 2.0;
        float r4110 = r4109 / r4106;
        float r4111 = r4108 - r4110;
        float r4112 = r4106 - r4105;
        float r4113 = r4105 / r4112;
        float r4114 = r4111 + r4113;
        return r4114;
}

double f_id(double x) {
        double r4115 = 1.0;
        double r4116 = x;
        double r4117 = r4116 + r4115;
        double r4118 = r4115 / r4117;
        double r4119 = 2.0;
        double r4120 = r4119 / r4116;
        double r4121 = r4118 - r4120;
        double r4122 = r4116 - r4115;
        double r4123 = r4115 / r4122;
        double r4124 = r4121 + r4123;
        return r4124;
}


double f_of(float x) {
        float r4125 = 2.0;
        float r4126 = x;
        float r4127 = r4126 * r4126;
        float r4128 = 1.0;
        float r4129 = r4127 - r4128;
        float r4130 = r4126 * r4129;
        float r4131 = r4125 / r4130;
        return r4131;
}

double f_od(double x) {
        double r4132 = 2.0;
        double r4133 = x;
        double r4134 = r4133 * r4133;
        double r4135 = 1.0;
        double r4136 = r4134 - r4135;
        double r4137 = r4133 * r4136;
        double r4138 = r4132 / r4137;
        return r4138;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4139, r4140, r4141, r4142, r4143, r4144, r4145, r4146, r4147, r4148;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4139, "1", 10, MPFR_RNDN);
        mpfr_init(r4140);
        mpfr_init(r4141);
        mpfr_init(r4142);
        mpfr_init_set_str(r4143, "2", 10, MPFR_RNDN);
        mpfr_init(r4144);
        mpfr_init(r4145);
        mpfr_init(r4146);
        mpfr_init(r4147);
        mpfr_init(r4148);
}

double f_im(double x) {
        ;
        mpfr_set_d(r4140, x, MPFR_RNDN);
        mpfr_add(r4141, r4140, r4139, MPFR_RNDN);
        mpfr_div(r4142, r4139, r4141, MPFR_RNDN);
        ;
        mpfr_div(r4144, r4143, r4140, MPFR_RNDN);
        mpfr_sub(r4145, r4142, r4144, MPFR_RNDN);
        mpfr_sub(r4146, r4140, r4139, MPFR_RNDN);
        mpfr_div(r4147, r4139, r4146, MPFR_RNDN);
        mpfr_add(r4148, r4145, r4147, MPFR_RNDN);
        return mpfr_get_d(r4148, MPFR_RNDN);
}

static mpfr_t r4149, r4150, r4151, r4152, r4153, r4154, r4155;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4149, "2", 10, MPFR_RNDN);
        mpfr_init(r4150);
        mpfr_init(r4151);
        mpfr_init_set_str(r4152, "1", 10, MPFR_RNDN);
        mpfr_init(r4153);
        mpfr_init(r4154);
        mpfr_init(r4155);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r4150, x, MPFR_RNDN);
        mpfr_mul(r4151, r4150, r4150, MPFR_RNDN);
        ;
        mpfr_sub(r4153, r4151, r4152, MPFR_RNDN);
        mpfr_mul(r4154, r4150, r4153, MPFR_RNDN);
        mpfr_div(r4155, r4149, r4154, MPFR_RNDN);
        return mpfr_get_d(r4155, MPFR_RNDN);
}

static mpfr_t r4156, r4157, r4158, r4159, r4160, r4161, r4162;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4156, "2", 10, MPFR_RNDN);
        mpfr_init(r4157);
        mpfr_init(r4158);
        mpfr_init_set_str(r4159, "1", 10, MPFR_RNDN);
        mpfr_init(r4160);
        mpfr_init(r4161);
        mpfr_init(r4162);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r4157, x, MPFR_RNDN);
        mpfr_mul(r4158, r4157, r4157, MPFR_RNDN);
        ;
        mpfr_sub(r4160, r4158, r4159, MPFR_RNDN);
        mpfr_mul(r4161, r4157, r4160, MPFR_RNDN);
        mpfr_div(r4162, r4156, r4161, MPFR_RNDN);
        return mpfr_get_d(r4162, MPFR_RNDN);
}

