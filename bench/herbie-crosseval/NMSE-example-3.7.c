#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.7";

double f_if(float x) {
        float r4783 = x;
        float r4784 = exp(r4783);
        float r4785 = 1.0;
        float r4786 = r4784 - r4785;
        return r4786;
}

double f_id(double x) {
        double r4787 = x;
        double r4788 = exp(r4787);
        double r4789 = 1.0;
        double r4790 = r4788 - r4789;
        return r4790;
}


double f_of(float x) {
        float r4791 = x;
        float r4792 = 1.0;
        float r4793 = 2.0;
        float r4794 = r4791 / r4793;
        float r4795 = r4792 + r4794;
        float r4796 = r4791 * r4791;
        float r4797 = 6.0;
        float r4798 = r4796 / r4797;
        float r4799 = r4795 + r4798;
        float r4800 = r4791 * r4799;
        return r4800;
}

double f_od(double x) {
        double r4801 = x;
        double r4802 = 1.0;
        double r4803 = 2.0;
        double r4804 = r4801 / r4803;
        double r4805 = r4802 + r4804;
        double r4806 = r4801 * r4801;
        double r4807 = 6.0;
        double r4808 = r4806 / r4807;
        double r4809 = r4805 + r4808;
        double r4810 = r4801 * r4809;
        return r4810;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4811, r4812, r4813, r4814;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4811);
        mpfr_init(r4812);
        mpfr_init_set_str(r4813, "1", 10, MPFR_RNDN);
        mpfr_init(r4814);
}

double f_im(double x) {
        mpfr_set_d(r4811, x, MPFR_RNDN);
        mpfr_exp(r4812, r4811, MPFR_RNDN);
        ;
        mpfr_sub(r4814, r4812, r4813, MPFR_RNDN);
        return mpfr_get_d(r4814, MPFR_RNDN);
}

static mpfr_t r4815, r4816, r4817, r4818, r4819, r4820, r4821, r4822, r4823, r4824;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4815);
        mpfr_init_set_str(r4816, "1", 10, MPFR_RNDN);
        mpfr_init_set_str(r4817, "2", 10, MPFR_RNDN);
        mpfr_init(r4818);
        mpfr_init(r4819);
        mpfr_init(r4820);
        mpfr_init_set_str(r4821, "6", 10, MPFR_RNDN);
        mpfr_init(r4822);
        mpfr_init(r4823);
        mpfr_init(r4824);
}

double f_fm(double x) {
        mpfr_set_d(r4815, x, MPFR_RNDN);
        ;
        ;
        mpfr_div(r4818, r4815, r4817, MPFR_RNDN);
        mpfr_add(r4819, r4816, r4818, MPFR_RNDN);
        mpfr_mul(r4820, r4815, r4815, MPFR_RNDN);
        ;
        mpfr_div(r4822, r4820, r4821, MPFR_RNDN);
        mpfr_add(r4823, r4819, r4822, MPFR_RNDN);
        mpfr_mul(r4824, r4815, r4823, MPFR_RNDN);
        return mpfr_get_d(r4824, MPFR_RNDN);
}

static mpfr_t r4825, r4826, r4827, r4828, r4829, r4830, r4831, r4832, r4833, r4834;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4825);
        mpfr_init_set_str(r4826, "1", 10, MPFR_RNDN);
        mpfr_init_set_str(r4827, "2", 10, MPFR_RNDN);
        mpfr_init(r4828);
        mpfr_init(r4829);
        mpfr_init(r4830);
        mpfr_init_set_str(r4831, "6", 10, MPFR_RNDN);
        mpfr_init(r4832);
        mpfr_init(r4833);
        mpfr_init(r4834);
}

double f_dm(double x) {
        mpfr_set_d(r4825, x, MPFR_RNDN);
        ;
        ;
        mpfr_div(r4828, r4825, r4827, MPFR_RNDN);
        mpfr_add(r4829, r4826, r4828, MPFR_RNDN);
        mpfr_mul(r4830, r4825, r4825, MPFR_RNDN);
        ;
        mpfr_div(r4832, r4830, r4831, MPFR_RNDN);
        mpfr_add(r4833, r4829, r4832, MPFR_RNDN);
        mpfr_mul(r4834, r4825, r4833, MPFR_RNDN);
        return mpfr_get_d(r4834, MPFR_RNDN);
}

