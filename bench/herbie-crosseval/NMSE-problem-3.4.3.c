#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.4.3";

double f_if(float eps) {
        float r5195 = 1.0;
        float r5196 = eps;
        float r5197 = r5195 - r5196;
        float r5198 = r5195 + r5196;
        float r5199 = r5197 / r5198;
        float r5200 = log(r5199);
        return r5200;
}

double f_id(double eps) {
        double r5201 = 1.0;
        double r5202 = eps;
        double r5203 = r5201 - r5202;
        double r5204 = r5201 + r5202;
        double r5205 = r5203 / r5204;
        double r5206 = log(r5205);
        return r5206;
}


double f_of(float eps) {
        float r5207 = -2.0;
        float r5208 = eps;
        float r5209 = 3.0;
        float r5210 = pow(r5208, r5209);
        float r5211 = r5210 / r5209;
        float r5212 = r5208 + r5211;
        float r5213 = 5.0;
        float r5214 = pow(r5208, r5213);
        float r5215 = r5214 / r5213;
        float r5216 = r5212 + r5215;
        float r5217 = r5207 * r5216;
        return r5217;
}

double f_od(double eps) {
        double r5218 = -2.0;
        double r5219 = eps;
        double r5220 = 3.0;
        double r5221 = pow(r5219, r5220);
        double r5222 = r5221 / r5220;
        double r5223 = r5219 + r5222;
        double r5224 = 5.0;
        double r5225 = pow(r5219, r5224);
        double r5226 = r5225 / r5224;
        double r5227 = r5223 + r5226;
        double r5228 = r5218 * r5227;
        return r5228;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r5229, r5230, r5231, r5232, r5233, r5234;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5229, "1", 10, MPFR_RNDN);
        mpfr_init(r5230);
        mpfr_init(r5231);
        mpfr_init(r5232);
        mpfr_init(r5233);
        mpfr_init(r5234);
}

double f_im(double eps) {
        ;
        mpfr_set_d(r5230, eps, MPFR_RNDN);
        mpfr_sub(r5231, r5229, r5230, MPFR_RNDN);
        mpfr_add(r5232, r5229, r5230, MPFR_RNDN);
        mpfr_div(r5233, r5231, r5232, MPFR_RNDN);
        mpfr_log(r5234, r5233, MPFR_RNDN);
        return mpfr_get_d(r5234, MPFR_RNDN);
}

static mpfr_t r5235, r5236, r5237, r5238, r5239, r5240, r5241, r5242, r5243, r5244, r5245;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5235, "-2", 10, MPFR_RNDN);
        mpfr_init(r5236);
        mpfr_init_set_str(r5237, "3", 10, MPFR_RNDN);
        mpfr_init(r5238);
        mpfr_init(r5239);
        mpfr_init(r5240);
        mpfr_init_set_str(r5241, "5", 10, MPFR_RNDN);
        mpfr_init(r5242);
        mpfr_init(r5243);
        mpfr_init(r5244);
        mpfr_init(r5245);
}

double f_fm(double eps) {
        ;
        mpfr_set_d(r5236, eps, MPFR_RNDN);
        ;
        mpfr_pow(r5238, r5236, r5237, MPFR_RNDN);
        mpfr_div(r5239, r5238, r5237, MPFR_RNDN);
        mpfr_add(r5240, r5236, r5239, MPFR_RNDN);
        ;
        mpfr_pow(r5242, r5236, r5241, MPFR_RNDN);
        mpfr_div(r5243, r5242, r5241, MPFR_RNDN);
        mpfr_add(r5244, r5240, r5243, MPFR_RNDN);
        mpfr_mul(r5245, r5235, r5244, MPFR_RNDN);
        return mpfr_get_d(r5245, MPFR_RNDN);
}

static mpfr_t r5246, r5247, r5248, r5249, r5250, r5251, r5252, r5253, r5254, r5255, r5256;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5246, "-2", 10, MPFR_RNDN);
        mpfr_init(r5247);
        mpfr_init_set_str(r5248, "3", 10, MPFR_RNDN);
        mpfr_init(r5249);
        mpfr_init(r5250);
        mpfr_init(r5251);
        mpfr_init_set_str(r5252, "5", 10, MPFR_RNDN);
        mpfr_init(r5253);
        mpfr_init(r5254);
        mpfr_init(r5255);
        mpfr_init(r5256);
}

double f_dm(double eps) {
        ;
        mpfr_set_d(r5247, eps, MPFR_RNDN);
        ;
        mpfr_pow(r5249, r5247, r5248, MPFR_RNDN);
        mpfr_div(r5250, r5249, r5248, MPFR_RNDN);
        mpfr_add(r5251, r5247, r5250, MPFR_RNDN);
        ;
        mpfr_pow(r5253, r5247, r5252, MPFR_RNDN);
        mpfr_div(r5254, r5253, r5252, MPFR_RNDN);
        mpfr_add(r5255, r5251, r5254, MPFR_RNDN);
        mpfr_mul(r5256, r5246, r5255, MPFR_RNDN);
        return mpfr_get_d(r5256, MPFR_RNDN);
}

