/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie      instrument-util.h ---*/
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

#ifndef _INSTRUMENT_UTIL_H
#define _INSTRUMENT_UTIL_H

#include <stdint.h>
#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_machine.h"

#ifdef VG_LITTLEENDIAN
#define ENDIAN Iend_LE
#else
#define ENDIAN Iend_BE;
#endif

#define mkU128(x) IRExpr_Const(IRConst_V128(x))
#define mkU64(x) IRExpr_Const(IRConst_U64(x))
#define mkU32(x) IRExpr_Const(IRConst_U32(x))
#define mkU1(x) IRExpr_Const(IRConst_U1(x))

IRTemp runLoad64(IRSB* sbOut, IRExpr* address);
#define runLoad64C(sbOut, addr_const) \
  runLoad64(sbOut, mkU64((uintptr_t)addr_const))
IRTemp runLoadG64(IRSB* sbOut, IRExpr* address, IRExpr* guard);
#define runLoadG64C(sbOut, addr_const, guard_temp)      \
  runLoadG64(sbOut, mkU64((uintptr_t)addr_const), IRExpr_RdTmp(guard_temp))
IRTemp runLoad32(IRSB* sbOut, IRExpr* address);
IRTemp runLoad128(IRSB* sbOut, IRExpr* address);

#define addStore(sbOut, src_expr, dest_addr_expr) \
  addStmtToIRSB(sbOut, IRStmt_Store(ENDIAN, dest_addr_expr, src_expr))
#define addStoreG(sbOut, guard_temp, src_expr, dest_addr_expr)          \
  addStmtToIRSB(sbOut, IRStmt_StoreG(ENDIAN, dest_addr_expr, src_expr, IRExpr_RdTmp(guard_temp)))
#define addStoreC(sbOut, src_expr, dest_addr_const) \
  addStore(sbOut, src_expr, mkU64((uintptr_t)dest_addr_const))
#define addStoreGC(sbOut, guard_temp, src_expr, dest_addr_const)        \
  addStoreG(sbOut, guard_temp, src_expr, mkU64((uintptr_t)dest_addr_const))

IRTemp runDirtyG_1_N(IRSB* sbOut, int nargs, const char* fname, void* f,
                     IRExpr** args, IRExpr* guard_expr);
#define runDirtyG_1_1(sbOut, guard_temp, f, arg_expr) \
  runDirtyG_1_N(sbOut, 1, #f, f, mkIRExprVec_1(arg_expr), \
                IRExpr_RdTmp(guard_temp))
IRStmt* mkDirtyG_0_N(int nargs, const char* fname, void* f,
                     IRExpr** args, IRExpr* guard_expr);
