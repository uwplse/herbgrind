#include <stdint.h>

#include "pub_tool_basics.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_mallocfree.h"

#include "mpfr.h"

#include "hg_mathreplace.h"
#include "hg_evaluate.h"
#include "hg_runtime.h"
#include "../types/hg_opinfo.h"
#include "../include/hg_macros.h"

VgHashTable* callToOpInfoMap = NULL;

typedef struct _OpInfo_Entry {
  // This member is here to make this structure compatible with the
  // hash table implementation in pub_tool_hashtable. None of our code
  // will actually use it.
  struct _OpInfo_Entry* next;
  // This part is also here for the hash table structure, but we'll
  // actually be messing with it as we'll set it to the address of the
  // wrapped call. This acts as the key in the hash table.
  UWord call_addr;
  // Finally, a pointer to the actual op info we're storing.
  Op_Info* info;
} OpInfo_Entry;

void performOp(OpType op, double* result, double* args){
  SizeT nargs;
  switch(op){
  case OP_SQRT:
  case OP_EXP:
  case OP_LOG:
  case OP_ABS:
  case OP_EXPM1:
  case OP_LOG1P:
  case OP_CBRT:
  case OP_CBRTF:
  case OP_CEIL:
  case OP_CEILF:
  case OP_COPYSIGN:
  case OP_COPYSIGNF:

  case OP_COS:
  case OP_SIN:
  case OP_TAN:
  case OP_ASIN:
  case OP_ASINF:
  case OP_ACOS:
  case OP_ACOSF:
  case OP_ATAN:
  case OP_ATANF:

  case OP_SINH:
  case OP_COSH:
  case OP_TANH:
  case OP_ASINH:
  case OP_ASINHF:
  case OP_ACOSH:
  case OP_ACOSHF:
  case OP_ATANH:
  case OP_ATANHF:
    nargs = 1;
    break;
  case OP_MOD:
  case OP_POW:
  case OP_ATAN2:
  case OP_ATAN2F:
  case OP_HYPOT:
    nargs = 2;
    break;
  }
  // We'll need the argument and the result in 64-bit mpfr, and
  // also shadow locations for both. We do the normal calculation
  // in MPFR instead of natively because we can't call the math
  // library here, and where we can it's being wrapped so doing so
  // would result in an infinite loop.
  mpfr_t *args_m, res;
  ShadowLocation **arg_shadows, *res_shadow;
  const HChar* plain_opname;
  const HChar* op_symbol;

  // Initialize our 64-bit mpfr arg and shadow, and get the result
  // shadow set up.
  args_m = VG_(malloc)("wrapped-args", nargs * sizeof(mpfr_t));
  arg_shadows = VG_(malloc)("wrapped-shadow", nargs * sizeof(ShadowLocation*));
  for (SizeT i = 0; i < nargs; ++i){
    mpfr_init2(args_m[i], 64);
    // Get the actual value from the pointer they gave us.
    mpfr_set_d(args_m[i], args[i], MPFR_RNDN);
    // Lookup the address in our shadow hash table to get the
    // shadow argument.
    arg_shadows[0] = getShadowLocMem((uintptr_t)&(args[i]), args[i]);
  }
  mpfr_init2(res,64);
  res_shadow = mkShadowLocation(Lt_Double);

  switch(op){
  case OP_SQRT:
  case OP_EXP:
  case OP_LOG:
  case OP_ABS:
  case OP_EXPM1:
  case OP_LOG1P:
  case OP_CBRT:
  case OP_CBRTF:

  case OP_COS:
  case OP_SIN:
  case OP_TAN:
  case OP_ASIN:
  case OP_ASINF:
  case OP_ACOS:
  case OP_ACOSF:
  case OP_ATAN:
  case OP_ATANF:

  case OP_SINH:
  case OP_COSH:
  case OP_TANH:
  case OP_ASINH:
  case OP_ASINHF:
  case OP_ACOSH:
  case OP_ACOSHF:
  case OP_ATANH:
  case OP_ATANHF:
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_rnd_t);
      switch(op){
      case OP_SQRT:
        plain_opname = "square root";
        op_symbol = "sqrt";
        mpfr_func = mpfr_sqrt;
        break;
      case OP_EXP:
        plain_opname = "exponentiate";
        op_symbol = "exp";
        mpfr_func = mpfr_exp;
        break;
      case OP_LOG:
        plain_opname = "log";
        op_symbol = "log";
        mpfr_func = mpfr_log;
        break;
      case OP_ABS:
        plain_opname = "absolute";
        op_symbol = "abs";
        mpfr_func = mpfr_abs;
        break;
      case OP_EXPM1:
        plain_opname = "exponentiate minus one";
        op_symbol = "expm1";
        mpfr_func = mpfr_expm1;
        break;
      case OP_LOG1P:
        plain_opname = "plus 1 log";
        op_symbol = "log1p";
        mpfr_func = mpfr_log1p;
        break;
      case OP_CBRT:
        plain_opname = "cube root";
        op_symbol = "cbrt";
        mpfr_func = mpfr_cbrt;
        break;
      case OP_CBRTF:
        plain_opname = "cube root (float)";
        op_symbol = "cbrtf";
        mpfr_func = mpfr_cbrt;
        break;

      case OP_COS:
        plain_opname = "cosine";
        op_symbol = "cos";
        mpfr_func = mpfr_cos;
        break;
      case OP_SIN:
        plain_opname = "sine";
        op_symbol = "sin";
        mpfr_func = mpfr_sin;
        break;
      case OP_TAN:
        plain_opname = "tangent";
        op_symbol = "tan";
        mpfr_func = mpfr_tan;
        break;
      case OP_ASIN:
        plain_opname = "arcsine";
        op_symbol = "asin";
        mpfr_func = mpfr_asin;
        break;
      case OP_ASINF:
        plain_opname = "arcsine (float)";
        op_symbol = "asinf";
        mpfr_func = mpfr_asin;
        break;
      case OP_ACOS:
        plain_opname = "arccosine";
        op_symbol = "acos";
        mpfr_func = mpfr_acos;
        break;
      case OP_ACOSF:
        plain_opname = "arccosine (float)";
        op_symbol = "acosf";
        mpfr_func = mpfr_acos;
        break;
      case OP_ATAN:
        plain_opname = "arctangent";
        op_symbol = "atan";
        mpfr_func = mpfr_atan;
        break;
      case OP_ATANF:
        plain_opname = "arctangent (float)";
        op_symbol = "atanf";
        mpfr_func = mpfr_atan;
        break;
      case OP_SINH:
        plain_opname = "hyperbolic sine";
        op_symbol = "sinh";
        mpfr_func = mpfr_sinh;
        break;
      case OP_COSH:
        plain_opname = "hyperbolic cosine";
        op_symbol = "cosh";
        mpfr_func = mpfr_cosh;
        break;
      case OP_TANH:
        plain_opname = "hyperbolic tangent";
        op_symbol = "tanh";
        mpfr_func = mpfr_tanh;
        break;
      case OP_ASINH:
        plain_opname = "hyperbolic arc-sine (float)";
        op_symbol = "asinhf";
        mpfr_func = mpfr_asinh;
        break;
      case OP_ASINHF:
        plain_opname = "hyperbolic arc-sine";
        op_symbol = "asinh";
        mpfr_func = mpfr_asinh;
        break;
      case OP_ACOSH:
        plain_opname = "hyperbolic arc-cosine";
        op_symbol = "acosh";
        mpfr_func = mpfr_acosh;
        break;
      case OP_ACOSHF:
        plain_opname = "hyperbolic arc-cosine (float)";
        op_symbol = "acoshf";
        mpfr_func = mpfr_acosh;
        break;
      case OP_ATANH:
        plain_opname = "hyperbolic arc-tangent";
        op_symbol = "atanh";
        mpfr_func = mpfr_atanh;
        break;
      case OP_ATANHF:
        plain_opname = "hyperbolic arc-tangent (float)";
        op_symbol = "atanhf";
        mpfr_func = mpfr_atanh;
        break;
      default:
        return;
      }
      // Perform the operation on both regular and shadow values.
      mpfr_func(res, args_m[0], MPFR_RNDN);
      mpfr_func(res_shadow->values[0].value, arg_shadows[0]->values[0].value, MPFR_RNDN);
    }
    break;
  case OP_CEIL:
  case OP_CEILF:
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr);
      switch(op){
      case OP_CEIL:
        plain_opname = "ceiling";
        op_symbol = "ceilf";
        mpfr_func = mpfr_ceil;
        break;
      case OP_CEILF:
        plain_opname = "ceiling (float)";
        op_symbol = "ceilf";
        mpfr_func = mpfr_ceil;
        break;
      default:
        return;
      }
      // Perform the operation on both regular and shadow values.
      mpfr_func(res, args_m[0]);
      mpfr_func(res_shadow->values[0].value, arg_shadows[0]->values[0].value);
    }
    break;
  case OP_MOD:
  case OP_POW:
  case OP_ATAN2:
  case OP_ATAN2F:
  case OP_HYPOT:
  case OP_COPYSIGN:
  case OP_COPYSIGNF:
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
      switch(op){
      case OP_MOD:
        plain_opname = "modulus";
        op_symbol = "mod";
        mpfr_func = mpfr_fmod;
        break;
      case OP_POW:
        plain_opname = "power";
        op_symbol = "pow";
        mpfr_func = mpfr_pow;
        break;
      case OP_ATAN2:
        plain_opname = "arctangent (two arguments)";
        op_symbol = "atan2";
        mpfr_func = mpfr_atan2;
        break;
      case OP_ATAN2F:
        plain_opname = "arctangent (two arguments, float)";
        op_symbol = "atan2f";
        mpfr_func = mpfr_atan2;
        break;
      case OP_HYPOT:
        plain_opname = "hypotenuse";
        op_symbol = "hypot";
        mpfr_func = mpfr_hypot;
        break;
      case OP_COPYSIGN:
        plain_opname = "copy sign";
        op_symbol = "copysign";
        mpfr_func = mpfr_copysign;
        break;
      case OP_COPYSIGNF:
        plain_opname = "copy sign (float)";
        op_symbol = "copysignf";
        mpfr_func = mpfr_copysign;
        break;
      default:
        return;
      }
      // Perform the operation on both regular and shadow values.
      mpfr_func(res, args_m[0], args_m[1], MPFR_RNDN);
      mpfr_func(res_shadow->values[0].value, arg_shadows[0]->values[0].value, arg_shadows[0]->values[0].value, MPFR_RNDN);
    }
    break;
  }

  // Set both the regular and shadow results using the pointer we
  // were given to the result memory.
  *result = mpfr_get_d(res, MPFR_RNDN);
  setMem((uintptr_t)result, res_shadow);

  // Either look up an existing op info entry for this call site, or
  // create one if one doesn't already exist.
  OpInfo_Entry* entry = VG_(HT_lookup)(callToOpInfoMap, last_abi_addr);
  if (entry == NULL){
    Op_Info* callInfo = mkOp_Info(nargs, 0x0, last_abi_addr, plain_opname, op_symbol);
    ALLOC(entry, "hg.opinfo_entry.1", 1, sizeof(OpInfo_Entry));
    entry->call_addr = last_abi_addr;
    entry->info = callInfo;
    VG_(HT_add_node)(callToOpInfoMap, entry);
  }
  // And finally, evaluate the error of the operation.
  evaluateOpError(&(res_shadow->values[0]), *result, entry->info);

  // And free up the arrays we malloc for variable number of args.
  for (int i = 0; i < nargs; ++i){
    mpfr_clear(args_m[i]);
  }
  mpfr_clear(res);
  VG_(free)(args_m);
  VG_(free)(arg_shadows);
}

ShadowLocation* getShadowLocMem(Addr addr, double float_arg){
  ShadowLocation* loc = getMem(addr);
  if (loc != NULL) return loc;

  loc = mkShadowLocation(Lt_Double);
  setMem(addr, loc);

  mpfr_set_d(loc->values[0].value, float_arg, MPFR_RNDN);
  return loc;
}
