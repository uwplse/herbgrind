#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.10";

double f_if(float x) {
        float r5009 = 1.0;
        float r5010 = x;
        float r5011 = r5009 - r5010;
        float r5012 = log(r5011);
        float r5013 = r5009 + r5010;
        float r5014 = log(r5013);
        float r5015 = r5012 / r5014;
        return r5015;
}

double f_id(double x) {
        double r5016 = 1.0;
        double r5017 = x;
        double r5018 = r5016 - r5017;
        double r5019 = log(r5018);
        double r5020 = r5016 + r5017;
        double r5021 = log(r5020);
        double r5022 = r5019 / r5021;
        return r5022;
}


double f_of(float x) {
        float r5023 = 1.0;
        float r5024 = x;
        float r5025 = r5023 + r5024;
        float r5026 = r5024 * r5024;
        float r5027 = 2.0;
        float r5028 = r5026 / r5027;
        float r5029 = r5025 + r5028;
        float r5030 = 0.4166666666666667;
        float r5031 = 3.0;
        float r5032 = pow(r5024, r5031);
        float r5033 = r5030 * r5032;
        float r5034 = r5029 + r5033;
        float r5035 = -r5034;
        return r5035;
}

double f_od(double x) {
        double r5036 = 1.0;
        double r5037 = x;
        double r5038 = r5036 + r5037;
        double r5039 = r5037 * r5037;
        double r5040 = 2.0;
        double r5041 = r5039 / r5040;
        double r5042 = r5038 + r5041;
        double r5043 = 0.4166666666666667;
        double r5044 = 3.0;
        double r5045 = pow(r5037, r5044);
        double r5046 = r5043 * r5045;
        double r5047 = r5042 + r5046;
        double r5048 = -r5047;
        return r5048;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r5049, r5050, r5051, r5052, r5053, r5054, r5055;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5049, "1", 10, MPFR_RNDN);
        mpfr_init(r5050);
        mpfr_init(r5051);
        mpfr_init(r5052);
        mpfr_init(r5053);
        mpfr_init(r5054);
        mpfr_init(r5055);
}

double f_im(double x) {
        ;
        mpfr_set_d(r5050, x, MPFR_RNDN);
        mpfr_sub(r5051, r5049, r5050, MPFR_RNDN);
        mpfr_log(r5052, r5051, MPFR_RNDN);
        mpfr_add(r5053, r5049, r5050, MPFR_RNDN);
        mpfr_log(r5054, r5053, MPFR_RNDN);
        mpfr_div(r5055, r5052, r5054, MPFR_RNDN);
        return mpfr_get_d(r5055, MPFR_RNDN);
}

static mpfr_t r5056, r5057, r5058, r5059, r5060, r5061, r5062, r5063, r5064, r5065, r5066, r5067, r5068;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5056, "1", 10, MPFR_RNDN);
        mpfr_init(r5057);
        mpfr_init(r5058);
        mpfr_init(r5059);
        mpfr_init_set_str(r5060, "2", 10, MPFR_RNDN);
        mpfr_init(r5061);
        mpfr_init(r5062);
        mpfr_init_set_str(r5063, "5/12", 10, MPFR_RNDN);
        mpfr_init_set_str(r5064, "3", 10, MPFR_RNDN);
        mpfr_init(r5065);
        mpfr_init(r5066);
        mpfr_init(r5067);
        mpfr_init(r5068);
}

double f_fm(double x) {
        ;
        mpfr_set_d(r5057, x, MPFR_RNDN);
        mpfr_add(r5058, r5056, r5057, MPFR_RNDN);
        mpfr_mul(r5059, r5057, r5057, MPFR_RNDN);
        ;
        mpfr_div(r5061, r5059, r5060, MPFR_RNDN);
        mpfr_add(r5062, r5058, r5061, MPFR_RNDN);
        ;
        ;
        mpfr_pow(r5065, r5057, r5064, MPFR_RNDN);
        mpfr_mul(r5066, r5063, r5065, MPFR_RNDN);
        mpfr_add(r5067, r5062, r5066, MPFR_RNDN);
        mpfr_neg(r5068, r5067, MPFR_RNDN);
        return mpfr_get_d(r5068, MPFR_RNDN);
}

static mpfr_t r5069, r5070, r5071, r5072, r5073, r5074, r5075, r5076, r5077, r5078, r5079, r5080, r5081;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init_set_str(r5069, "1", 10, MPFR_RNDN);
        mpfr_init(r5070);
        mpfr_init(r5071);
        mpfr_init(r5072);
        mpfr_init_set_str(r5073, "2", 10, MPFR_RNDN);
        mpfr_init(r5074);
        mpfr_init(r5075);
        mpfr_init_set_str(r5076, "5/12", 10, MPFR_RNDN);
        mpfr_init_set_str(r5077, "3", 10, MPFR_RNDN);
        mpfr_init(r5078);
        mpfr_init(r5079);
        mpfr_init(r5080);
        mpfr_init(r5081);
}

double f_dm(double x) {
        ;
        mpfr_set_d(r5070, x, MPFR_RNDN);
        mpfr_add(r5071, r5069, r5070, MPFR_RNDN);
        mpfr_mul(r5072, r5070, r5070, MPFR_RNDN);
        ;
        mpfr_div(r5074, r5072, r5073, MPFR_RNDN);
        mpfr_add(r5075, r5071, r5074, MPFR_RNDN);
        ;
        ;
        mpfr_pow(r5078, r5070, r5077, MPFR_RNDN);
        mpfr_mul(r5079, r5076, r5078, MPFR_RNDN);
        mpfr_add(r5080, r5075, r5079, MPFR_RNDN);
        mpfr_neg(r5081, r5080, MPFR_RNDN);
        return mpfr_get_d(r5081, MPFR_RNDN);
}

