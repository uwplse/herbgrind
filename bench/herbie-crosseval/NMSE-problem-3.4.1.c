#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.4.1";

double f_if(float x) {
        float r5082 = 1.0;
        float r5083 = x;
        float r5084 = cos(r5083);
        float r5085 = r5082 - r5084;
        float r5086 = r5083 * r5083;
        float r5087 = r5085 / r5086;
        return r5087;
}

double f_id(double x) {
        double r5088 = 1.0;
        double r5089 = x;
        double r5090 = cos(r5089);
        double r5091 = r5088 - r5090;
        double r5092 = r5089 * r5089;
        double r5093 = r5091 / r5092;
        return r5093;
}


double f_of(float x) {
        float r5094 = 1.0;
        float r5095 = x;
        float r5096 = cos(r5095);
        float r5097 = r5094 - r5096;
        float r5098 = r5095 * r5095;
        float r5099 = r5097 / r5098;
        return r5099;
}

double f_od(double x) {
        double r5100 = 1.0;
        double r5101 = x;
        double r5102 = cos(r5101);
        double r5103 = r5100 - r5102;
        double r5104 = r5101 * r5101;
        double r5105 = r5103 / r5104;
        return r5105;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r5106, r5107, r5108, r5109, r5110, r5111;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5106, "1", 10, MPFR_RNDN);
        mpfr_init(r5107);
        mpfr_init(r5108);
        mpfr_init(r5109);
        mpfr_init(r5110);
        mpfr_init(r5111);
}

double f_im(double x) {
        ;
        mpfr_set_d(r5107, x, MPFR_RNDN);
        mpfr_cos(r5108, r5107, MPFR_RNDN);
        mpfr_sub(r5109, r5106, r5108, MPFR_RNDN);
        mpfr_mul(r5110, r5107, r5107, MPFR_RNDN);
        mpfr_div(r5111, r5109, r5110, MPFR_RNDN);
        return mpfr_get_d(r5111, MPFR_RNDN);
}

static mpfr_t r5112, r5113, r5114, r5115, r5116, r5117;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5112, "1", 10, MPFR_RNDN);
        mpfr_init(r5113);
        mpfr_init(r5114);
        mpfr_init(r5115);
        mpfr_init(r5116);
        mpfr_init(r5117);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r5113, x, MPFR_RNDN);
        mpfr_cos(r5114, r5113, MPFR_RNDN);
        mpfr_sub(r5115, r5112, r5114, MPFR_RNDN);
        mpfr_mul(r5116, r5113, r5113, MPFR_RNDN);
        mpfr_div(r5117, r5115, r5116, MPFR_RNDN);
        return mpfr_get_d(r5117, MPFR_RNDN);
}

static mpfr_t r5118, r5119, r5120, r5121, r5122, r5123;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5118, "1", 10, MPFR_RNDN);
        mpfr_init(r5119);
        mpfr_init(r5120);
        mpfr_init(r5121);
        mpfr_init(r5122);
        mpfr_init(r5123);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r5119, x, MPFR_RNDN);
        mpfr_cos(r5120, r5119, MPFR_RNDN);
        mpfr_sub(r5121, r5118, r5120, MPFR_RNDN);
        mpfr_mul(r5122, r5119, r5119, MPFR_RNDN);
        mpfr_div(r5123, r5121, r5122, MPFR_RNDN);
        return mpfr_get_d(r5123, MPFR_RNDN);
}

