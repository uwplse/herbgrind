#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.4";

double f_if(float x) {
        float r3873 = 1.0;
        float r3874 = x;
        float r3875 = cos(r3874);
        float r3876 = r3873 - r3875;
        float r3877 = sin(r3874);
        float r3878 = r3876 / r3877;
        return r3878;
}

double f_id(double x) {
        double r3879 = 1.0;
        double r3880 = x;
        double r3881 = cos(r3880);
        double r3882 = r3879 - r3881;
        double r3883 = sin(r3880);
        double r3884 = r3882 / r3883;
        return r3884;
}


double f_of(float x) {
        float r3885 = x;
        float r3886 = 2.0;
        float r3887 = r3885 / r3886;
        float r3888 = tan(r3887);
        return r3888;
}

double f_od(double x) {
        double r3889 = x;
        double r3890 = 2.0;
        double r3891 = r3889 / r3890;
        double r3892 = tan(r3891);
        return r3892;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r3893, r3894, r3895, r3896, r3897, r3898;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3893, "1", 10, MPFR_RNDN);
        mpfr_init(r3894);
        mpfr_init(r3895);
        mpfr_init(r3896);
        mpfr_init(r3897);
        mpfr_init(r3898);
}

double f_im(double x) {
        ;
        mpfr_set_d(r3894, x, MPFR_RNDN);
        mpfr_cos(r3895, r3894, MPFR_RNDN);
        mpfr_sub(r3896, r3893, r3895, MPFR_RNDN);
        mpfr_sin(r3897, r3894, MPFR_RNDN);
        mpfr_div(r3898, r3896, r3897, MPFR_RNDN);
        return mpfr_get_d(r3898, MPFR_RNDN);
}

static mpfr_t r3899, r3900, r3901, r3902;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r3899);
        mpfr_init_set_str(r3900, "2", 10, MPFR_RNDN);
        mpfr_init(r3901);
        mpfr_init(r3902);
}

double f_fm(double x) {
        mpfr_set_d(r3899, x, MPFR_RNDN);
        ;
        mpfr_div(r3901, r3899, r3900, MPFR_RNDN);
        mpfr_tan(r3902, r3901, MPFR_RNDN);
        return mpfr_get_d(r3902, MPFR_RNDN);
}

static mpfr_t r3903, r3904, r3905, r3906;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r3903);
        mpfr_init_set_str(r3904, "2", 10, MPFR_RNDN);
        mpfr_init(r3905);
        mpfr_init(r3906);
}

double f_dm(double x) {
        mpfr_set_d(r3903, x, MPFR_RNDN);
        ;
        mpfr_div(r3905, r3903, r3904, MPFR_RNDN);
        mpfr_tan(r3906, r3905, MPFR_RNDN);
        return mpfr_get_d(r3906, MPFR_RNDN);
}

