/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie      instrument-util.c ---*/
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

#include "instrument-util.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"

IRTemp runLoad64(IRSB* sbOut, IRExpr* address){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest, IRExpr_Load(ENDIAN, Ity_I64, address)));
  return dest;
}
IRTemp runLoadG64(IRSB* sbOut, IRExpr* address, IRExpr* guard){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  addStmtToIRSB(sbOut, IRStmt_LoadG(ENDIAN, ILGop_Ident64, dest, address, mkU64(0), guard));
  return dest;
}

IRTemp runUnop(IRSB* sbOut, IROp op_code, IRExpr* arg){
  IRType resultType;
  IRType argTypes[4];
  typeOfPrimop(op_code, &resultType, &(argTypes[0]),
               &(argTypes[1]), &(argTypes[2]), &(argTypes[3]));
  IRTemp result = newIRTemp(sbOut->tyenv, resultType);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(result, IRExpr_Unop(op_code, arg)));
  return result;
}
IRTemp runBinop(IRSB* sbOut, IROp op_code, IRExpr* arg1, IRExpr* arg2){
  IRType resultType;
  IRType argTypes[4];
  typeOfPrimop(op_code, &resultType, &(argTypes[0]),
               &(argTypes[1]), &(argTypes[2]), &(argTypes[3]));
  IRTemp result = newIRTemp(sbOut->tyenv, resultType);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(result, IRExpr_Binop(op_code, arg1, arg2)));
  return result;
}

IRTemp runAnd(IRSB* sbOut, IRTemp arg1_temp, IRTemp arg2_temp){
  IRTemp arg1_temp8 = runUnop(sbOut, Iop_1Uto8, IRExpr_RdTmp(arg1_temp));
  IRTemp arg2_temp8 = runUnop(sbOut, Iop_1Uto8, IRExpr_RdTmp(arg2_temp));
  IRTemp dest_temp8 = runBinop(sbOut, Iop_And8,
                               IRExpr_RdTmp(arg1_temp8),
                               IRExpr_RdTmp(arg2_temp8));
  IRTemp dest_temp32 = runUnop(sbOut, Iop_8Uto32,
                               IRExpr_RdTmp(dest_temp8));
  return runUnop(sbOut, Iop_32to1, IRExpr_RdTmp(dest_temp32));
}

IRTemp runOr(IRSB* sbOut, IRTemp arg1_temp, IRTemp arg2_temp){
  IRTemp arg1_temp8 = runUnop(sbOut, Iop_1Uto8, IRExpr_RdTmp(arg1_temp));
  IRTemp arg2_temp8 = runUnop(sbOut, Iop_1Uto8, IRExpr_RdTmp(arg2_temp));
  IRTemp dest_temp8 = runBinop(sbOut, Iop_Or8,
                               IRExpr_RdTmp(arg1_temp8),
                               IRExpr_RdTmp(arg2_temp8));
  IRTemp dest_temp32 = runUnop(sbOut, Iop_8Uto32,
                               IRExpr_RdTmp(dest_temp8));
  return runUnop(sbOut, Iop_32to1, IRExpr_RdTmp(dest_temp32));
}

IRTemp runPureCFunc64(IRSB* sbOut, IRExpr* arg,
                      void* mkFunc, const char* funcName){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(dest,
                             IRExpr_CCall(mkIRCallee(1, funcName,
                                                     VG_(fnptr_to_fnentry)(mkFunc)),
                                          Ity_I64, mkIRExprVec_1(arg))));
  return dest;
}

IRTemp runPureC2Func64(IRSB* sbOut, IRExpr* arg1, IRExpr* arg2,
                       void* mkFunc, const char* funcName){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(dest,
                             IRExpr_CCall(mkIRCallee(2, funcName,
                                                     VG_(fnptr_to_fnentry)(mkFunc)),
                                          Ity_I64, mkIRExprVec_2(arg1,
                                                                 arg2))));
  return dest;
}

IRStmt* mkDirtyG_0_N(int nargs, const char* fname, void* f,
                     IRExpr** args, IRExpr* guard_expr){
  IRDirty* dirty =
    unsafeIRDirty_0_N(nargs, fname, VG_(fnptr_to_fnentry)(f), args);
  dirty->guard = guard_expr;
  return IRStmt_Dirty(dirty);
}

IRTemp runDirtyG_1_N(IRSB* sbOut, int nargs, const char* fname, void* f,
                     IRExpr** args, IRExpr* guard_expr){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  IRDirty* dirty =
    unsafeIRDirty_1_N(dest, nargs, fname, VG_(fnptr_to_fnentry)(f), args);
  dirty->guard = guard_expr;
  addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
  return dest;
}

IRTemp runITE(IRSB* sbOut, IRTemp cond_temp,
              IRExpr* true_expr, IRExpr* false_expr){
  tl_assert(typeOfIRExpr(sbOut->tyenv, true_expr) ==
            typeOfIRExpr(sbOut->tyenv, false_expr));
  IRTemp dest = newIRTemp(sbOut->tyenv,
                          typeOfIRExpr(sbOut->tyenv, true_expr));
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest,
                                    IRExpr_ITE(IRExpr_RdTmp(cond_temp),
                                               true_expr,
                                               false_expr)));
  return dest;
}