#define mkDirtyG_0_1(f, arg_expr, guard_temp)                           \
  mkDirtyG_0_N(1,#f,f, mkIRExprVec_1(arg_expr), IRExpr_RdTmp(guard_temp))
#define mkDirtyG_0_2(f, arg1_expr, arg2_expr, guard_temp)               \
  mkDirtyG_0_N(2,#f,f, mkIRExprVec_2(arg1_expr, arg2_expr),             \
               IRExpr_RdTmp(guard_temp))
#define mkDirtyG_0_3(f, arg1_expr, arg2_expr, arg3_expr, guard_temp)    \
  mkDirtyG_0_N(3,#f,f, mkIRExprVec_3(arg1_expr, arg2_expr, arg3_expr),  \
               IRExpr_RdTmp(guard_temp))

IRTemp runUnop(IRSB* sbOut, IROp op_code, IRExpr* arg);

#define runUnopT(sbOut, op_code, arg_temp) \
  runUnop(sbOut, op_code, IRExpr_RdTmp(arg_temp))

IRTemp runBinop(IRSB* sbOut, IROp op_code, IRExpr* arg1, IRExpr* arg2);

#define runBinopT(sbOut, op_code, arg1_temp, arg2_temp)           \
  runBinop(sbOut, op_code, IRExpr_RdTmp(arg1_temp), IRExpr_RdTmp(arg2_temp))

IRTemp runOr(IRSB* sbOut, IRTemp arg1_temp, IRTemp arg2_temp);
IRTemp runAnd(IRSB* sbOut, IRTemp arg1_temp, IRTemp arg2_temp);
IRTemp runAndto64(IRSB* sbOut, IRTemp arg1_temp, IRTemp arg2_temp);

#define runNonZeroCheck64(sbOut, check_temp) \
  runBinop(sbOut, Iop_CmpNE64, IRExpr_RdTmp(check_temp), mkU64(0));
#define runZeroCheck64(sbOut, check_temp) \
  runBinop(sbOut, Iop_CmpEQ64, IRExpr_RdTmp(check_temp), mkU64(0));

#define addPrintOp(op_code) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(1, "ppIROp", VG_(fnptr_to_fnentry)(ppIROp), mkIRExprVec_1(mkU64((uintptr_t)op_code)))));

#define addPrint(string) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(1, "print", VG_(fnptr_to_fnentry)(VG_(printf)), mkIRExprVec_1(mkU64((uintptr_t)string)))));
#define addPrintG(guard_temp, string)                                    \
  addStmtToIRSB(sbOut, mkDirtyG_0_1(VG_(fnptr_to_fnentry)(VG_(printf)), mkU64((uintptr_t)string), guard_temp));
#define addPrintGE(guard_expr, string)                                  \
  addStmtToIRSB(sbOut, mkDirtyG_0_N(1, "printf", VG_(fnptr_to_fnentry)(VG_(printf)), mkIRExprVec_1(mkU64((uintptr_t)string)), guard_expr));

#define addPrint2(format, arg)                                          \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(2, "print", VG_(fnptr_to_fnentry)(VG_(printf)), mkIRExprVec_2(mkU64((uintptr_t)format), arg))));
#define addPrintG2(guard_temp, format, arg)                              \
  addStmtToIRSB(sbOut, mkDirtyG_0_2(VG_(fnptr_to_fnentry)(VG_(printf)), mkU64((uintptr_t)format), arg, guard_temp));
#define addPrint3(format, arg1, arg2)                                   \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "print", VG_(fnptr_to_fnentry)(VG_(printf)), mkIRExprVec_3(mkU64((uintptr_t)format), arg1, arg2))));
#define addPrintG3(guard_temp, format, arg1, arg2)                      \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(VG_(fnptr_to_fnentry)(VG_(printf)), mkU64((uintptr_t)format), arg1, arg2, guard_temp));

#define mkConvert(dest_temp, input_temp, conversion)                    \
  unsafeIRDirty_1_N(dest_temp, 1, #conversion, VG_(fnptr_to_fnentry)(conversion), mkIRExprVec_1(IRExpr_RdTmp(input_temp)));

IRTemp runITE(IRSB* sbOut, IRTemp cond_temp,
              IRExpr* true_expr, IRExpr* false_expr);

IRTemp runPureCFunc64(IRSB* sbOut, IRExpr* arg,
                      void* mkFunc, const char* funcName);
IRTemp runPureC2Func64(IRSB* sbOut, IRExpr* arg1, IRExpr* arg2,
                       void* mkFunc, const char* funcName);

#define runArrowAddr(sbOut, struct_temp, struct_type, member)           \
  runBinop(sbOut, Iop_Add64, IRExpr_RdTmp(struct_temp), mkU64(offsetof(struct_type, member)))
#define runArrow(sbOut, struct_temp, struct_type, member)               \
  runLoad64(sbOut, IRExpr_RdTmp(runArrowAddr(sbOut, struct_temp, struct_type, member)))
#define runArrowG(sbOut, guard_temp, struct_temp, struct_type, member)  \
  runLoadG64(sbOut, \
             IRExpr_RdTmp(runArrowAddr(sbOut, struct_temp,              \
                                       struct_type, member)),           \
             IRExpr_RdTmp(guard_temp))
#define addStoreArrow(sbOut, struct_temp, struct_type, \
                      member, new_value_expr)          \
  addStore(sbOut, new_value_expr, \
           IRExpr_RdTmp(runArrowAddr(sbOut, struct_temp, \
                                     struct_type, member)))
#define addStoreArrowG(sbOut, guard_temp, struct_temp, struct_type,     \
                       member, new_value_expr)          \
  addStoreG(sbOut, guard_temp,                                \
            new_value_expr,                                     \
            IRExpr_RdTmp(runArrowAddr(sbOut, struct_temp,       \
                                      struct_type, member)))

#define runIndexAddr(sbOut, array_addr_expr, element_type, index) \
  runBinop(sbOut, Iop_Add64, array_addr_expr, mkU64(sizeof(element_type) \
                                                    * index))
#define runIndex(sbOut, array_addr_expr, element_type, index)   \
  runLoad64(sbOut, IRExpr_RdTmp(runIndexAddr(sbOut, array_addr_expr, \
                                             element_type, index)))
#define runIndexG(sbOut, guard_temp, array_addr_expr,                   \
                  element_type, const_index)                            \
  runLoadG64(sbOut, IRExpr_RdTmp(runIndexAddr(sbOut, array_addr_expr,   \
                                                element_type, \
                                                const_index)),      \
               IRExpr_RdTmp(guard_temp))
#define addStoreIndex(sbOut, array_addr_expr, element_type, \
                      const_index, src_expr) \
  addStore(sbOut, src_expr, \
             IRExpr_RdTmp(runIndexAddr(sbOut,                           \
                                       array_addr_expr,                 \
                                       element_type,                    \
                                       const_index)))

#define addStoreIndexG(sbOut, guard_temp, array_addr_expr, \
                       element_type, const_index, src_expr) \
  addStoreG(sbOut, guard_temp, \
            src_expr,                                   \
            IRExpr_RdTmp(runIndexAddr(sbOut,            \
                                      array_addr_expr,  \
                                      element_type,     \
                                      const_index))) 
IRTemp runPureCCall(IRSB* sbOut, IRCallee* callee, IRType retty,
                      IRExpr** aPureCCall(sbOut

#define runSemanticWiden(sbOut, single_temp) \
  runUnopT(sbOut,                                \
           Iop_ReinterpF64asI64,                 \
           runUnopT(sbOut,                       \
                    Iop_F32toF64,                \
                    runUnopT(sbOut,               \
                             Iop_ReinterpI32asF32,      \
                             single_temp)))

#endif
