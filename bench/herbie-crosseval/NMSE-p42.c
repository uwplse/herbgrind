#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE p42";

double f_if(float a, float b, float c) {
        float r4517 = b;
        float r4518 = -r4517;
        float r4519 = r4517 * r4517;
        float r4520 = 4.0;
        float r4521 = a;
        float r4522 = c;
        float r4523 = r4521 * r4522;
        float r4524 = r4520 * r4523;
        float r4525 = r4519 - r4524;
        float r4526 = sqrt(r4525);
        float r4527 = r4518 - r4526;
        float r4528 = 2.0;
        float r4529 = r4528 * r4521;
        float r4530 = r4527 / r4529;
        return r4530;
}

double f_id(double a, double b, double c) {
        double r4531 = b;
        double r4532 = -r4531;
        double r4533 = r4531 * r4531;
        double r4534 = 4.0;
        double r4535 = a;
        double r4536 = c;
        double r4537 = r4535 * r4536;
        double r4538 = r4534 * r4537;
        double r4539 = r4533 - r4538;
        double r4540 = sqrt(r4539);
        double r4541 = r4532 - r4540;
        double r4542 = 2.0;
        double r4543 = r4542 * r4535;
        double r4544 = r4541 / r4543;
        return r4544;
}


double f_of(float a, float b, float c) {
        float r4545 = b;
        float r4546 = 0.0;
        bool r4547 = r4545 < r4546;
        float r4548 = c;
        float r4549 = a;
        float r4550 = -r4545;
        float r4551 = r4545 * r4545;
        float r4552 = 4.0;
        float r4553 = r4549 * r4548;
        float r4554 = r4552 * r4553;
        float r4555 = r4551 - r4554;
        float r4556 = sqrt(r4555);
        float r4557 = r4550 + r4556;
        float r4558 = 2.0;
        float r4559 = r4558 * r4549;
        float r4560 = r4557 / r4559;
        float r4561 = r4549 * r4560;
        float r4562 = r4548 / r4561;
        float r4563 = r4550 - r4556;
        float r4564 = r4563 / r4559;
        float r4565 = r4547 ? r4562 : r4564;
        return r4565;
}

double f_od(double a, double b, double c) {
        double r4566 = b;
        double r4567 = 0.0;
        bool r4568 = r4566 < r4567;
        double r4569 = c;
        double r4570 = a;
        double r4571 = -r4566;
        double r4572 = r4566 * r4566;
        double r4573 = 4.0;
        double r4574 = r4570 * r4569;
        double r4575 = r4573 * r4574;
        double r4576 = r4572 - r4575;
        double r4577 = sqrt(r4576);
        double r4578 = r4571 + r4577;
        double r4579 = 2.0;
        double r4580 = r4579 * r4570;
        double r4581 = r4578 / r4580;
        double r4582 = r4570 * r4581;
        double r4583 = r4569 / r4582;
        double r4584 = r4571 - r4577;
        double r4585 = r4584 / r4580;
        double r4586 = r4568 ? r4583 : r4585;
        return r4586;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4587, r4588, r4589, r4590, r4591, r4592, r4593, r4594, r4595, r4596, r4597, r4598, r4599, r4600;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4587);
        mpfr_init(r4588);
        mpfr_init(r4589);
        mpfr_init_set_str(r4590, "4", 10, MPFR_RNDN);
        mpfr_init(r4591);
        mpfr_init(r4592);
        mpfr_init(r4593);
        mpfr_init(r4594);
        mpfr_init(r4595);
        mpfr_init(r4596);
        mpfr_init(r4597);
        mpfr_init_set_str(r4598, "2", 10, MPFR_RNDN);
        mpfr_init(r4599);
        mpfr_init(r4600);
}

double f_im(double a, double b, double c) {
        mpfr_set_d(r4587, b, MPFR_RNDN);
        mpfr_neg(r4588, r4587, MPFR_RNDN);
        mpfr_mul(r4589, r4587, r4587, MPFR_RNDN);
        ;
        mpfr_set_d(r4591, a, MPFR_RNDN);
        mpfr_set_d(r4592, c, MPFR_RNDN);
        mpfr_mul(r4593, r4591, r4592, MPFR_RNDN);
        mpfr_mul(r4594, r4590, r4593, MPFR_RNDN);
        mpfr_sub(r4595, r4589, r4594, MPFR_RNDN);
        mpfr_sqrt(r4596, r4595, MPFR_RNDN);
        mpfr_sub(r4597, r4588, r4596, MPFR_RNDN);
        ;
        mpfr_mul(r4599, r4598, r4591, MPFR_RNDN);
        mpfr_div(r4600, r4597, r4599, MPFR_RNDN);
        return mpfr_get_d(r4600, MPFR_RNDN);
}

