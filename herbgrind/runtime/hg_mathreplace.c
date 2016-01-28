#include "pub_tool_basics.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_tooliface.h"

#include "mpfr.h"
#include "hg_mathreplace.h"

void performOp(OpType op, double* result, double* args){
  switch(op){
  case OP_SQRT:
    {
      mpfr_t arg, res;
      mpfr_init_set_d(arg, args[0], MPFR_RNDN);
      mpfr_init(res);
      mpfr_sqrt(res, arg, MPFR_RNDN);
      *result = mpfr_get_d(res, MPFR_RNDN);
    }
    break;
  }
}
