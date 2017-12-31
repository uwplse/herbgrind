/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie          mathreplace.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017 Alex Sanchez-Stern

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include "mathreplace.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_stacktrace.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_libcbase.h"
#include "../../include/mathreplace-funcs.h"
#include "../../helper/runtime-util.h"
#include "../value-shadowstate/value-shadowstate.h"
#include "realop.h"
#include "error.h"
#include "symbolic-op.h"
#include "influence-op.h"
#include "local-op.h"
#include <math.h>
#include <inttypes.h>

#define NCALLFRAMES 5
#define MAX_WRAPPED_ARGS 3

void performWrappedOp(OpType type, double* resLoc, double* args){
#ifndef USE_MPFR
  tl_assert2(0, "Can't wrap math ops in GMP mode!\n");
#endif
  int nargs = getWrappedNumArgs(type);
  ValueType op_precision = getWrappedPrecision(type);
  ShadowValue* shadowArgs[MAX_WRAPPED_ARGS];
  for(int i = 0; i < nargs; ++i){
    shadowArgs[i] = getMemShadow((UWord)&(args[i]));
    if (shadowArgs[i] == NULL){
      shadowArgs[i] = mkShadowValue(op_precision, args[i]);
      addMemShadow((UWord)&(args[i]), shadowArgs[i]);
      // When a shadow value is created, it has a single reference,
      // because it assumes you're going to put it in a temp without
      // bumping it's reference counter. Letting it start at zero
      // references can make sketch things happen. However, adding it
      // to memory adds a reference too, so we get a redundant
      // reference. This removes that.
      disownShadowValue(shadowArgs[i]);
    }
  }
  if (print_inputs){
    for(int i = 0; i < nargs; ++i){
      VG_(printf)("Arg %d is computed as ", i + 1);
      ppFloat(args[i]);
      VG_(printf)(", and is shadowed as ");
      ppFloat(getDouble(shadowArgs[i]->real));
      VG_(printf)("\n");
    }
  }
  ShadowValue* shadowResult = runWrappedShadowOp(type, shadowArgs);
  *resLoc = runEmulatedWrappedOp(type, args);
  removeMemShadow((UWord)(uintptr_t)resLoc);
  addMemShadow((UWord)(uintptr_t)resLoc, shadowResult);

  Addr callAddr = getCallAddr();
  ShadowOpInfo* info = getWrappedOpInfo(callAddr, type, nargs);
  if (print_errors_long || print_errors){
    printOpInfo(info);
    VG_(printf)(":\n");
  }
  double bitsGlobalError =
    updateError(&(info->agg.global_error), shadowResult->real, *resLoc);
  execSymbolicOp(info, &(shadowResult->expr),
                 *resLoc, shadowArgs,
                 bitsGlobalError > error_threshold);
  if (!no_reals){
    execLocalOp(info, shadowResult->real, shadowResult, shadowArgs);
  }
  execInfluencesOp(info, &(shadowResult->influences), shadowArgs);
  if (print_semantic_ops){
    VG_(printf)("%p = %s", shadowResult, getWrappedName(type));
    switch(nargs){
    case 0:
      tl_assert(0);
      return;
    case 1:
      VG_(printf)("(%p)\n", shadowArgs[0]);
      break;
    default:
      VG_(printf)("(%p,", shadowArgs[0]);
      for(int i = 1; i < nargs; ++i){
        VG_(printf)(" %p", shadowArgs[i]);
      }
      VG_(printf)(")\n");
    }
  }
  if (use_ranges){
    updateRanges(info->agg.inputs.range_records, args, nargs);
  }
}

ShadowOpInfo* getWrappedOpInfo(Addr callAddr, OpType opType, int nargs){
  MrOpInfoEntry key = {.call_addr = callAddr, .type = opType};
  MrOpInfoEntry* entry =
    VG_(HT_gen_lookup)(mathreplaceOpInfoMap, &key, cmp_op_entry_by_type);
  if (entry == NULL){
    ShadowOpInfo* callInfo =
      mkShadowOpInfo(0x0, callAddr, callAddr, nargs);
    callInfo->op_type = opType;
    entry = VG_(perm_malloc)(sizeof(MrOpInfoEntry),
                             vg_alignof(MrOpInfoEntry));
    entry->call_addr = callAddr;
    entry->info = callInfo;
    entry->type = opType;
    VG_(HT_add_node)(mathreplaceOpInfoMap, entry);
  }
  return entry->info;
}

int getWrappedNumArgs(OpType type){
  switch(type){
  case UNARY_OPS_CASES:
    return 1;
  case BINARY_OPS_CASES:
    return 2;
  case TERNARY_OPS_CASES:
    return 3;
  default:
    tl_assert(0);
    return 0;
  }
}

ValueType getWrappedPrecision(OpType type){
  switch(type){
  case SINGLE_CASES:
    return Vt_Single;
  case DOUBLE_CASES:
    return Vt_Double;
  default:
    tl_assert(0);
    return 0;
  }
}

const char* getWrappedName(OpType type){
  const char* namevar;
  GET_OP_NAMES(namevar, type);
  return namevar;
}

ShadowValue* runWrappedShadowOp(OpType type, ShadowValue** shadowArgs){
  ShadowValue* result = mkShadowValueBare(getWrappedPrecision(type));
  if (no_reals) return result;
  switch(type){
  case UNARY_OPS_ROUND_CASES:
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_rnd_t);

      GET_UNARY_OPS_ROUND_F(mpfr_func, type);

      mpfr_func(result->real->RVAL,
                shadowArgs[0]->real->RVAL, MPFR_RNDN);
    }
    break;
  case UNARY_OPS_NOROUND_CASES:
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr);
      GET_UNARY_OPS_NOROUND_F(mpfr_func, type);

      mpfr_func(result->real->RVAL, shadowArgs[0]->real->RVAL);
    }
    break;
  case BINARY_OPS_CASES:
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
      GET_BINARY_OPS_F(mpfr_func, type);

      mpfr_func(result->real->RVAL,
                shadowArgs[0]->real->RVAL,
                shadowArgs[1]->real->RVAL, MPFR_RNDN);
    }
    break;
  case TERNARY_OPS_CASES:
    {
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
      GET_TERNARY_OPS_F(mpfr_func, type);

      mpfr_func(result->real->RVAL,
                shadowArgs[0]->real->RVAL,
                shadowArgs[1]->real->RVAL,
                shadowArgs[2]->real->RVAL,
                MPFR_RNDN);
    }
    break;
  default:
    tl_assert(0);
    return NULL;
  }
  return result;
}

double runEmulatedWrappedOp(OpType type, double* args){
  double result;
  RUN(result, type, args);
  return result;
}

Word cmp_op_entry_by_type(const void* node1, const void* node2){
  const MrOpInfoEntry* entry1 = (const MrOpInfoEntry*)node1;
  const MrOpInfoEntry* entry2 = (const MrOpInfoEntry*)node2;
  return !(entry1->type == entry2->type);
}

void performSpecialWrappedOp(SpecialOpType type, double* args,
                             double* res1, double* res2){
#ifndef USE_MPFR
  tl_assert2(0, "Can't wrap math ops in GMP mode!\n");
#endif
  switch(type){
  case OP_SINCOS:
    performWrappedOp(OP_SIN, res1, args);
    performWrappedOp(OP_COS, res2, args);
    break;
  case OP_SINCOSF:
    performWrappedOp(OP_SINF, res1, args);
    performWrappedOp(OP_COSF, res2, args);
    break;
  }
}
