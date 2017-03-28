/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie      instrument-util.c ---*/
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

#include "instrument-util.h"
#include "../runtime/value-shadowstate/value-shadowstate.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"

IRExpr* runLoad64(IRSB* sbOut, IRExpr* address){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest, IRExpr_Load(ENDIAN, Ity_I64, address)));
  return IRExpr_RdTmp(dest);
}
IRExpr* runLoad32(IRSB* sbOut, IRExpr* address){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I32);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest, IRExpr_Load(ENDIAN, Ity_I32, address)));
  return IRExpr_RdTmp(dest);
}
IRExpr* runLoad128(IRSB* sbOut, IRExpr* address){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_V128);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest, IRExpr_Load(ENDIAN, Ity_V128, address)));
  return IRExpr_RdTmp(dest);
}
IRExpr* runLoadG64(IRSB* sbOut, IRExpr* address, IRExpr* guard){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  addStmtToIRSB(sbOut, IRStmt_LoadG(ENDIAN, ILGop_Ident64, dest, address, mkU64(0), guard));
  return IRExpr_RdTmp(dest);
}

IRExpr* runGetC(IRSB* sbOut, IRType type, Int address){
  IRTemp dest = newIRTemp(sbOut->tyenv, type);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest, IRExpr_Get(address, type)));
  return IRExpr_RdTmp(dest);
}

IRExpr* runGetI(IRSB* sbOut,
                IRExpr* varOffset, int constOffset,
                Int arrayBase, Int numElems, IRType elemType){
  IRTemp dest = newIRTemp(sbOut->tyenv, elemType);
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(dest,
                             IRExpr_GetI(mkIRRegArray(arrayBase,
                                                      elemType,
                                                      numElems),
                                         varOffset, constOffset)));
  return IRExpr_RdTmp(dest);
}

IRExpr* runUnop(IRSB* sbOut, IROp op_code, IRExpr* arg){
  IRType resultType;
  IRType argTypes[4];
  typeOfPrimop(op_code, &resultType, &(argTypes[0]),
               &(argTypes[1]), &(argTypes[2]), &(argTypes[3]));
  IRTemp result = newIRTemp(sbOut->tyenv, resultType);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(result, IRExpr_Unop(op_code, arg)));
  return IRExpr_RdTmp(result);
}
IRExpr* runBinop(IRSB* sbOut, IROp op_code, IRExpr* arg1, IRExpr* arg2){
  IRType resultType;
  IRType argTypes[4];
  typeOfPrimop(op_code, &resultType, &(argTypes[0]),
               &(argTypes[1]), &(argTypes[2]), &(argTypes[3]));
  IRTemp result = newIRTemp(sbOut->tyenv, resultType);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(result, IRExpr_Binop(op_code, arg1, arg2)));
  return IRExpr_RdTmp(result);
}

IRExpr* runAnd(IRSB* sbOut, IRExpr* arg1, IRExpr* arg2){
  IRExpr* arg1_32 = runUnop(sbOut, Iop_1Uto32, arg1);
  IRExpr* arg2_32 = runUnop(sbOut, Iop_1Uto32, arg2);
  IRExpr* dest_32 = runBinop(sbOut, Iop_And32,
                             arg1_32,
                             arg2_32);
  return runUnop(sbOut, Iop_32to1, dest_32);
}
IRExpr* runAndto64(IRSB* sbOut, IRExpr* arg1, IRExpr* arg2){
  IRExpr* arg1_64 = runUnop(sbOut, Iop_1Uto64, arg1);
  IRExpr* arg2_64 = runUnop(sbOut, Iop_1Uto64, arg2);
  return runBinop(sbOut, Iop_And64, arg1_64, arg2_64);
}

IRExpr* runOr(IRSB* sbOut, IRExpr* arg1, IRExpr* arg2){
  IRExpr* arg1_32 = runUnop(sbOut, Iop_1Uto32, arg1);
  IRExpr* arg2_32 = runUnop(sbOut, Iop_1Uto32, arg2);
  IRExpr* dest_32 = runBinop(sbOut, Iop_Or32, arg1_32, arg2_32);
  return runUnop(sbOut, Iop_32to1, dest_32);
}

