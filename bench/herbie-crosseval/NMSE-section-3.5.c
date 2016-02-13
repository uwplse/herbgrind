#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE section 3.5";

double f_if(float a, float x) {
        float r5425 = a;
        float r5426 = x;
        float r5427 = r5425 * r5426;
        float r5428 = exp(r5427);
        float r5429 = 1.0;
        float r5430 = r5428 - r5429;
        return r5430;
}

double f_id(double a, double x) {
        double r5431 = a;
        double r5432 = x;
        double r5433 = r5431 * r5432;
        double r5434 = exp(r5433);
        double r5435 = 1.0;
        double r5436 = r5434 - r5435;
        return r5436;
}


double f_of(float a, float x) {
        float r5437 = a;
        float r5438 = x;
        float r5439 = r5437 * r5438;
        float r5440 = fabs(r5439);
        float r5441 = 0.1;
        bool r5442 = r5440 < r5441;
        float r5443 = 1.0;
        float r5444 = 2.0;
        float r5445 = r5439 / r5444;
        float r5446 = r5439 * r5439;
        float r5447 = 6.0;
        float r5448 = r5446 / r5447;
        float r5449 = r5445 + r5448;
        float r5450 = r5443 + r5449;
        float r5451 = r5439 * r5450;
        float r5452 = exp(r5439);
        float r5453 = r5452 - r5443;
        float r5454 = r5442 ? r5451 : r5453;
        return r5454;
}

double f_od(double a, double x) {
        double r5455 = a;
        double r5456 = x;
        double r5457 = r5455 * r5456;
        double r5458 = fabs(r5457);
        double r5459 = 0.1;
        bool r5460 = r5458 < r5459;
        double r5461 = 1.0;
        double r5462 = 2.0;
        double r5463 = r5457 / r5462;
        double r5464 = r5457 * r5457;
        double r5465 = 6.0;
        double r5466 = r5464 / r5465;
        double r5467 = r5463 + r5466;
        double r5468 = r5461 + r5467;
        double r5469 = r5457 * r5468;
        double r5470 = exp(r5457);
        double r5471 = r5470 - r5461;
        double r5472 = r5460 ? r5469 : r5471;
        return r5472;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r5473, r5474, r5475, r5476, r5477, r5478;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5473);
        mpfr_init(r5474);
        mpfr_init(r5475);
        mpfr_init(r5476);
        mpfr_init_set_str(r5477, "1", 10, MPFR_RNDN);
        mpfr_init(r5478);
}

double f_im(double a, double x) {
        mpfr_set_d(r5473, a, MPFR_RNDN);
        mpfr_set_d(r5474, x, MPFR_RNDN);
        mpfr_mul(r5475, r5473, r5474, MPFR_RNDN);
        mpfr_exp(r5476, r5475, MPFR_RNDN);
        ;
        mpfr_sub(r5478, r5476, r5477, MPFR_RNDN);
        return mpfr_get_d(r5478, MPFR_RNDN);
}

static mpfr_t r5479, r5480, r5481, r5482, r5483, r5484, r5485, r5486, r5487, r5488, r5489, r5490, r5491, r5492, r5493, r5494, r5495, r5496;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5479);
        mpfr_init(r5480);
        mpfr_init(r5481);
        mpfr_init(r5482);
        mpfr_init_set_str(r5483, "1/10", 10, MPFR_RNDN);
        mpfr_init(r5484);
        mpfr_init_set_str(r5485, "1", 10, MPFR_RNDN);
        mpfr_init_set_str(r5486, "2", 10, MPFR_RNDN);
        mpfr_init(r5487);
        mpfr_init(r5488);
        mpfr_init_set_str(r5489, "6", 10, MPFR_RNDN);
        mpfr_init(r5490);
        mpfr_init(r5491);
        mpfr_init(r5492);
        mpfr_init(r5493);
        mpfr_init(r5494);
        mpfr_init(r5495);
        mpfr_init(r5496);
}

double f_fm(double a, double x) {
        mpfr_set_d(r5479, a, MPFR_RNDN);
        mpfr_set_d(r5480, x, MPFR_RNDN);
        mpfr_mul(r5481, r5479, r5480, MPFR_RNDN);
        mpfr_abs(r5482, r5481, MPFR_RNDN);
        ;
        mpfr_set_si(r5484, mpfr_cmp(r5482, r5483) < 0, MPFR_RNDN);
        ;
        ;
        mpfr_div(r5487, r5481, r5486, MPFR_RNDN);
        mpfr_mul(r5488, r5481, r5481, MPFR_RNDN);
        ;
        mpfr_div(r5490, r5488, r5489, MPFR_RNDN);
        mpfr_add(r5491, r5487, r5490, MPFR_RNDN);
        mpfr_add(r5492, r5485, r5491, MPFR_RNDN);
        mpfr_mul(r5493, r5481, r5492, MPFR_RNDN);
        mpfr_exp(r5494, r5481, MPFR_RNDN);
        mpfr_sub(r5495, r5494, r5485, MPFR_RNDN);
        if (mpfr_get_si(r5484, MPFR_RNDN)) { mpfr_set(r5496, r5493, MPFR_RNDN); } else { mpfr_set(r5496, r5495, MPFR_RNDN); };
        return mpfr_get_d(r5496, MPFR_RNDN);
}

static mpfr_t r5497, r5498, r5499, r5500, r5501, r5502, r5503, r5504, r5505, r5506, r5507, r5508, r5509, r5510, r5511, r5512, r5513, r5514;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r5497);
        mpfr_init(r5498);
        mpfr_init(r5499);
        mpfr_init(r5500);
        mpfr_init_set_str(r5501, "1/10", 10, MPFR_RNDN);
        mpfr_init(r5502);
        mpfr_init_set_str(r5503, "1", 10, MPFR_RNDN);
        mpfr_init_set_str(r5504, "2", 10, MPFR_RNDN);
        mpfr_init(r5505);
        mpfr_init(r5506);
        mpfr_init_set_str(r5507, "6", 10, MPFR_RNDN);
        mpfr_init(r5508);
        mpfr_init(r5509);
        mpfr_init(r5510);
        mpfr_init(r5511);
        mpfr_init(r5512);
        mpfr_init(r5513);
        mpfr_init(r5514);
}

double f_dm(double a, double x) {
        mpfr_set_d(r5497, a, MPFR_RNDN);
        mpfr_set_d(r5498, x, MPFR_RNDN);
        mpfr_mul(r5499, r5497, r5498, MPFR_RNDN);
        mpfr_abs(r5500, r5499, MPFR_RNDN);
        ;
        mpfr_set_si(r5502, mpfr_cmp(r5500, r5501) < 0, MPFR_RNDN);
        ;
        ;
        mpfr_div(r5505, r5499, r5504, MPFR_RNDN);
        mpfr_mul(r5506, r5499, r5499, MPFR_RNDN);
        ;
        mpfr_div(r5508, r5506, r5507, MPFR_RNDN);
        mpfr_add(r5509, r5505, r5508, MPFR_RNDN);
        mpfr_add(r5510, r5503, r5509, MPFR_RNDN);
        mpfr_mul(r5511, r5499, r5510, MPFR_RNDN);
        mpfr_exp(r5512, r5499, MPFR_RNDN);
        mpfr_sub(r5513, r5512, r5503, MPFR_RNDN);
        if (mpfr_get_si(r5502, MPFR_RNDN)) { mpfr_set(r5514, r5511, MPFR_RNDN); } else { mpfr_set(r5514, r5513, MPFR_RNDN); };
        return mpfr_get_d(r5514, MPFR_RNDN);
}

