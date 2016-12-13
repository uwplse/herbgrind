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

#include "../helper/instrument-util.h"
#include "../helper/debug.h"

#include "../runtime/value-shadowstate/value-shadowstate.h"
#include "instrument-storage.h"

void instrumentSemanticOp(IRSB* sbOut, IROp op_code,
                          int nargs, IRExpr** argExprs,
                          Addr curAddr, IRTemp dest){
  IRTemp shadowArgs[4];
  for(int i = 0; i < nargs; ++i){
    if (isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[i]))){
      shadowArgs[i] =
        runGetArg(sbOut, argPrecision(op_code), argExprs[i],
                  numChannelsIn(op_code));
    }
  }

  for(int i = 0; i < nargs; ++i){
    if (isFloatType(typeOfIRExpr(sbOut->tyenv, argExprs[i]))){
      if (argExprs[i]->tag == Iex_Const){
        addDisownNonNull(sbOut, shadowArgs[i], numChannelsIn(op_code));
      } else {
        cleanupAtEndOfBlock(sbOut,
                            argExprs[i]->Iex.RdTmp.tmp,
                            numChannelsIn(op_code));
      }
    }
  }
}

IRTemp runGetArg(IRSB* sbOut, FloatType type, IRExpr* argExpr,
                 int num_vals){
  if (argExpr->tag == Iex_Const){
    IRTemp result = runMakeInput(sbOut, argExpr, type);
    return result;
  } else {
    // Iex_RdTmp
    IRTemp loadedArg =
      runLoad64C(sbOut,
                 &(shadowTemps[argExpr->Iex.RdTmp.tmp]));
    IRTemp argNull = runZeroCheck64(sbOut, loadedArg);

    IRTemp freshArg = runMakeInputG(sbOut, argNull, argExpr, type);

    addStoreGC(sbOut, argNull, IRExpr_RdTmp(freshArg), &(shadowTemps[argExpr->Iex.RdTmp.tmp]));

    IRTemp result = runITE(sbOut, argNull,
                           IRExpr_RdTmp(freshArg),
                           IRExpr_RdTmp(loadedArg));
    return result;

  }
}
