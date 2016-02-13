#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.9";

double f_if(float x) {
        float r4930 = 1.0;
        float r4931 = x;
        float r4932 = r4930 / r4931;
        float r4933 = 1.0 / tan(r4931);
        float r4934 = r4932 - r4933;
        return r4934;
}

double f_id(double x) {
        double r4935 = 1.0;
        double r4936 = x;
        double r4937 = r4935 / r4936;
        double r4938 = 1.0 / tan(r4936);
        double r4939 = r4937 - r4938;
        return r4939;
}


double f_of(float x) {
        float r4940 = x;
        float r4941 = fabs(r4940);
        float r4942 = 0.026;
        bool r4943 = r4941 < r4942;
        float r4944 = 3.0;
        float r4945 = r4940 / r4944;
        float r4946 = 1.0;
        float r4947 = r4940 * r4940;
        float r4948 = 15.0;
        float r4949 = r4947 / r4948;
        float r4950 = r4946 + r4949;
        float r4951 = r4945 * r4950;
        float r4952 = r4946 / r4940;
        float r4953 = 1.0 / tan(r4940);
        float r4954 = r4952 - r4953;
        float r4955 = r4943 ? r4951 : r4954;
        return r4955;
}

double f_od(double x) {
        double r4956 = x;
        double r4957 = fabs(r4956);
        double r4958 = 0.026;
        bool r4959 = r4957 < r4958;
        double r4960 = 3.0;
        double r4961 = r4956 / r4960;
        double r4962 = 1.0;
        double r4963 = r4956 * r4956;
        double r4964 = 15.0;
        double r4965 = r4963 / r4964;
        double r4966 = r4962 + r4965;
        double r4967 = r4961 * r4966;
        double r4968 = r4962 / r4956;
        double r4969 = 1.0 / tan(r4956);
        double r4970 = r4968 - r4969;
        double r4971 = r4959 ? r4967 : r4970;
        return r4971;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4972, r4973, r4974, r4975, r4976;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r4972, "1", 10, MPFR_RNDN);
        mpfr_init(r4973);
        mpfr_init(r4974);
        mpfr_init(r4975);
        mpfr_init(r4976);
}

double f_im(double x) {
        ;
        mpfr_set_d(r4973, x, MPFR_RNDN);
        mpfr_div(r4974, r4972, r4973, MPFR_RNDN);
        mpfr_cot(r4975, r4973, MPFR_RNDN);
        mpfr_sub(r4976, r4974, r4975, MPFR_RNDN);
        return mpfr_get_d(r4976, MPFR_RNDN);
}

static mpfr_t r4977, r4978, r4979, r4980, r4981, r4982, r4983, r4984, r4985, r4986, r4987, r4988, r4989, r4990, r4991, r4992;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4977);
        mpfr_init(r4978);
        mpfr_init_set_str(r4979, "0.026", 10, MPFR_RNDN);
        mpfr_init(r4980);
        mpfr_init_set_str(r4981, "3", 10, MPFR_RNDN);
        mpfr_init(r4982);
        mpfr_init_set_str(r4983, "1", 10, MPFR_RNDN);
        mpfr_init(r4984);
        mpfr_init_set_str(r4985, "15", 10, MPFR_RNDN);
        mpfr_init(r4986);
        mpfr_init(r4987);
        mpfr_init(r4988);
        mpfr_init(r4989);
        mpfr_init(r4990);
        mpfr_init(r4991);
        mpfr_init(r4992);
}

double f_fm(double x) {
        mpfr_set_d(r4977, x, MPFR_RNDN);
        mpfr_abs(r4978, r4977, MPFR_RNDN);
        ;
        mpfr_set_si(r4980, mpfr_cmp(r4978, r4979) < 0, MPFR_RNDN);
        ;
        mpfr_div(r4982, r4977, r4981, MPFR_RNDN);
        ;
        mpfr_mul(r4984, r4977, r4977, MPFR_RNDN);
        ;
        mpfr_div(r4986, r4984, r4985, MPFR_RNDN);
        mpfr_add(r4987, r4983, r4986, MPFR_RNDN);
        mpfr_mul(r4988, r4982, r4987, MPFR_RNDN);
        mpfr_div(r4989, r4983, r4977, MPFR_RNDN);
        mpfr_cot(r4990, r4977, MPFR_RNDN);
        mpfr_sub(r4991, r4989, r4990, MPFR_RNDN);
        if (mpfr_get_si(r4980, MPFR_RNDN)) { mpfr_set(r4992, r4988, MPFR_RNDN); } else { mpfr_set(r4992, r4991, MPFR_RNDN); };
        return mpfr_get_d(r4992, MPFR_RNDN);
}

static mpfr_t r4993, r4994, r4995, r4996, r4997, r4998, r4999, r5000, r5001, r5002, r5003, r5004, r5005, r5006, r5007, r5008;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4993);
        mpfr_init(r4994);
        mpfr_init_set_str(r4995, "0.026", 10, MPFR_RNDN);
        mpfr_init(r4996);
        mpfr_init_set_str(r4997, "3", 10, MPFR_RNDN);
        mpfr_init(r4998);
        mpfr_init_set_str(r4999, "1", 10, MPFR_RNDN);
        mpfr_init(r5000);
        mpfr_init_set_str(r5001, "15", 10, MPFR_RNDN);
        mpfr_init(r5002);
        mpfr_init(r5003);
        mpfr_init(r5004);
        mpfr_init(r5005);
        mpfr_init(r5006);
        mpfr_init(r5007);
        mpfr_init(r5008);
}

double f_dm(double x) {
        mpfr_set_d(r4993, x, MPFR_RNDN);
        mpfr_abs(r4994, r4993, MPFR_RNDN);
        ;
        mpfr_set_si(r4996, mpfr_cmp(r4994, r4995) < 0, MPFR_RNDN);
        ;
        mpfr_div(r4998, r4993, r4997, MPFR_RNDN);
        ;
        mpfr_mul(r5000, r4993, r4993, MPFR_RNDN);
        ;
        mpfr_div(r5002, r5000, r5001, MPFR_RNDN);
        mpfr_add(r5003, r4999, r5002, MPFR_RNDN);
        mpfr_mul(r5004, r4998, r5003, MPFR_RNDN);
        mpfr_div(r5005, r4999, r4993, MPFR_RNDN);
        mpfr_cot(r5006, r4993, MPFR_RNDN);
        mpfr_sub(r5007, r5005, r5006, MPFR_RNDN);
        if (mpfr_get_si(r4996, MPFR_RNDN)) { mpfr_set(r5008, r5004, MPFR_RNDN); } else { mpfr_set(r5008, r5007, MPFR_RNDN); };
        return mpfr_get_d(r5008, MPFR_RNDN);
}

