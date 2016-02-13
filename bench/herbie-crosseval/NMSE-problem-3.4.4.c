#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.4.4";

double f_if(float x) {
        float r5257 = 2.0;
        float r5258 = x;
        float r5259 = r5257 * r5258;
        float r5260 = exp(r5259);
        float r5261 = 1.0;
        float r5262 = r5260 - r5261;
        float r5263 = exp(r5258);
        float r5264 = r5263 - r5261;
        float r5265 = r5262 / r5264;
        float r5266 = sqrt(r5265);
        return r5266;
}

double f_id(double x) {
        double r5267 = 2.0;
        double r5268 = x;
        double r5269 = r5267 * r5268;
        double r5270 = exp(r5269);
        double r5271 = 1.0;
        double r5272 = r5270 - r5271;
        double r5273 = exp(r5268);
        double r5274 = r5273 - r5271;
        double r5275 = r5272 / r5274;
        double r5276 = sqrt(r5275);
        return r5276;
}


double f_of(float x) {
        float r5277 = 2.0;
        float r5278 = x;
        float r5279 = r5277 * r5278;
        float r5280 = exp(r5279);
        float r5281 = 1.0;
        float r5282 = r5280 - r5281;
        float r5283 = exp(r5278);
        float r5284 = r5283 - r5281;
        float r5285 = r5282 / r5284;
        float r5286 = sqrt(r5285);
        return r5286;
}

double f_od(double x) {
        double r5287 = 2.0;
        double r5288 = x;
        double r5289 = r5287 * r5288;
        double r5290 = exp(r5289);
        double r5291 = 1.0;
        double r5292 = r5290 - r5291;
        double r5293 = exp(r5288);
        double r5294 = r5293 - r5291;
        double r5295 = r5292 / r5294;
        double r5296 = sqrt(r5295);
        return r5296;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r5297, r5298, r5299, r5300, r5301, r5302, r5303, r5304, r5305, r5306;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5297, "2", 10, MPFR_RNDN);
        mpfr_init(r5298);
        mpfr_init(r5299);
        mpfr_init(r5300);
        mpfr_init_set_str(r5301, "1", 10, MPFR_RNDN);
        mpfr_init(r5302);
        mpfr_init(r5303);
        mpfr_init(r5304);
        mpfr_init(r5305);
        mpfr_init(r5306);
}

double f_im(double x) {
        ;
        mpfr_set_d(r5298, x, MPFR_RNDN);
        mpfr_mul(r5299, r5297, r5298, MPFR_RNDN);
        mpfr_exp(r5300, r5299, MPFR_RNDN);
        ;
        mpfr_sub(r5302, r5300, r5301, MPFR_RNDN);
        mpfr_exp(r5303, r5298, MPFR_RNDN);
        mpfr_sub(r5304, r5303, r5301, MPFR_RNDN);
        mpfr_div(r5305, r5302, r5304, MPFR_RNDN);
        mpfr_sqrt(r5306, r5305, MPFR_RNDN);
        return mpfr_get_d(r5306, MPFR_RNDN);
}

static mpfr_t r5307, r5308, r5309, r5310, r5311, r5312, r5313, r5314, r5315, r5316;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5307, "2", 10, MPFR_RNDN);
        mpfr_init(r5308);
        mpfr_init(r5309);
        mpfr_init(r5310);
        mpfr_init_set_str(r5311, "1", 10, MPFR_RNDN);
        mpfr_init(r5312);
        mpfr_init(r5313);
        mpfr_init(r5314);
        mpfr_init(r5315);
        mpfr_init(r5316);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r5308, x, MPFR_RNDN);
        mpfr_mul(r5309, r5307, r5308, MPFR_RNDN);
        mpfr_exp(r5310, r5309, MPFR_RNDN);
        ;
        mpfr_sub(r5312, r5310, r5311, MPFR_RNDN);
        mpfr_exp(r5313, r5308, MPFR_RNDN);
        mpfr_sub(r5314, r5313, r5311, MPFR_RNDN);
        mpfr_div(r5315, r5312, r5314, MPFR_RNDN);
        mpfr_sqrt(r5316, r5315, MPFR_RNDN);
        return mpfr_get_d(r5316, MPFR_RNDN);
}

static mpfr_t r5317, r5318, r5319, r5320, r5321, r5322, r5323, r5324, r5325, r5326;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5317, "2", 10, MPFR_RNDN);
        mpfr_init(r5318);
        mpfr_init(r5319);
        mpfr_init(r5320);
        mpfr_init_set_str(r5321, "1", 10, MPFR_RNDN);
        mpfr_init(r5322);
        mpfr_init(r5323);
        mpfr_init(r5324);
        mpfr_init(r5325);
        mpfr_init(r5326);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r5318, x, MPFR_RNDN);
        mpfr_mul(r5319, r5317, r5318, MPFR_RNDN);
        mpfr_exp(r5320, r5319, MPFR_RNDN);
        ;
        mpfr_sub(r5322, r5320, r5321, MPFR_RNDN);
        mpfr_exp(r5323, r5318, MPFR_RNDN);
        mpfr_sub(r5324, r5323, r5321, MPFR_RNDN);
        mpfr_div(r5325, r5322, r5324, MPFR_RNDN);
        mpfr_sqrt(r5326, r5325, MPFR_RNDN);
        return mpfr_get_d(r5326, MPFR_RNDN);
}

