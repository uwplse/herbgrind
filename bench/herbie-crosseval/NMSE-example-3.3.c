#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.3";

double f_if(float x, float eps) {
        float r3819 = x;
        float r3820 = eps;
        float r3821 = r3819 + r3820;
        float r3822 = sin(r3821);
        float r3823 = sin(r3819);
        float r3824 = r3822 - r3823;
        return r3824;
}

double f_id(double x, double eps) {
        double r3825 = x;
        double r3826 = eps;
        double r3827 = r3825 + r3826;
        double r3828 = sin(r3827);
        double r3829 = sin(r3825);
        double r3830 = r3828 - r3829;
        return r3830;
}


double f_of(float x, float eps) {
        float r3831 = 2.0;
        float r3832 = x;
        float r3833 = eps;
        float r3834 = r3833 / r3831;
        float r3835 = r3832 + r3834;
        float r3836 = cos(r3835);
        float r3837 = sin(r3834);
        float r3838 = r3836 * r3837;
        float r3839 = r3831 * r3838;
        return r3839;
}

double f_od(double x, double eps) {
        double r3840 = 2.0;
        double r3841 = x;
        double r3842 = eps;
        double r3843 = r3842 / r3840;
        double r3844 = r3841 + r3843;
        double r3845 = cos(r3844);
        double r3846 = sin(r3843);
        double r3847 = r3845 * r3846;
        double r3848 = r3840 * r3847;
        return r3848;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r3849, r3850, r3851, r3852, r3853, r3854;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r3849);
        mpfr_init(r3850);
        mpfr_init(r3851);
        mpfr_init(r3852);
        mpfr_init(r3853);
        mpfr_init(r3854);
}

double f_im(double x, double eps) {
        mpfr_set_d(r3849, x, MPFR_RNDN);
        mpfr_set_d(r3850, eps, MPFR_RNDN);
        mpfr_add(r3851, r3849, r3850, MPFR_RNDN);
        mpfr_sin(r3852, r3851, MPFR_RNDN);
        mpfr_sin(r3853, r3849, MPFR_RNDN);
        mpfr_sub(r3854, r3852, r3853, MPFR_RNDN);
        return mpfr_get_d(r3854, MPFR_RNDN);
}

static mpfr_t r3855, r3856, r3857, r3858, r3859, r3860, r3861, r3862, r3863;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3855, "2", 10, MPFR_RNDN);
        mpfr_init(r3856);
        mpfr_init(r3857);
        mpfr_init(r3858);
        mpfr_init(r3859);
        mpfr_init(r3860);
        mpfr_init(r3861);
        mpfr_init(r3862);
        mpfr_init(r3863);
}

double f_fm(double x, double eps) {
        ;
        mpfr_set_d(r3856, x, MPFR_RNDN);
        mpfr_set_d(r3857, eps, MPFR_RNDN);
        mpfr_div(r3858, r3857, r3855, MPFR_RNDN);
        mpfr_add(r3859, r3856, r3858, MPFR_RNDN);
        mpfr_cos(r3860, r3859, MPFR_RNDN);
        mpfr_sin(r3861, r3858, MPFR_RNDN);
        mpfr_mul(r3862, r3860, r3861, MPFR_RNDN);
        mpfr_mul(r3863, r3855, r3862, MPFR_RNDN);
        return mpfr_get_d(r3863, MPFR_RNDN);
}

static mpfr_t r3864, r3865, r3866, r3867, r3868, r3869, r3870, r3871, r3872;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3864, "2", 10, MPFR_RNDN);
        mpfr_init(r3865);
        mpfr_init(r3866);
        mpfr_init(r3867);
        mpfr_init(r3868);
        mpfr_init(r3869);
        mpfr_init(r3870);
        mpfr_init(r3871);
        mpfr_init(r3872);
}

double f_dm(double x, double eps) {
        ;
        mpfr_set_d(r3865, x, MPFR_RNDN);
        mpfr_set_d(r3866, eps, MPFR_RNDN);
        mpfr_div(r3867, r3866, r3864, MPFR_RNDN);
        mpfr_add(r3868, r3865, r3867, MPFR_RNDN);
        mpfr_cos(r3869, r3868, MPFR_RNDN);
        mpfr_sin(r3870, r3867, MPFR_RNDN);
        mpfr_mul(r3871, r3869, r3870, MPFR_RNDN);
        mpfr_mul(r3872, r3864, r3871, MPFR_RNDN);
        return mpfr_get_d(r3872, MPFR_RNDN);
}

