#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.2.1";

double f_if(float a, float b_2F2, float c) {
        float r4713 = b_2F2;
        float r4714 = -r4713;
        float r4715 = r4713 * r4713;
        float r4716 = a;
        float r4717 = c;
        float r4718 = r4716 * r4717;
        float r4719 = r4715 - r4718;
        float r4720 = sqrt(r4719);
        float r4721 = r4714 + r4720;
        float r4722 = r4721 / r4716;
        return r4722;
}

double f_id(double a, double b_2F2, double c) {
        double r4723 = b_2F2;
        double r4724 = -r4723;
        double r4725 = r4723 * r4723;
        double r4726 = a;
        double r4727 = c;
        double r4728 = r4726 * r4727;
        double r4729 = r4725 - r4728;
        double r4730 = sqrt(r4729);
        double r4731 = r4724 + r4730;
        double r4732 = r4731 / r4726;
        return r4732;
}


double f_of(float a, float b_2F2, float c) {
        float r4733 = b_2F2;
        float r4734 = -r4733;
        float r4735 = r4733 * r4733;
        float r4736 = a;
        float r4737 = c;
        float r4738 = r4736 * r4737;
        float r4739 = r4735 - r4738;
        float r4740 = sqrt(r4739);
        float r4741 = r4734 + r4740;
        float r4742 = r4741 / r4736;
        return r4742;
}

double f_od(double a, double b_2F2, double c) {
        double r4743 = b_2F2;
        double r4744 = -r4743;
        double r4745 = r4743 * r4743;
        double r4746 = a;
        double r4747 = c;
        double r4748 = r4746 * r4747;
        double r4749 = r4745 - r4748;
        double r4750 = sqrt(r4749);
        double r4751 = r4744 + r4750;
        double r4752 = r4751 / r4746;
        return r4752;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4753, r4754, r4755, r4756, r4757, r4758, r4759, r4760, r4761, r4762;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4753);
        mpfr_init(r4754);
        mpfr_init(r4755);
        mpfr_init(r4756);
        mpfr_init(r4757);
        mpfr_init(r4758);
        mpfr_init(r4759);
        mpfr_init(r4760);
        mpfr_init(r4761);
        mpfr_init(r4762);
}

double f_im(double a, double b_2F2, double c) {
        mpfr_set_d(r4753, b_2F2, MPFR_RNDN);
        mpfr_neg(r4754, r4753, MPFR_RNDN);
        mpfr_mul(r4755, r4753, r4753, MPFR_RNDN);
        mpfr_set_d(r4756, a, MPFR_RNDN);
        mpfr_set_d(r4757, c, MPFR_RNDN);
        mpfr_mul(r4758, r4756, r4757, MPFR_RNDN);
        mpfr_sub(r4759, r4755, r4758, MPFR_RNDN);
        mpfr_sqrt(r4760, r4759, MPFR_RNDN);
        mpfr_add(r4761, r4754, r4760, MPFR_RNDN);
        mpfr_div(r4762, r4761, r4756, MPFR_RNDN);
        return mpfr_get_d(r4762, MPFR_RNDN);
}

static mpfr_t r4763, r4764, r4765, r4766, r4767, r4768, r4769, r4770, r4771, r4772;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4763);
        mpfr_init(r4764);
        mpfr_init(r4765);
        mpfr_init(r4766);
        mpfr_init(r4767);
        mpfr_init(r4768);
        mpfr_init(r4769);
        mpfr_init(r4770);
        mpfr_init(r4771);
        mpfr_init(r4772);
}

double f_fm(double a, double b_2F2, double c) {
        mpfr_set_d(r4763, b_2F2, MPFR_RNDN);
        mpfr_neg(r4764, r4763, MPFR_RNDN);
        mpfr_mul(r4765, r4763, r4763, MPFR_RNDN);
        mpfr_set_d(r4766, a, MPFR_RNDN);
        mpfr_set_d(r4767, c, MPFR_RNDN);
        mpfr_mul(r4768, r4766, r4767, MPFR_RNDN);
        mpfr_sub(r4769, r4765, r4768, MPFR_RNDN);
        mpfr_sqrt(r4770, r4769, MPFR_RNDN);
        mpfr_add(r4771, r4764, r4770, MPFR_RNDN);
        mpfr_div(r4772, r4771, r4766, MPFR_RNDN);
        return mpfr_get_d(r4772, MPFR_RNDN);
}

static mpfr_t r4773, r4774, r4775, r4776, r4777, r4778, r4779, r4780, r4781, r4782;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4773);
        mpfr_init(r4774);
        mpfr_init(r4775);
        mpfr_init(r4776);
        mpfr_init(r4777);
        mpfr_init(r4778);
        mpfr_init(r4779);
        mpfr_init(r4780);
        mpfr_init(r4781);
        mpfr_init(r4782);
}

double f_dm(double a, double b_2F2, double c) {
        mpfr_set_d(r4773, b_2F2, MPFR_RNDN);
        mpfr_neg(r4774, r4773, MPFR_RNDN);
        mpfr_mul(r4775, r4773, r4773, MPFR_RNDN);
        mpfr_set_d(r4776, a, MPFR_RNDN);
        mpfr_set_d(r4777, c, MPFR_RNDN);
        mpfr_mul(r4778, r4776, r4777, MPFR_RNDN);
        mpfr_sub(r4779, r4775, r4778, MPFR_RNDN);
        mpfr_sqrt(r4780, r4779, MPFR_RNDN);
        mpfr_add(r4781, r4774, r4780, MPFR_RNDN);
        mpfr_div(r4782, r4781, r4776, MPFR_RNDN);
        return mpfr_get_d(r4782, MPFR_RNDN);
}

