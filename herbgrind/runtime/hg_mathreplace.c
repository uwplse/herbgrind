#include <stdint.h>

#include "pub_tool_basics.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_tooliface.h"

#include "mpfr.h"

#include "hg_mathreplace.h"
#include "hg_storage_runtime.h"
#include "hg_evaluate.h"

void performOp(OpType op, double* result, double* args){
  switch(op){
  case OP_SQRT:
    {
      // We'll need the argument and the result in 64-bit mpfr, and
      // also shadow locations for both. We do the normal calculation
      // in MPFR instead of natively because we can't call the math
      // library here, and where we can it's being wrapped so doing so
      // would result in an infinite loop.
      mpfr_t arg, res;
      ShadowLocation *arg_shadow, *res_shadow;

      // Initialize our 64-bit mpfr arg and shadow, and get the result
      // shadow set up.
      mpfr_init2(arg,64);
      mpfr_init2(res,64);
      res_shadow = mkShadowLocation(Lt_Double);

      // Get the actual argument from the pointer they gave us, and
      // then lookup that address in our shadow hash table to get the
      // shadow argument.
      mpfr_set_d(arg, args[0], MPFR_RNDN);
      arg_shadow = getShadowLocMem((uintptr_t)&(args[0]), args[0]);

      // Perform the square root on both regular and shadow values.
      mpfr_sqrt(res, arg, MPFR_RNDN);
      mpfr_sqrt(res_shadow->values[0].value, arg_shadow->values[0].value, MPFR_RNDN);

      // Set both the regular and shadow results using the pointer we
      // were given to the result memory.
      *result = mpfr_get_d(res, MPFR_RNDN);
      setMem((uintptr_t)result, res_shadow);

      // And finally, evaluate the error of the operation.
      evaluateOpError(&(res_shadow->values[0]), *result);
    }
    break;
  }
}

ShadowLocation* getShadowLocMem(Addr addr, double float_arg){
  ShadowLocation* loc = getMem(addr);
  if (loc != NULL) return loc;

  loc = mkShadowLocation(Lt_Double);
  setMem(addr, loc);

  mpfr_set_d(loc->values[0].value, float_arg, MPFR_RNDN);
  return loc;
}
