/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie          mathreplace.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016 Alex Sanchez-Stern

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
#include "../value-shadowstate/value-shadowstate.h"
#include "realop.h"
#include "error.h"
#include <math.h>

VgHashTable* callToOpInfoMap = NULL;
typedef struct _opInfoEntry {
  struct _opInfoEntry* next;
  UWord call_addr;
  ShadowOpInfo* info;
} OpInfoEntry;

#define NCALLFRAMES 5

Addr getCallAddr(void);
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
    if (VG_(strcmp)(filename, "mathwrap.c") == 0) continue;
    return addr;
  }
  return 0;
}

void performWrappedOp(OpType type, double* resLoc, double* args){
#ifndef USE_MPFR
  tl_assert2(0, "Can't wrap math ops in GMP mode!\n");
#endif
  int nargs = getWrappedNumArgs(type);
  FloatType op_precision = getWrappedPrecision(type);
  ShadowValue** shadowArgs =
    VG_(malloc)("wrapped shadow args", sizeof(ShadowValue*) * nargs);
  for(int i = 0; i < nargs; ++i){
    shadowArgs[i] = getMemShadow((UWord)&(args[i]));
    if (shadowArgs[i] == NULL){
      shadowArgs[i] = mkShadowValue(op_precision, args[i]);
    }
  }
  ShadowValue* shadowResult = runWrappedShadowOp(type, shadowArgs);
  *resLoc = runEmulatedWrappedOp(type, args);
  removeMemShadow((UWord)resLoc);
  addMemShadow((UWord)resLoc, shadowResult);

  Addr callAddr = getCallAddr();
  OpInfoEntry* entry = VG_(HT_lookup)(callToOpInfoMap, callAddr);
  if (entry == NULL){
    ShadowOpInfo* callInfo =
      mkShadowOpInfo(0x0, callAddr, 1, nargs, op_precision);
    callInfo->name = getWrappedName(type);
    updateError(callInfo,
                shadowResult->real,
                *resLoc);
  }
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

FloatType getWrappedPrecision(OpType type){
  switch(type){
  case SINGLE_CASES:
    return Ft_Single;
  case DOUBLE_CASES:
    return Ft_Double;
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
      VG_(printf)("get here.\n");
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