IRStmt* mkDirtyG_0_N(int nargs, const char* fname, void* f,
                     IRExpr** args, IRExpr* guard_expr){
  IRDirty* dirty =
    unsafeIRDirty_0_N(nargs, fname, VG_(fnptr_to_fnentry)(f), args);
  dirty->guard = guard_expr;
  return IRStmt_Dirty(dirty);
}

IRExpr* runDirtyG_1_N(IRSB* sbOut, int nargs, const char* fname, void* f,
                     IRExpr** args, IRExpr* guard_expr){
  IRTemp dest = newIRTemp(sbOut->tyenv, Ity_I64);
  IRDirty* dirty =
    unsafeIRDirty_1_N(dest, nargs, fname, VG_(fnptr_to_fnentry)(f), args);
  dirty->guard = guard_expr;
  addStmtToIRSB(sbOut, IRStmt_Dirty(dirty));
  return IRExpr_RdTmp(dest);
}

IRExpr* runPureCCall(IRSB* sbOut, IRCallee* callee, IRType retty,
                     IRExpr** args){
  IRTemp dest = newIRTemp(sbOut->tyenv, retty);
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest,
                                    IRExpr_CCall(callee, retty, args)));
  return IRExpr_RdTmp(dest);
}

IRExpr* runITE(IRSB* sbOut, IRExpr* cond,
               IRExpr* true_expr, IRExpr* false_expr){
  tl_assert(typeOfIRExpr(sbOut->tyenv, true_expr) ==
            typeOfIRExpr(sbOut->tyenv, false_expr));
  IRTemp dest = newIRTemp(sbOut->tyenv,
                          typeOfIRExpr(sbOut->tyenv, true_expr));
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest,
                                    IRExpr_ITE(cond,
                                               true_expr,
                                               false_expr)));
  return IRExpr_RdTmp(dest);
}
#define FSTRINGSIZE 1024
void addPrintStore(IRSB* sbOut, IRExpr* val, const char* format, ...){
  char* buffer = VG_(perm_malloc)(sizeof(char) * FSTRINGSIZE,
                                  vg_alignof(char));
  va_list vl;
  va_start(vl, format);
  VG_(vsnprintf)(buffer, FSTRINGSIZE, format, vl);
  va_end(vl);
  IRExpr* stExists = runNonZeroCheck64(sbOut, val);
  addPrintG3(stExists, "%s = %p\n", mkU64((uintptr_t)buffer), val);
}
void addPrintStoreAlways(IRSB* sbOut, IRExpr* val, const char* format, ...){
  char* buffer = VG_(perm_malloc)(sizeof(char) * FSTRINGSIZE,
                                  vg_alignof(char));
  va_list vl;
  va_start(vl, format);
  VG_(vsnprintf)(buffer, FSTRINGSIZE, format, vl);
  va_end(vl);
  addPrint3("%s = %p\n", mkU64((uintptr_t)buffer), val);
}
void addPrintStoreValue(IRSB* sbOut, IRExpr* val, const char* format, ...){
  char* buffer = VG_(perm_malloc)(sizeof(char) * FSTRINGSIZE,
                                  vg_alignof(char));
  va_list vl;
  va_start(vl, format);
  VG_(vsnprintf)(buffer, FSTRINGSIZE, format, vl);
  va_end(vl);
  IRExpr* stExists = runNonZeroCheck64(sbOut, val);
  addStmtToIRSB(sbOut, mkDirtyG_0_2(printStoreValue, mkU64((uintptr_t)buffer), val, stExists));
}

void addPrintStoreValueF1(IRSB* sbOut, IRExpr* val, const char* format, IRExpr* pval){
  IRExpr* stExists = runNonZeroCheck64(sbOut, val);
  addStmtToIRSB(sbOut, mkDirtyG_0_3(printStoreValueF, val, mkU64((uintptr_t)format), pval, stExists));
}
