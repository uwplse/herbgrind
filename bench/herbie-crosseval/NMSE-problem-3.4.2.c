#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.4.2";

double f_if(float a, float b, float eps) {
        float r5124 = eps;
        float r5125 = a;
        float r5126 = b;
        float r5127 = r5125 + r5126;
        float r5128 = r5127 * r5124;
        float r5129 = exp(r5128);
        float r5130 = 1.0;
        float r5131 = r5129 - r5130;
        float r5132 = r5124 * r5131;
        float r5133 = r5125 * r5124;
        float r5134 = exp(r5133);
        float r5135 = r5134 - r5130;
        float r5136 = r5126 * r5124;
        float r5137 = exp(r5136);
        float r5138 = r5137 - r5130;
        float r5139 = r5135 * r5138;
        float r5140 = r5132 / r5139;
        return r5140;
}

double f_id(double a, double b, double eps) {
        double r5141 = eps;
        double r5142 = a;
        double r5143 = b;
        double r5144 = r5142 + r5143;
        double r5145 = r5144 * r5141;
        double r5146 = exp(r5145);
        double r5147 = 1.0;
        double r5148 = r5146 - r5147;
        double r5149 = r5141 * r5148;
        double r5150 = r5142 * r5141;
        double r5151 = exp(r5150);
        double r5152 = r5151 - r5147;
        double r5153 = r5143 * r5141;
        double r5154 = exp(r5153);
        double r5155 = r5154 - r5147;
        double r5156 = r5152 * r5155;
        double r5157 = r5149 / r5156;
        return r5157;
}


double f_of(float a, float b, float eps) {
        float r5158 = a;
        float r5159 = b;
        float r5160 = r5158 + r5159;
        float r5161 = r5158 * r5159;
        float r5162 = r5160 / r5161;
        return r5162;
}

double f_od(double a, double b, double eps) {
        double r5163 = a;
        double r5164 = b;
        double r5165 = r5163 + r5164;
        double r5166 = r5163 * r5164;
        double r5167 = r5165 / r5166;
        return r5167;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r5168, r5169, r5170, r5171, r5172, r5173, r5174, r5175, r5176, r5177, r5178, r5179, r5180, r5181, r5182, r5183, r5184;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5168);
        mpfr_init(r5169);
        mpfr_init(r5170);
        mpfr_init(r5171);
        mpfr_init(r5172);
        mpfr_init(r5173);
        mpfr_init_set_str(r5174, "1", 10, MPFR_RNDN);
        mpfr_init(r5175);
        mpfr_init(r5176);
        mpfr_init(r5177);
        mpfr_init(r5178);
        mpfr_init(r5179);
        mpfr_init(r5180);
        mpfr_init(r5181);
        mpfr_init(r5182);
        mpfr_init(r5183);
        mpfr_init(r5184);
}

double f_im(double a, double b, double eps) {
        mpfr_set_d(r5168, eps, MPFR_RNDN);
        mpfr_set_d(r5169, a, MPFR_RNDN);
        mpfr_set_d(r5170, b, MPFR_RNDN);
        mpfr_add(r5171, r5169, r5170, MPFR_RNDN);
        mpfr_mul(r5172, r5171, r5168, MPFR_RNDN);
        mpfr_exp(r5173, r5172, MPFR_RNDN);
        ;
        mpfr_sub(r5175, r5173, r5174, MPFR_RNDN);
        mpfr_mul(r5176, r5168, r5175, MPFR_RNDN);
        mpfr_mul(r5177, r5169, r5168, MPFR_RNDN);
        mpfr_exp(r5178, r5177, MPFR_RNDN);
        mpfr_sub(r5179, r5178, r5174, MPFR_RNDN);
        mpfr_mul(r5180, r5170, r5168, MPFR_RNDN);
        mpfr_exp(r5181, r5180, MPFR_RNDN);
        mpfr_sub(r5182, r5181, r5174, MPFR_RNDN);
        mpfr_mul(r5183, r5179, r5182, MPFR_RNDN);
        mpfr_div(r5184, r5176, r5183, MPFR_RNDN);
        return mpfr_get_d(r5184, MPFR_RNDN);
}

static mpfr_t r5185, r5186, r5187, r5188, r5189;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5185);
        mpfr_init(r5186);
        mpfr_init(r5187);
        mpfr_init(r5188);
        mpfr_init(r5189);
}

double f_fm(double a, double b, double eps) {
        mpfr_set_d(r5185, a, MPFR_RNDN);
        mpfr_set_d(r5186, b, MPFR_RNDN);
        mpfr_add(r5187, r5185, r5186, MPFR_RNDN);
        mpfr_mul(r5188, r5185, r5186, MPFR_RNDN);
        mpfr_div(r5189, r5187, r5188, MPFR_RNDN);
        return mpfr_get_d(r5189, MPFR_RNDN);
}

static mpfr_t r5190, r5191, r5192, r5193, r5194;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5190);
        mpfr_init(r5191);
        mpfr_init(r5192);
        mpfr_init(r5193);
        mpfr_init(r5194);
}

double f_dm(double a, double b, double eps) {
        mpfr_set_d(r5190, a, MPFR_RNDN);
        mpfr_set_d(r5191, b, MPFR_RNDN);
        mpfr_add(r5192, r5190, r5191, MPFR_RNDN);
        mpfr_mul(r5193, r5190, r5191, MPFR_RNDN);
        mpfr_div(r5194, r5192, r5193, MPFR_RNDN);
        return mpfr_get_d(r5194, MPFR_RNDN);
}

