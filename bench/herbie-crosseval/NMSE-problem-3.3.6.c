#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE problem 3.3.6";

double f_if(float N) {
        float r4261 = N;
        float r4262 = 1.0;
        float r4263 = r4261 + r4262;
        float r4264 = log(r4263);
        float r4265 = log(r4261);
        float r4266 = r4264 - r4265;
        return r4266;
}

double f_id(double N) {
        double r4267 = N;
        double r4268 = 1.0;
        double r4269 = r4267 + r4268;
        double r4270 = log(r4269);
        double r4271 = log(r4267);
        double r4272 = r4270 - r4271;
        return r4272;
}


double f_of(float N) {
        float r4273 = N;
        float r4274 = 1.0;
        float r4275 = r4273 + r4274;
        float r4276 = log(r4275);
        float r4277 = log(r4273);
        float r4278 = r4276 - r4277;
        return r4278;
}

double f_od(double N) {
        double r4279 = N;
        double r4280 = 1.0;
        double r4281 = r4279 + r4280;
        double r4282 = log(r4281);
        double r4283 = log(r4279);
        double r4284 = r4282 - r4283;
        return r4284;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4285, r4286, r4287, r4288, r4289, r4290;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4285);
        mpfr_init_set_str(r4286, "1", 10, MPFR_RNDN);
        mpfr_init(r4287);
        mpfr_init(r4288);
        mpfr_init(r4289);
        mpfr_init(r4290);
}

double f_im(double N) {
        mpfr_set_d(r4285, N, MPFR_RNDN);
        ;
        mpfr_add(r4287, r4285, r4286, MPFR_RNDN);
        mpfr_log(r4288, r4287, MPFR_RNDN);
        mpfr_log(r4289, r4285, MPFR_RNDN);
        mpfr_sub(r4290, r4288, r4289, MPFR_RNDN);
        return mpfr_get_d(r4290, MPFR_RNDN);
}

static mpfr_t r4291, r4292, r4293, r4294, r4295, r4296;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4291);
        mpfr_init_set_str(r4292, "1", 10, MPFR_RNDN);
        mpfr_init(r4293);
        mpfr_init(r4294);
        mpfr_init(r4295);
        mpfr_init(r4296);
}

double f_fm(double N) {
        mpfr_set_d(r4291, N, MPFR_RNDN);
        ;
        mpfr_add(r4293, r4291, r4292, MPFR_RNDN);
        mpfr_log(r4294, r4293, MPFR_RNDN);
        mpfr_log(r4295, r4291, MPFR_RNDN);
        mpfr_sub(r4296, r4294, r4295, MPFR_RNDN);
        return mpfr_get_d(r4296, MPFR_RNDN);
}

static mpfr_t r4297, r4298, r4299, r4300, r4301, r4302;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4297);
        mpfr_init_set_str(r4298, "1", 10, MPFR_RNDN);
        mpfr_init(r4299);
        mpfr_init(r4300);
        mpfr_init(r4301);
        mpfr_init(r4302);
}

double f_dm(double N) {
        mpfr_set_d(r4297, N, MPFR_RNDN);
        ;
        mpfr_add(r4299, r4297, r4298, MPFR_RNDN);
        mpfr_log(r4300, r4299, MPFR_RNDN);
        mpfr_log(r4301, r4297, MPFR_RNDN);
        mpfr_sub(r4302, r4300, r4301, MPFR_RNDN);
        return mpfr_get_d(r4302, MPFR_RNDN);
}

