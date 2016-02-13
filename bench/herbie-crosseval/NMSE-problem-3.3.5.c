#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.3.5";

double f_if(float x, float eps) {
        float r4219 = x;
        float r4220 = eps;
        float r4221 = r4219 + r4220;
        float r4222 = cos(r4221);
        float r4223 = cos(r4219);
        float r4224 = r4222 - r4223;
        return r4224;
}

double f_id(double x, double eps) {
        double r4225 = x;
        double r4226 = eps;
        double r4227 = r4225 + r4226;
        double r4228 = cos(r4227);
        double r4229 = cos(r4225);
        double r4230 = r4228 - r4229;
        return r4230;
}


double f_of(float x, float eps) {
        float r4231 = x;
        float r4232 = eps;
        float r4233 = r4231 + r4232;
        float r4234 = cos(r4233);
        float r4235 = cos(r4231);
        float r4236 = r4234 - r4235;
        return r4236;
}

double f_od(double x, double eps) {
        double r4237 = x;
        double r4238 = eps;
        double r4239 = r4237 + r4238;
        double r4240 = cos(r4239);
        double r4241 = cos(r4237);
        double r4242 = r4240 - r4241;
        return r4242;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4243, r4244, r4245, r4246, r4247, r4248;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4243);
        mpfr_init(r4244);
        mpfr_init(r4245);
        mpfr_init(r4246);
        mpfr_init(r4247);
        mpfr_init(r4248);
}

double f_im(double x, double eps) {
        mpfr_set_d(r4243, x, MPFR_RNDN);
        mpfr_set_d(r4244, eps, MPFR_RNDN);
        mpfr_add(r4245, r4243, r4244, MPFR_RNDN);
        mpfr_cos(r4246, r4245, MPFR_RNDN);
        mpfr_cos(r4247, r4243, MPFR_RNDN);
        mpfr_sub(r4248, r4246, r4247, MPFR_RNDN);
        return mpfr_get_d(r4248, MPFR_RNDN);
}

static mpfr_t r4249, r4250, r4251, r4252, r4253, r4254;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4249);
        mpfr_init(r4250);
        mpfr_init(r4251);
        mpfr_init(r4252);
        mpfr_init(r4253);
        mpfr_init(r4254);
}

double f_fm(double x, double eps) {
        mpfr_set_d(r4249, x, MPFR_RNDN);
        mpfr_set_d(r4250, eps, MPFR_RNDN);
        mpfr_add(r4251, r4249, r4250, MPFR_RNDN);
        mpfr_cos(r4252, r4251, MPFR_RNDN);
        mpfr_cos(r4253, r4249, MPFR_RNDN);
        mpfr_sub(r4254, r4252, r4253, MPFR_RNDN);
        return mpfr_get_d(r4254, MPFR_RNDN);
}

static mpfr_t r4255, r4256, r4257, r4258, r4259, r4260;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4255);
        mpfr_init(r4256);
        mpfr_init(r4257);
        mpfr_init(r4258);
        mpfr_init(r4259);
        mpfr_init(r4260);
}

double f_dm(double x, double eps) {
        mpfr_set_d(r4255, x, MPFR_RNDN);
        mpfr_set_d(r4256, eps, MPFR_RNDN);
        mpfr_add(r4257, r4255, r4256, MPFR_RNDN);
        mpfr_cos(r4258, r4257, MPFR_RNDN);
        mpfr_cos(r4259, r4255, MPFR_RNDN);
        mpfr_sub(r4260, r4258, r4259, MPFR_RNDN);
        return mpfr_get_d(r4260, MPFR_RNDN);
}

