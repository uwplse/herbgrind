#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.6";

double f_if(float x) {
        float r3953 = 1.0;
        float r3954 = x;
        float r3955 = sqrt(r3954);
        float r3956 = r3953 / r3955;
        float r3957 = r3954 + r3953;
        float r3958 = sqrt(r3957);
        float r3959 = r3953 / r3958;
        float r3960 = r3956 - r3959;
        return r3960;
}

double f_id(double x) {
        double r3961 = 1.0;
        double r3962 = x;
        double r3963 = sqrt(r3962);
        double r3964 = r3961 / r3963;
        double r3965 = r3962 + r3961;
        double r3966 = sqrt(r3965);
        double r3967 = r3961 / r3966;
        double r3968 = r3964 - r3967;
        return r3968;
}


double f_of(float x) {
        float r3969 = 1.0;
        float r3970 = x;
        float r3971 = r3970 + r3969;
        float r3972 = sqrt(r3970);
        float r3973 = r3971 * r3972;
        float r3974 = sqrt(r3971);
        float r3975 = r3970 * r3974;
        float r3976 = r3973 + r3975;
        float r3977 = r3969 / r3976;
        return r3977;
}

double f_od(double x) {
        double r3978 = 1.0;
        double r3979 = x;
        double r3980 = r3979 + r3978;
        double r3981 = sqrt(r3979);
        double r3982 = r3980 * r3981;
        double r3983 = sqrt(r3980);
        double r3984 = r3979 * r3983;
        double r3985 = r3982 + r3984;
        double r3986 = r3978 / r3985;
        return r3986;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r3987, r3988, r3989, r3990, r3991, r3992, r3993, r3994;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3987, "1", 10, MPFR_RNDN);
        mpfr_init(r3988);
        mpfr_init(r3989);
        mpfr_init(r3990);
        mpfr_init(r3991);
        mpfr_init(r3992);
        mpfr_init(r3993);
        mpfr_init(r3994);
}

double f_im(double x) {
        ;
        mpfr_set_d(r3988, x, MPFR_RNDN);
        mpfr_sqrt(r3989, r3988, MPFR_RNDN);
        mpfr_div(r3990, r3987, r3989, MPFR_RNDN);
        mpfr_add(r3991, r3988, r3987, MPFR_RNDN);
        mpfr_sqrt(r3992, r3991, MPFR_RNDN);
        mpfr_div(r3993, r3987, r3992, MPFR_RNDN);
        mpfr_sub(r3994, r3990, r3993, MPFR_RNDN);
        return mpfr_get_d(r3994, MPFR_RNDN);
}

static mpfr_t r3995, r3996, r3997, r3998, r3999, r4000, r4001, r4002, r4003;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3995, "1", 10, MPFR_RNDN);
        mpfr_init(r3996);
        mpfr_init(r3997);
        mpfr_init(r3998);
        mpfr_init(r3999);
        mpfr_init(r4000);
        mpfr_init(r4001);
        mpfr_init(r4002);
        mpfr_init(r4003);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r3996, x, MPFR_RNDN);
        mpfr_add(r3997, r3996, r3995, MPFR_RNDN);
        mpfr_sqrt(r3998, r3996, MPFR_RNDN);
        mpfr_mul(r3999, r3997, r3998, MPFR_RNDN);
        mpfr_sqrt(r4000, r3997, MPFR_RNDN);
        mpfr_mul(r4001, r3996, r4000, MPFR_RNDN);
        mpfr_add(r4002, r3999, r4001, MPFR_RNDN);
        mpfr_div(r4003, r3995, r4002, MPFR_RNDN);
        return mpfr_get_d(r4003, MPFR_RNDN);
}

static mpfr_t r4004, r4005, r4006, r4007, r4008, r4009, r4010, r4011, r4012;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4004, "1", 10, MPFR_RNDN);
        mpfr_init(r4005);
        mpfr_init(r4006);
        mpfr_init(r4007);
        mpfr_init(r4008);
        mpfr_init(r4009);
        mpfr_init(r4010);
        mpfr_init(r4011);
        mpfr_init(r4012);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r4005, x, MPFR_RNDN);
        mpfr_add(r4006, r4005, r4004, MPFR_RNDN);
        mpfr_sqrt(r4007, r4005, MPFR_RNDN);
        mpfr_mul(r4008, r4006, r4007, MPFR_RNDN);
        mpfr_sqrt(r4009, r4006, MPFR_RNDN);
        mpfr_mul(r4010, r4005, r4009, MPFR_RNDN);
        mpfr_add(r4011, r4008, r4010, MPFR_RNDN);
        mpfr_div(r4012, r4004, r4011, MPFR_RNDN);
        return mpfr_get_d(r4012, MPFR_RNDN);
}

