#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.5";

double f_if(float N) {
        float r3907 = N;
        float r3908 = 1.0;
        float r3909 = r3907 + r3908;
        float r3910 = atan(r3909);
        float r3911 = atan(r3907);
        float r3912 = r3910 - r3911;
        return r3912;
}

double f_id(double N) {
        double r3913 = N;
        double r3914 = 1.0;
        double r3915 = r3913 + r3914;
        double r3916 = atan(r3915);
        double r3917 = atan(r3913);
        double r3918 = r3916 - r3917;
        return r3918;
}


double f_of(float N) {
        float r3919 = 1.0;
        float r3920 = N;
        float r3921 = r3920 + r3919;
        float r3922 = r3920 * r3921;
        float r3923 = r3919 + r3922;
        float r3924 = r3919 / r3923;
        float r3925 = atan(r3924);
        return r3925;
}

double f_od(double N) {
        double r3926 = 1.0;
        double r3927 = N;
        double r3928 = r3927 + r3926;
        double r3929 = r3927 * r3928;
        double r3930 = r3926 + r3929;
        double r3931 = r3926 / r3930;
        double r3932 = atan(r3931);
        return r3932;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r3933, r3934, r3935, r3936, r3937, r3938;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r3933);
        mpfr_init_set_str(r3934, "1", 10, MPFR_RNDN);
        mpfr_init(r3935);
        mpfr_init(r3936);
        mpfr_init(r3937);
        mpfr_init(r3938);
}

double f_im(double N) {
        mpfr_set_d(r3933, N, MPFR_RNDN);
        ;
        mpfr_add(r3935, r3933, r3934, MPFR_RNDN);
        mpfr_atan(r3936, r3935, MPFR_RNDN);
        mpfr_atan(r3937, r3933, MPFR_RNDN);
        mpfr_sub(r3938, r3936, r3937, MPFR_RNDN);
        return mpfr_get_d(r3938, MPFR_RNDN);
}

static mpfr_t r3939, r3940, r3941, r3942, r3943, r3944, r3945;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3939, "1", 10, MPFR_RNDN);
        mpfr_init(r3940);
        mpfr_init(r3941);
        mpfr_init(r3942);
        mpfr_init(r3943);
        mpfr_init(r3944);
        mpfr_init(r3945);
}

double f_fm(double N) {
        ;
        mpfr_set_d(r3940, N, MPFR_RNDN);
        mpfr_add(r3941, r3940, r3939, MPFR_RNDN);
        mpfr_mul(r3942, r3940, r3941, MPFR_RNDN);
        mpfr_add(r3943, r3939, r3942, MPFR_RNDN);
        mpfr_div(r3944, r3939, r3943, MPFR_RNDN);
        mpfr_atan(r3945, r3944, MPFR_RNDN);
        return mpfr_get_d(r3945, MPFR_RNDN);
}

static mpfr_t r3946, r3947, r3948, r3949, r3950, r3951, r3952;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3946, "1", 10, MPFR_RNDN);
        mpfr_init(r3947);
        mpfr_init(r3948);
        mpfr_init(r3949);
        mpfr_init(r3950);
        mpfr_init(r3951);
        mpfr_init(r3952);
}

double f_dm(double N) {
        ;
        mpfr_set_d(r3947, N, MPFR_RNDN);
        mpfr_add(r3948, r3947, r3946, MPFR_RNDN);
        mpfr_mul(r3949, r3947, r3948, MPFR_RNDN);
        mpfr_add(r3950, r3946, r3949, MPFR_RNDN);
        mpfr_div(r3951, r3946, r3950, MPFR_RNDN);
        mpfr_atan(r3952, r3951, MPFR_RNDN);
        return mpfr_get_d(r3952, MPFR_RNDN);
}

