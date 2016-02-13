#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.3.1";

double f_if(float x) {
        float r4013 = 1.0;
        float r4014 = x;
        float r4015 = r4014 + r4013;
        float r4016 = r4013 / r4015;
        float r4017 = r4013 / r4014;
        float r4018 = r4016 - r4017;
        return r4018;
}

double f_id(double x) {
        double r4019 = 1.0;
        double r4020 = x;
        double r4021 = r4020 + r4019;
        double r4022 = r4019 / r4021;
        double r4023 = r4019 / r4020;
        double r4024 = r4022 - r4023;
        return r4024;
}


double f_of(float x) {
        float r4025 = 1.0;
        float r4026 = x;
        float r4027 = r4026 + r4025;
        float r4028 = r4025 / r4027;
        float r4029 = r4025 / r4026;
        float r4030 = r4028 - r4029;
        return r4030;
}

double f_od(double x) {
        double r4031 = 1.0;
        double r4032 = x;
        double r4033 = r4032 + r4031;
        double r4034 = r4031 / r4033;
        double r4035 = r4031 / r4032;
        double r4036 = r4034 - r4035;
        return r4036;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4037, r4038, r4039, r4040, r4041, r4042;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4037, "1", 10, MPFR_RNDN);
        mpfr_init(r4038);
        mpfr_init(r4039);
        mpfr_init(r4040);
        mpfr_init(r4041);
        mpfr_init(r4042);
}

double f_im(double x) {
        ;
        mpfr_set_d(r4038, x, MPFR_RNDN);
        mpfr_add(r4039, r4038, r4037, MPFR_RNDN);
        mpfr_div(r4040, r4037, r4039, MPFR_RNDN);
        mpfr_div(r4041, r4037, r4038, MPFR_RNDN);
        mpfr_sub(r4042, r4040, r4041, MPFR_RNDN);
        return mpfr_get_d(r4042, MPFR_RNDN);
}

static mpfr_t r4043, r4044, r4045, r4046, r4047, r4048;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4043, "1", 10, MPFR_RNDN);
        mpfr_init(r4044);
        mpfr_init(r4045);
        mpfr_init(r4046);
        mpfr_init(r4047);
        mpfr_init(r4048);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r4044, x, MPFR_RNDN);
        mpfr_add(r4045, r4044, r4043, MPFR_RNDN);
        mpfr_div(r4046, r4043, r4045, MPFR_RNDN);
        mpfr_div(r4047, r4043, r4044, MPFR_RNDN);
        mpfr_sub(r4048, r4046, r4047, MPFR_RNDN);
        return mpfr_get_d(r4048, MPFR_RNDN);
}

static mpfr_t r4049, r4050, r4051, r4052, r4053, r4054;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4049, "1", 10, MPFR_RNDN);
        mpfr_init(r4050);
        mpfr_init(r4051);
        mpfr_init(r4052);
        mpfr_init(r4053);
        mpfr_init(r4054);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r4050, x, MPFR_RNDN);
        mpfr_add(r4051, r4050, r4049, MPFR_RNDN);
        mpfr_div(r4052, r4049, r4051, MPFR_RNDN);
        mpfr_div(r4053, r4049, r4050, MPFR_RNDN);
        mpfr_sub(r4054, r4052, r4053, MPFR_RNDN);
        return mpfr_get_d(r4054, MPFR_RNDN);
}

