#include <tgmath.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <stdbool.h>

char *name = "NMSE example 3.8";

double f_if(float N) {
        float r4835 = N;
        float r4836 = 1.0;
        float r4837 = r4835 + r4836;
        float r4838 = log(r4837);
        float r4839 = r4837 * r4838;
        float r4840 = log(r4835);
        float r4841 = r4835 * r4840;
        float r4842 = r4839 - r4841;
        float r4843 = r4842 - r4836;
        return r4843;
}

double f_id(double N) {
        double r4844 = N;
        double r4845 = 1.0;
        double r4846 = r4844 + r4845;
        double r4847 = log(r4846);
        double r4848 = r4846 * r4847;
        double r4849 = log(r4844);
        double r4850 = r4844 * r4849;
        double r4851 = r4848 - r4850;
        double r4852 = r4851 - r4845;
        return r4852;
}


double f_of(float N) {
        float r4853 = N;
        float r4854 = 1.0;
        float r4855 = r4853 + r4854;
        float r4856 = log(r4855);
        float r4857 = 2.0;
        float r4858 = r4857 * r4853;
        float r4859 = r4854 / r4858;
        float r4860 = 3.0;
        float r4861 = r4853 * r4853;
        float r4862 = r4860 * r4861;
        float r4863 = r4854 / r4862;
        float r4864 = 4.0;
        float r4865 = pow(r4853, r4860);
        float r4866 = r4864 / r4865;
        float r4867 = r4863 - r4866;
        float r4868 = r4859 - r4867;
        float r4869 = r4856 - r4868;
        return r4869;
}

double f_od(double N) {
        double r4870 = N;
        double r4871 = 1.0;
        double r4872 = r4870 + r4871;
        double r4873 = log(r4872);
        double r4874 = 2.0;
        double r4875 = r4874 * r4870;
        double r4876 = r4871 / r4875;
        double r4877 = 3.0;
        double r4878 = r4870 * r4870;
        double r4879 = r4877 * r4878;
        double r4880 = r4871 / r4879;
        double r4881 = 4.0;
        double r4882 = pow(r4870, r4877);
        double r4883 = r4881 / r4882;
        double r4884 = r4880 - r4883;
        double r4885 = r4876 - r4884;
        double r4886 = r4873 - r4885;
        return r4886;
}

void mpfr_fmod2(mpfr_t r, mpfr_t n, mpfr_t d) {
        mpfr_fmod(r, n, d, MPFR_RNDN);
        if (mpfr_cmp_ui(r, 0) < 0) mpfr_add(r, r, d, MPFR_RNDN);
}


static mpfr_t r4887, r4888, r4889, r4890, r4891, r4892, r4893, r4894, r4895;

void setup_mpfr_f_im() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4887);
        mpfr_init_set_str(r4888, "1", 10, MPFR_RNDN);
        mpfr_init(r4889);
        mpfr_init(r4890);
        mpfr_init(r4891);
        mpfr_init(r4892);
        mpfr_init(r4893);
        mpfr_init(r4894);
        mpfr_init(r4895);
}

double f_im(double N) {
        mpfr_set_d(r4887, N, MPFR_RNDN);
        ;
        mpfr_add(r4889, r4887, r4888, MPFR_RNDN);
        mpfr_log(r4890, r4889, MPFR_RNDN);
        mpfr_mul(r4891, r4889, r4890, MPFR_RNDN);
        mpfr_log(r4892, r4887, MPFR_RNDN);
        mpfr_mul(r4893, r4887, r4892, MPFR_RNDN);
        mpfr_sub(r4894, r4891, r4893, MPFR_RNDN);
        mpfr_sub(r4895, r4894, r4888, MPFR_RNDN);
        return mpfr_get_d(r4895, MPFR_RNDN);
}

static mpfr_t r4896, r4897, r4898, r4899, r4900, r4901, r4902, r4903, r4904, r4905, r4906, r4907, r4908, r4909, r4910, r4911, r4912;

