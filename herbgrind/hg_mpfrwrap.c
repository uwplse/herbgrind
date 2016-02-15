#include "pub_tool_clreq.h"
#include "pub_tool_redir.h"

#include "include/herbgrind.h"
#include "mpfr.h"

double VG_REPLACE_FUNCTION_ZU(NONE, mpfr_set_default_prec)(mpfr_prec_t prec);
double VG_REPLACE_FUNCTION_ZU(NONE, mpfr_set_default_prec)(mpfr_prec_t prec){
}