static mpfr_t r4601, r4602, r4603, r4604, r4605, r4606, r4607, r4608, r4609, r4610, r4611, r4612, r4613, r4614, r4615, r4616, r4617, r4618, r4619, r4620, r4621;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4601);
        mpfr_init_set_str(r4602, "0", 10, MPFR_RNDN);
        mpfr_init(r4603);
        mpfr_init(r4604);
        mpfr_init(r4605);
        mpfr_init(r4606);
        mpfr_init(r4607);
        mpfr_init_set_str(r4608, "4", 10, MPFR_RNDN);
        mpfr_init(r4609);
        mpfr_init(r4610);
        mpfr_init(r4611);
        mpfr_init(r4612);
        mpfr_init(r4613);
        mpfr_init_set_str(r4614, "2", 10, MPFR_RNDN);
        mpfr_init(r4615);
        mpfr_init(r4616);
        mpfr_init(r4617);
        mpfr_init(r4618);
        mpfr_init(r4619);
        mpfr_init(r4620);
        mpfr_init(r4621);
}

double f_fm(double a, double b, double c) {
        mpfr_set_d(r4601, b, MPFR_RNDN);
        ;
        mpfr_set_si(r4603, mpfr_cmp(r4601, r4602) < 0, MPFR_RNDN);
        mpfr_set_d(r4604, c, MPFR_RNDN);
        mpfr_set_d(r4605, a, MPFR_RNDN);
        mpfr_neg(r4606, r4601, MPFR_RNDN);
        mpfr_mul(r4607, r4601, r4601, MPFR_RNDN);
        ;
        mpfr_mul(r4609, r4605, r4604, MPFR_RNDN);
        mpfr_mul(r4610, r4608, r4609, MPFR_RNDN);
        mpfr_sub(r4611, r4607, r4610, MPFR_RNDN);
        mpfr_sqrt(r4612, r4611, MPFR_RNDN);
        mpfr_add(r4613, r4606, r4612, MPFR_RNDN);
        ;
        mpfr_mul(r4615, r4614, r4605, MPFR_RNDN);
        mpfr_div(r4616, r4613, r4615, MPFR_RNDN);
        mpfr_mul(r4617, r4605, r4616, MPFR_RNDN);
        mpfr_div(r4618, r4604, r4617, MPFR_RNDN);
        mpfr_sub(r4619, r4606, r4612, MPFR_RNDN);
        mpfr_div(r4620, r4619, r4615, MPFR_RNDN);
        if (mpfr_get_si(r4603, MPFR_RNDN)) { mpfr_set(r4621, r4618, MPFR_RNDN); } else { mpfr_set(r4621, r4620, MPFR_RNDN); };
        return mpfr_get_d(r4621, MPFR_RNDN);
}

static mpfr_t r4622, r4623, r4624, r4625, r4626, r4627, r4628, r4629, r4630, r4631, r4632, r4633, r4634, r4635, r4636, r4637, r4638, r4639, r4640, r4641, r4642;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4622);
        mpfr_init_set_str(r4623, "0", 10, MPFR_RNDN);
        mpfr_init(r4624);
        mpfr_init(r4625);
        mpfr_init(r4626);
        mpfr_init(r4627);
        mpfr_init(r4628);
        mpfr_init_set_str(r4629, "4", 10, MPFR_RNDN);
        mpfr_init(r4630);
        mpfr_init(r4631);
        mpfr_init(r4632);
        mpfr_init(r4633);
        mpfr_init(r4634);
        mpfr_init_set_str(r4635, "2", 10, MPFR_RNDN);
        mpfr_init(r4636);
        mpfr_init(r4637);
        mpfr_init(r4638);
        mpfr_init(r4639);
        mpfr_init(r4640);
        mpfr_init(r4641);
        mpfr_init(r4642);
}

double f_dm(double a, double b, double c) {
        mpfr_set_d(r4622, b, MPFR_RNDN);
        ;
        mpfr_set_si(r4624, mpfr_cmp(r4622, r4623) < 0, MPFR_RNDN);
        mpfr_set_d(r4625, c, MPFR_RNDN);
        mpfr_set_d(r4626, a, MPFR_RNDN);
        mpfr_neg(r4627, r4622, MPFR_RNDN);
        mpfr_mul(r4628, r4622, r4622, MPFR_RNDN);
        ;
        mpfr_mul(r4630, r4626, r4625, MPFR_RNDN);
        mpfr_mul(r4631, r4629, r4630, MPFR_RNDN);
        mpfr_sub(r4632, r4628, r4631, MPFR_RNDN);
        mpfr_sqrt(r4633, r4632, MPFR_RNDN);
        mpfr_add(r4634, r4627, r4633, MPFR_RNDN);
        ;
        mpfr_mul(r4636, r4635, r4626, MPFR_RNDN);
        mpfr_div(r4637, r4634, r4636, MPFR_RNDN);
        mpfr_mul(r4638, r4626, r4637, MPFR_RNDN);
        mpfr_div(r4639, r4625, r4638, MPFR_RNDN);
        mpfr_sub(r4640, r4627, r4633, MPFR_RNDN);
        mpfr_div(r4641, r4640, r4636, MPFR_RNDN);
        if (mpfr_get_si(r4624, MPFR_RNDN)) { mpfr_set(r4642, r4639, MPFR_RNDN); } else { mpfr_set(r4642, r4641, MPFR_RNDN); };
        return mpfr_get_d(r4642, MPFR_RNDN);
}

