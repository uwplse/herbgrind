#include <stdint.h>

#include "pub_tool_basics.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_stacktrace.h"
#include "pub_tool_threadstate.h"

#include "mpfr.h"

#include "hg_mathreplace.h"
#include "hg_evaluate.h"
#include "hg_runtime.h"
#include "hg_storage_runtime.h"
#include "../types/hg_opinfo.h"
#include "../types/hg_ast.h"
#include "../include/hg_macros.h"
#include "../include/hg_mathreplace_funcs.h"

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

#define NCALLFRAMES 5

Addr getCallAddr(void){
  Addr trace[NCALLFRAMES];
  UInt nframes = VG_(get_StackTrace)(VG_(get_running_tid)(),
                                     trace, NCALLFRAMES, // Is this right?
                                     NULL, NULL,
                                     0);
  for(int i = 0; i < nframes; ++i){
    Addr addr = trace[i];
    // Basically, this whole block discards addresses which are part
    // of the redirection process or internal to the replacement
    // function, and are "below" the location of the call in the calls
    // stack. Currently it looks like we really only have to look at
    // the second frame up, but screw it, this probably isn't the
    // performance bottleneck, and it might be nice to have the
    // robustness somewhere down the line.
    const HChar* filename;
    if (!VG_(get_filename)(addr, &filename)) continue;
    if (VG_(strcmp)(filename, "hg_mathwrap.c") == 0) continue;
    return addr;
  }
  return 0;
}

void performOp(OpType op, double* result, double* args){
  SizeT nargs;
  const HChar* plain_opname;
  const HChar* op_symbol;

  switch(op){
    // This is a macro defined in include/hg_mathreplace_funcs.h which
    // expands to a bunch of cases like "case OP_SQRT:" which
    // coorespond to the elements of the Op enum which take a single
    // argument.
  UNARY_OPS_CASES
    nargs = 1;
    break;
    // This is a macro defined in include/hg_mathreplace_funcs.h which
    // expands to a bunch of cases like "case OP_MOD:" which
    // coorespond to the elements of the Op enum which take two
    // arguments.
  BINARY_OPS_CASES
    nargs = 2;
    break;
    // This is a macro defined in include/hg_mathreplace_funcs.h which
    // expands to a bunch of cases like "case OP_FMA:" which
    // coorespond to the elements of the Op enum which take three
    // arguments.
  TERNARY_OPS_CASES
    nargs = 3;
    break;
  }

  // Populate the plain_opname and op_symbol fields.
  GET_OP_STATIC_INFO(op)

  // Either look up an existing op info entry for this call site, or
  // create one if one doesn't already exist.
  Addr callAddr = getCallAddr();
  OpInfo_Entry* entry = VG_(HT_lookup)(callToOpInfoMap, callAddr);
  if (entry == NULL){
    Op_Info* callInfo = mkOp_Info(nargs, 0x0, callAddr, plain_opname, op_symbol);
    ALLOC(entry, "hg.opinfo_entry.1", 1, sizeof(OpInfo_Entry));
    entry->call_addr = callAddr;
    entry->info = callInfo;
    VG_(HT_add_node)(callToOpInfoMap, entry);
  }

  // We'll need the argument and the result in 64-bit mpfr, and
  // also shadow locations for both. We do the normal calculation
  // in MPFR instead of natively because we can't call the math
  // library here, and where we can it's being wrapped so doing so
  // would result in an infinite loop.
  mpfr_t *args_m, res;
  ShadowLocation **arg_shadows, *res_shadow;

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
    arg_shadows[i] = getShadowLocMem((uintptr_t)&(args[i]), args[i], i);
  }
  mpfr_init2(res,64);
  res_shadow = mkShadowLocation(Lt_Double);

  switch(op){
    // This expands to the cases for ops which take one argument and
    // whose mpfr functions want a rounding mode.
    UNARY_OPS_ROUND_CASES
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_rnd_t);
      // This is a macro, defined in include/hg_mathreplace_funcs.h,
      // which expands to a big switch statement which fills in the
      // plain_opname, op_symbol, and mpfr_func values for each
      // unary operation which needs a rounding mode argument to
      // it's mpfr_func.
      GET_UNARY_OPS_ROUND_INFO(op)
      // Perform the operation on both regular and shadow values.
      mpfr_func(res, args_m[0], MPFR_RNDN);
      mpfr_func(res_shadow->values[0].value, arg_shadows[0]->values[0].value, MPFR_RNDN);
    }
    break;
    // This expands to a bunch of cases for the ops which take one
    // argument and whose mpfr functions don't need a rounding mode.
  UNARY_OPS_NOROUND_CASES
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr);
      // See above comment on UNARY_OPS_ROUND_INFO_CASES
      GET_UNARY_OPS_NOROUND_INFO(op)
      // Perform the operation on both regular and shadow values.
      mpfr_func(res, args_m[0]);
      mpfr_func(res_shadow->values[0].value, arg_shadows[0]->values[0].value);
    }
    break;
    // This expands to a bunch of cases for operations which take two
    // arguments.
    BINARY_OPS_CASES
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
      // See above comment on UNARY_OPS_ROUND_INFO_CASES
      GET_BINARY_OPS_INFO(op)
      // Perform the operation on both regular and shadow values.
      mpfr_func(res, args_m[0], args_m[1], MPFR_RNDN);
      mpfr_func(res_shadow->values[0].value,
                arg_shadows[0]->values[0].value,
                arg_shadows[1]->values[0].value, MPFR_RNDN);
    }
    break;
    // This expands to a bunch of cases for operations which take two
    // arguments.
    TERNARY_OPS_CASES
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
      // See above comment on UNARY_OPS_ROUND_INFO_CASES
      GET_TERNARY_OPS_INFO(op)
      // Perform the operation on both regular and shadow values.
      mpfr_func(res, args_m[0], args_m[1], args_m[2], MPFR_RNDN);
      mpfr_func(res_shadow->values[0].value,
                arg_shadows[0]->values[0].value,
                arg_shadows[1]->values[0].value,
                arg_shadows[2]->values[0].value,
                MPFR_RNDN);
    }
    break;
  }

  // Set both the regular and shadow results using the pointer we
  // were given to the result memory.
  *result = mpfr_get_d(res, MPFR_RNDN);
  setMem((uintptr_t)result, res_shadow);

  // Set up the ast record of this operation.
  switch(nargs){
  case 1:
    initValueBranchAST(&(res_shadow->values[0]), entry->info, nargs,
                       &(arg_shadows[0]->values[0]));
    break;
  case 2:
    initValueBranchAST(&(res_shadow->values[0]), entry->info, nargs,
                       &(arg_shadows[1]->values[0]),
                       &(arg_shadows[2]->values[0]));
    break;
  case 3:
    initValueBranchAST(&(res_shadow->values[0]), entry->info, nargs,
                       &(arg_shadows[1]->values[0]),
                       &(arg_shadows[2]->values[0]),
                       &(arg_shadows[3]->values[0]));
    break;
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

ShadowLocation* getShadowLocMem(Addr addr, double float_arg, Int argIndex){
  ShadowLocation* loc = getMem(addr);
  if (loc != NULL) return loc;
  if (getSavedArg(argIndex) != NULL){
    return getSavedArg(argIndex);
  }

  loc = mkShadowLocation(Lt_Double);
  setMem(addr, loc);

  mpfr_set_d(loc->values[0].value, float_arg, MPFR_RNDN);
  initValueLeafAST(&(loc->values[0]));
  return loc;
}
