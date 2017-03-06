/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          semantic-op.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
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

#include "semantic-op.h"

#include "pub_tool_libcassert.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"

#include "../helper/instrument-util.h"
#include "../helper/debug.h"
#include "../helper/ir-info.h"

#include "../runtime/op-shadowstate/shadowop-info.h"
#include "../runtime/shadowop/shadowop.h"

#include "instrument-storage.h"
#include "ownership.h"

void instrumentSemanticOp(IRSB* sbOut, IROp op_code,
                          int nargs, IRExpr** argExprs,
                          Addr curAddr, Addr blockAddr,
                          IRTemp dest){
  if (print_semantic_ops){
    ppIROp(op_code);
    VG_(printf)(" on ");
    for(int i = 0; i < nargs; ++i){
      ppIRExpr(argExprs[i]);
      VG_(printf)(", ");
    }
    VG_(printf)("\n");
    addPrintOp(op_code);
    addPrint("\n");
  }
  IRExpr* args[4];
  for(int i = 0; i < nargs; ++i){
    tl_assert(isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[i])));
    args[i] =
      runGetArg(sbOut, argExprs[i],
                argPrecision(op_code), numChannelsIn(op_code));
  }

  if (print_semantic_ops){
    ppIROp(op_code);
    VG_(printf)("\n");
    addPrintOp(op_code);
    addPrint("\n");
  }

  IRExpr* shadowOutput =
    runShadowOp(sbOut, op_code, curAddr, blockAddr,
                args, nargs, dest);
  if (print_temp_moves){
    addPrint3("Putting result of op, %p, in %d", shadowOutput, mkU64(dest));
    addPrint2(" (with %d values)\n", mkU64(numChannelsOut(op_code)));
  }
  addStoreTemp(sbOut, shadowOutput, argPrecision(op_code), dest);

  for(int i = 0; i < nargs; ++i){
    if (!canStoreShadow(sbOut->tyenv, argExprs[i])){
      addDisownNonNull(sbOut, args[i], numChannelsIn(op_code));
    }
  }
}

IRExpr* runShadowOp(IRSB* sbOut, IROp op_code,
                    Addr curAddr, Addr block_addr,
                    IRExpr** args, int nargs,
                    IRTemp dest){
  ShadowOpInfo* info = mkShadowOpInfo(op_code, curAddr,
                                      block_addr, nargs);
  for(int i = 0; i < nargs; ++i){
    addStoreC(sbOut, args[i], (&shadowArgs[i]));
  }
  addStoreC(sbOut, IRExpr_RdTmp(dest), &computedResult);
  tl_assert(info->op_code < Iop_LAST);
  return runPureCCall64_2(sbOut,
                          executeShadowOp,
                          mkU64((uintptr_t)info),
                          mkU64((uintptr_t)shadowArgs));
}

IRExpr* runGetArg(IRSB* sbOut, IRExpr* argExpr,
                  FloatType type, int num_vals){
  tl_assert2(canBeFloat(sbOut->tyenv, argExpr),
             "Temp %d can't hold a float, "
             "but we're using it as an argument!\n",
             argExpr->Iex.RdTmp.tmp);
  if (!canHaveShadow(sbOut->tyenv, argExpr)) {
    IRExpr* result = runMakeInput(sbOut, argExpr, type, num_vals);
    if (print_temp_moves){
      addPrint3("Making temp %p for non-shadowed (with %d values).\n", result, mkU64(num_vals));
    }
    return result;
  } else {
    IRExpr* loaded =
      runLoadTemp(sbOut, argExpr->Iex.RdTmp.tmp);
    if (hasStaticShadow(argExpr)){
      return loaded;
    } else {
      IRExpr* shouldMake = runZeroCheck64(sbOut, loaded);
      IRExpr* freshArg =
        runMakeInputG(sbOut, shouldMake, argExpr, type, num_vals);

      IRExpr* result = runITE(sbOut, shouldMake, freshArg, loaded);
      IRExpr* shouldntMake = runUnop(sbOut, Iop_Not1, shouldMake);
      if (print_temp_moves){
        addPrintG3(shouldMake, "Making %p in %d",
                   freshArg, mkU64(argExpr->Iex.RdTmp.tmp));
        addPrintG2(shouldMake, " with %d values\n",
                   mkU64(num_vals));
        addPrintG3(shouldntMake, "Loaded %p from %d\n",
                   loaded, mkU64(argExpr->Iex.RdTmp.tmp));
      }
      addNumValsAssertG(sbOut, shouldntMake, "loaded2", result, num_vals);
      return result;
    }
  }
}