void setup_mpfr_f_fm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4896);
        mpfr_init_set_str(r4897, "1", 10, MPFR_RNDN);
        mpfr_init(r4898);
        mpfr_init(r4899);
        mpfr_init_set_str(r4900, "2", 10, MPFR_RNDN);
        mpfr_init(r4901);
        mpfr_init(r4902);
        mpfr_init_set_str(r4903, "3", 10, MPFR_RNDN);
        mpfr_init(r4904);
        mpfr_init(r4905);
        mpfr_init(r4906);
        mpfr_init_set_str(r4907, "4", 10, MPFR_RNDN);
        mpfr_init(r4908);
        mpfr_init(r4909);
        mpfr_init(r4910);
        mpfr_init(r4911);
        mpfr_init(r4912);
}

double f_fm(double N) {
        mpfr_set_d(r4896, N, MPFR_RNDN);
        ;
        mpfr_add(r4898, r4896, r4897, MPFR_RNDN);
        mpfr_log(r4899, r4898, MPFR_RNDN);
        ;
        mpfr_mul(r4901, r4900, r4896, MPFR_RNDN);
        mpfr_div(r4902, r4897, r4901, MPFR_RNDN);
        ;
        mpfr_mul(r4904, r4896, r4896, MPFR_RNDN);
        mpfr_mul(r4905, r4903, r4904, MPFR_RNDN);
        mpfr_div(r4906, r4897, r4905, MPFR_RNDN);
        ;
        mpfr_pow(r4908, r4896, r4903, MPFR_RNDN);
        mpfr_div(r4909, r4907, r4908, MPFR_RNDN);
        mpfr_sub(r4910, r4906, r4909, MPFR_RNDN);
        mpfr_sub(r4911, r4902, r4910, MPFR_RNDN);
        mpfr_sub(r4912, r4899, r4911, MPFR_RNDN);
        return mpfr_get_d(r4912, MPFR_RNDN);
}

static mpfr_t r4913, r4914, r4915, r4916, r4917, r4918, r4919, r4920, r4921, r4922, r4923, r4924, r4925, r4926, r4927, r4928, r4929;

void setup_mpfr_f_dm() {
        mpfr_set_default_prec(3016);
        mpfr_init(r4913);
        mpfr_init_set_str(r4914, "1", 10, MPFR_RNDN);
        mpfr_init(r4915);
        mpfr_init(r4916);
        mpfr_init_set_str(r4917, "2", 10, MPFR_RNDN);
        mpfr_init(r4918);
        mpfr_init(r4919);
        mpfr_init_set_str(r4920, "3", 10, MPFR_RNDN);
        mpfr_init(r4921);
        mpfr_init(r4922);
        mpfr_init(r4923);
        mpfr_init_set_str(r4924, "4", 10, MPFR_RNDN);
        mpfr_init(r4925);
        mpfr_init(r4926);
        mpfr_init(r4927);
        mpfr_init(r4928);
        mpfr_init(r4929);
}

double f_dm(double N) {
        mpfr_set_d(r4913, N, MPFR_RNDN);
        ;
        mpfr_add(r4915, r4913, r4914, MPFR_RNDN);
        mpfr_log(r4916, r4915, MPFR_RNDN);
        ;
        mpfr_mul(r4918, r4917, r4913, MPFR_RNDN);
        mpfr_div(r4919, r4914, r4918, MPFR_RNDN);
        ;
        mpfr_mul(r4921, r4913, r4913, MPFR_RNDN);
        mpfr_mul(r4922, r4920, r4921, MPFR_RNDN);
        mpfr_div(r4923, r4914, r4922, MPFR_RNDN);
        ;
        mpfr_pow(r4925, r4913, r4920, MPFR_RNDN);
        mpfr_div(r4926, r4924, r4925, MPFR_RNDN);
        mpfr_sub(r4927, r4923, r4926, MPFR_RNDN);
        mpfr_sub(r4928, r4919, r4927, MPFR_RNDN);
        mpfr_sub(r4929, r4916, r4928, MPFR_RNDN);
        return mpfr_get_d(r4929, MPFR_RNDN);
}

