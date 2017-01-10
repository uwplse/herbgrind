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

#include "../runtime/op-shadowstate/shadowop-info.h"
#include "../runtime/shadowop/shadowop.h"

#include "instrument-storage.h"

void instrumentSemanticOp(IRSB* sbOut, IROp op_code,
                          int nargs, IRExpr** argExprs,
                          Addr curAddr, IRTemp dest){
  IRExpr* args[4];
  for(int i = 0; i < nargs; ++i){
    if (isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[i]))){
      args[i] =
        runGetArg(sbOut, argExprs[i],
                  argPrecision(op_code), numChannelsIn(op_code));
    }
  }

  IRExpr* shadowOutput =
    runShadowOp(sbOut, op_code, curAddr, args, nargs);
  addStoreTemp(sbOut, shadowOutput, argPrecision(op_code),
               dest, typeOfIRTemp(sbOut->tyenv, dest));

  for(int i = 0; i < nargs; ++i){
    if (isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[i]))){
      if (argExprs[i]->tag == Iex_Const){
        addDisownNonNull(sbOut, args[i], numChannelsIn(op_code));
      }
    }
  }
}

IRExpr* runShadowOp(IRSB* sbOut, IROp op_code,
                    Addr curAddr,
                    IRExpr** args, int nargs){
  ShadowOpInfo* info =
    VG_(perm_malloc)(sizeof(ShadowOpInfo), vg_alignof(ShadowOpInfo));
  info->op_code = op_code;
  info->op_addr = curAddr;
  info->exinfo.numSIMDOperands = numSIMDOperands(op_code);
  info->exinfo.numChannels = numChannelsOut(op_code);
  info->exinfo.nargs = nargs;
  info->exinfo.argPrecision = argPrecision(op_code);
  for(int i = 0; i < nargs; ++i){
    addStoreC(sbOut, args[i], (&shadowArgs[i]));
  }
  return runPureCCall(sbOut,
                      mkIRCallee(2, "executeShadowOp",
                                 VG_(fnptr_to_fnentry)(executeShadowOp)),
                      Ity_I64,
                      mkIRExprVec_2(mkU64((uintptr_t)info),
                                    mkU64((uintptr_t)&shadowArgs)));
}

IRExpr* runGetArg(IRSB* sbOut, IRExpr* argExpr,
                  FloatType type, int num_vals){
  if (argExpr->tag == Iex_Const) { // TODO !canHaveShadow
    IRExpr* result = runMakeInput(sbOut, argExpr, type, num_vals);
    if (print_moves){
      addPrint2("Making temp %p for constant.\n", result);
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
      if (print_moves){
        addPrintG3(shouldMake, "Making %p in %d\n",
                   freshArg, mkU64(argExpr->Iex.RdTmp.tmp));
        addPrintG3(shouldntMake, "Loaded %p from %d\n",
                   loaded, mkU64(argExpr->Iex.RdTmp.tmp));
      }
      addNumValsAssertG(sbOut, shouldntMake, "loaded2", result, num_vals);

      return result;
    }
  }
}
