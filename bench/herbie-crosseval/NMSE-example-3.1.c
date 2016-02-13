#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.1";

double f_if(float x) {
        float r3773 = x;
        float r3774 = 1.0;
        float r3775 = r3773 + r3774;
        float r3776 = sqrt(r3775);
        float r3777 = sqrt(r3773);
        float r3778 = r3776 - r3777;
        return r3778;
}

double f_id(double x) {
        double r3779 = x;
        double r3780 = 1.0;
        double r3781 = r3779 + r3780;
        double r3782 = sqrt(r3781);
        double r3783 = sqrt(r3779);
        double r3784 = r3782 - r3783;
        return r3784;
}


double f_of(float x) {
        float r3785 = 1.0;
        float r3786 = x;
        float r3787 = r3786 + r3785;
        float r3788 = sqrt(r3787);
        float r3789 = sqrt(r3786);
        float r3790 = r3788 + r3789;
        float r3791 = r3785 / r3790;
        return r3791;
}

double f_od(double x) {
        double r3792 = 1.0;
        double r3793 = x;
        double r3794 = r3793 + r3792;
        double r3795 = sqrt(r3794);
        double r3796 = sqrt(r3793);
        double r3797 = r3795 + r3796;
        double r3798 = r3792 / r3797;
        return r3798;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r3799, r3800, r3801, r3802, r3803, r3804;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r3799);
        mpfr_init_set_str(r3800, "1", 10, MPFR_RNDN);
        mpfr_init(r3801);
        mpfr_init(r3802);
        mpfr_init(r3803);
        mpfr_init(r3804);
}

double f_im(double x) {
        mpfr_set_d(r3799, x, MPFR_RNDN);
        ;
        mpfr_add(r3801, r3799, r3800, MPFR_RNDN);
        mpfr_sqrt(r3802, r3801, MPFR_RNDN);
        mpfr_sqrt(r3803, r3799, MPFR_RNDN);
        mpfr_sub(r3804, r3802, r3803, MPFR_RNDN);
        return mpfr_get_d(r3804, MPFR_RNDN);
}

static mpfr_t r3805, r3806, r3807, r3808, r3809, r3810, r3811;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3805, "1", 10, MPFR_RNDN);
        mpfr_init(r3806);
        mpfr_init(r3807);
        mpfr_init(r3808);
        mpfr_init(r3809);
        mpfr_init(r3810);
        mpfr_init(r3811);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r3806, x, MPFR_RNDN);
        mpfr_add(r3807, r3806, r3805, MPFR_RNDN);
        mpfr_sqrt(r3808, r3807, MPFR_RNDN);
        mpfr_sqrt(r3809, r3806, MPFR_RNDN);
        mpfr_add(r3810, r3808, r3809, MPFR_RNDN);
        mpfr_div(r3811, r3805, r3810, MPFR_RNDN);
        return mpfr_get_d(r3811, MPFR_RNDN);
}

static mpfr_t r3812, r3813, r3814, r3815, r3816, r3817, r3818;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r3812, "1", 10, MPFR_RNDN);
        mpfr_init(r3813);
        mpfr_init(r3814);
        mpfr_init(r3815);
        mpfr_init(r3816);
        mpfr_init(r3817);
        mpfr_init(r3818);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r3813, x, MPFR_RNDN);
        mpfr_add(r3814, r3813, r3812, MPFR_RNDN);
        mpfr_sqrt(r3815, r3814, MPFR_RNDN);
        mpfr_sqrt(r3816, r3813, MPFR_RNDN);
        mpfr_add(r3817, r3815, r3816, MPFR_RNDN);
        mpfr_div(r3818, r3812, r3817, MPFR_RNDN);
        return mpfr_get_d(r3818, MPFR_RNDN);
}

