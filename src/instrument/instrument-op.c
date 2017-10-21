/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie        instrument-op.c ---*/
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

#include "pub_tool_libcassert.h"
#include "pub_tool_machine.h"
#include "pub_tool_xarray.h"

#include "pub_tool_libcprint.h"

#include "instrument-op.h"
#include "instrument-storage.h"
#include "conversion.h"
#include "semantic-op.h"
#include "ownership.h"
#include "../runtime/shadowop/shadowop.h"
#include "../runtime/shadowop/conversions.h"
#include "../runtime/shadowop/exit-float-op.h"
#include "../runtime/value-shadowstate/exprs.h"
#include "../runtime/value-shadowstate/shadowval.h"
#include "../runtime/value-shadowstate/real.h"
#include "../helper/instrument-util.h"
#include "../helper/debug.h"
#include "../options.h"

void instrumentOp(IRSB* sbOut, IRTemp dest, IRExpr* expr,
                  Addr curAddr, Addr blockAddr,
                  int instrIdx){
  // Get the IROp value, the number of args, and the argument
  // expressions out of the structure based on whether it's a Unop, a
  // Binop, or a Triop.
  IROp op_code;
  int nargs;
  IRExpr* argExprs[4];
  switch(expr->tag){
  case Iex_Unop:
    op_code = expr->Iex.Unop.op;
    nargs = 1;
    argExprs[0] = expr->Iex.Unop.arg;
    break;
  case Iex_Binop:
    op_code = expr->Iex.Binop.op;
    switch(op_code){
    case Iop_Sqrt64Fx2:
    case Iop_SqrtF64:
    case Iop_SqrtF32:
    case Iop_RecpExpF64:
    case Iop_RecpExpF32:
    case Iop_SinF64:
    case Iop_CosF64:
    case Iop_TanF64:
    case Iop_2xm1F64:
      nargs = 1;
      argExprs[0] = expr->Iex.Binop.arg2;
      break;
    default:
      nargs = 2;
      argExprs[0] = expr->Iex.Binop.arg1;
      argExprs[1] = expr->Iex.Binop.arg2;
      break;
    }
    break;
  case Iex_Triop:
    op_code = expr->Iex.Triop.details->op;
    nargs = 2;
    argExprs[0] = expr->Iex.Triop.details->arg2;
    argExprs[1] = expr->Iex.Triop.details->arg3;
    break;
  case Iex_Qop:
    op_code = expr->Iex.Qop.details->op;
    nargs = 3;
    argExprs[0] = expr->Iex.Qop.details->arg2;
    argExprs[1] = expr->Iex.Qop.details->arg3;
    argExprs[2] = expr->Iex.Qop.details->arg4;
    break;
  default:
    tl_assert(0);
    return;
  }
  // If the op isn't a float op, dont shadow it.
  if (isSpecialOp(op_code)){
    handleSpecialOp(sbOut, op_code, argExprs, dest,
                    curAddr, blockAddr);
  } else if (isExitFloatOp(op_code)){
    if (mark_on_escape){
      handleExitFloatOp(sbOut, op_code, argExprs, dest,
                        curAddr, blockAddr);
    }
  } else if (isFloatOp(op_code)){
    if (isConversionOp(op_code)){
      instrumentConversion(sbOut, op_code, argExprs, dest,
                           instrIdx);
    } else {
      instrumentSemanticOp(sbOut, op_code, nargs, argExprs,
                           curAddr, blockAddr, dest);
    }
  } else {
    for(int i = 0; i < nargs; ++i){
      if (staticallyFloat(argExprs[i])){
        ppIROp(op_code);
        VG_(printf)(" on ");
        ppIRExpr(argExprs[i]);
        VG_(printf)("\n");
        tl_assert(!staticallyFloat(argExprs[i]));
      }
    }
    addStoreTempNonFloat(sbOut, dest);
  }
}
void handleExitFloatOp(IRSB* sbOut, IROp op_code,
                       IRExpr** argExprs, IRTemp dest,
                       Addr curAddr, Addr blockAddr){
  if (!RUNNING) return;
  tempShadowStatus[dest] = Ss_Unshadowed;
  switch(op_code){
  case Iop_CmpF64:
  case Iop_CmpF32:
  case Iop_CmpLT32F0x4:
  case Iop_CmpLT64F0x2:
  case Iop_CmpLE64F0x2:
  case Iop_CmpUN64F0x2:
  case Iop_CmpUN32F0x4:
    {
      ShadowCmpInfo* info =
        VG_(perm_malloc)(sizeof(ShadowCmpInfo),
                         vg_alignof(ShadowCmpInfo));
      info->op_addr = curAddr;
      info->op_code = op_code;
      info->precision = argPrecision(op_code);

      for(int i = 0; i < 2; ++i){
        addStoreC(sbOut, argExprs[i],
                  (uintptr_t)
                  (argPrecision(op_code) == Vt_Single ?
                   ((void*)computedArgs.argValuesF[i]) :
                   ((void*)computedArgs.argValues[i])));
        if (argExprs[i]->tag == Iex_RdTmp){
          info->argTemps[i] = argExprs[i]->Iex.RdTmp.tmp;
          cleanupAtEndOfBlock(sbOut, info->argTemps[i]);
        } else {
          info->argTemps[i] = -1;
        }
      }
      addStoreC(sbOut, IRExpr_RdTmp(dest), &computedResult);
      cleanupAtEndOfBlock(sbOut, dest);

      IRDirty* dirty =
        unsafeIRDirty_0_N(1, "checkCompare",
                          VG_(fnptr_to_fnentry)(checkCompare),
                          mkIRExprVec_1(mkU64((uintptr_t)
                                              info)));
      dirty->mFx = Ifx_Read;
      dirty->mAddr = mkU64((uintptr_t)&computedArgs);
      dirty->mSize =
        sizeof(computedArgs)
        + sizeof(computedResult)
        + sizeof(shadowTemps);
      addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));

      if (follow_real_execution){
        addStmtToIRSB(sbOut, IRStmt_WrTmp(dest, runLoad64C(sbOut, &computedResult)));
      }
    }
    break;
  case Iop_CmpEQ32Fx2:
  case Iop_CmpGT32Fx2:
  case Iop_CmpGE32Fx2:
  case Iop_CmpEQ32Fx4:
  case Iop_CmpLE32Fx4:
  case Iop_CmpUN32Fx4:
  case Iop_CmpLT32Fx4:
  case Iop_CmpEQ32F0x4:
  case Iop_CmpLE32F0x4:
    addPrintOp(op_code);
    addPrint3("\nArgs are {%f, %f} ",
              runUnop(sbOut, Iop_V128HIto64,
                      argExprs[0]),
              runUnop(sbOut, Iop_V128to64,
                      argExprs[0]));
    addPrint3("and {%f, %f} ",
              runUnop(sbOut, Iop_V128HIto64,
                      argExprs[1]),
              runUnop(sbOut, Iop_V128to64,
                      argExprs[1]));
    addPrint3("Result is {%lX, %lX}\n",
              runUnop(sbOut, Iop_V128HIto64, IRExpr_RdTmp(dest)),
              runUnop(sbOut, Iop_V128to64, IRExpr_RdTmp(dest)));
    addAssertNEQ(sbOut, "done.", mkU64(0), mkU64(0));
    break;
  case Iop_CmpLT64Fx2:
  case Iop_CmpEQ64Fx2:
  case Iop_CmpLE64Fx2:
  case Iop_CmpUN64Fx2:
  case Iop_CmpEQ64F0x2:
    addPrintOp(op_code);
    addPrint3("\nArgs are {%f, %f} ",
              runUnop(sbOut, Iop_V128HIto64,
                      argExprs[0]),
              runUnop(sbOut, Iop_V128to64,
                      argExprs[0]));
    addPrint3("and {%f, %f} ",
              runUnop(sbOut, Iop_V128HIto64,
                      argExprs[1]),
              runUnop(sbOut, Iop_V128to64,
                      argExprs[1]));
    addPrint3("Result is {%lX, %lX}\n",
              runUnop(sbOut, Iop_V128HIto64, IRExpr_RdTmp(dest)),
              runUnop(sbOut, Iop_V128to64, IRExpr_RdTmp(dest)));
    addAssertNEQ(sbOut, "done.", mkU64(0), mkU64(0));
    break;
  case Iop_F64toI16S:
  case Iop_F64toI32S:
  case Iop_F64toI32U:
  case Iop_F64toI64S:
  case Iop_F64toI64U:
  case Iop_F32toI32S:
  case Iop_F32toI32U:
  case Iop_F32toI64S:
  case Iop_F32toI64U:
    {
      ValueType argPrecision;
      switch(op_code){
      case Iop_F64toI16S:
      case Iop_F64toI32S:
      case Iop_F64toI32U:
      case Iop_F64toI64S:
      case Iop_F64toI64U:
        argPrecision = Vt_Double;
        break;
      case Iop_F32toI32S:
      case Iop_F32toI32U:
      case Iop_F32toI64S:
      case Iop_F32toI64U:
      default:
        argPrecision = Vt_Single;
        break;
      }
      int argTemp;
      addStoreC(sbOut, argExprs[1],
                (uintptr_t)
                (argPrecision == Vt_Single ?
                 ((void*)computedArgs.argValuesF[0]) :
                 ((void*)computedArgs.argValues[0])));
      if (argExprs[1]->tag == Iex_RdTmp){
        argTemp = argExprs[1]->Iex.RdTmp.tmp;
        cleanupAtEndOfBlock(sbOut, argTemp);
      } else {
        argTemp = -1;
      }
      addStoreC(sbOut, IRExpr_RdTmp(dest), &computedResult);
      cleanupAtEndOfBlock(sbOut, dest);

      IRDirty* dirty =
        unsafeIRDirty_0_N(2, "checkConvert",
                          VG_(fnptr_to_fnentry)(checkConvert),
                          mkIRExprVec_3(mkU64(argPrecision),
                                        mkU64(argTemp),
                                        mkU64(curAddr)));
      dirty->mFx = Ifx_Read;
      dirty->mAddr = mkU64((uintptr_t)&computedArgs);
      dirty->mSize =
        sizeof(computedArgs)
        + sizeof(computedResult)
        + sizeof(shadowTemps);
      addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
      if (follow_real_execution){
        addStmtToIRSB(sbOut, IRStmt_WrTmp(dest, runLoad64C(sbOut, &computedResult)));
      }
    }
    break;
  default:
    return;
  }
}

void handleSpecialOp(IRSB* sbOut, IROp op_code,
                     IRExpr** argExprs, IRTemp dest,
                     Addr curAddr, Addr block_addr){
  switch(op_code){
  case Iop_I32StoF64:
  case Iop_I64StoF64:
  case Iop_AndV128:
  case Iop_OrV128:
  case Iop_NotV128:
  case Iop_Shr64:
  case Iop_Shl64:
  case Iop_Sar64:
    tempShadowStatus[dest] = Ss_Unknown;
    break;
  case Iop_XorV128:
    instrumentPossibleNegate(sbOut, argExprs, dest, curAddr, block_addr);
    break;
  default:
    tl_assert(0);
    break;
  }
}
