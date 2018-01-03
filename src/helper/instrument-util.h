/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie      instrument-util.h ---*/
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

#ifndef _INSTRUMENT_UTIL_H
#define _INSTRUMENT_UTIL_H

#include <stdint.h>
#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_machine.h"
#include "runtime-util.h"

#ifdef VG_LITTLEENDIAN
#define ENDIAN Iend_LE
#else
#define ENDIAN Iend_BE
#endif

#define mkU128(x) IRExpr_Const(IRConst_V128(x))
#define mkU64(x) IRExpr_Const(IRConst_U64(x))
#define mkU32(x) IRExpr_Const(IRConst_U32(x))
#define mkU1(x) IRExpr_Const(IRConst_U1(x))

IRExpr* runLoad64(IRSB* sbOut, IRExpr* address);
#define runLoad64C(sbOut, addr_const) \
  runLoad64(sbOut, mkU64((uintptr_t)addr_const))
IRExpr* runLoadG64(IRSB* sbOut, IRExpr* address, IRExpr* guard);
#define runLoadG64C(sbOut, addr_const, guard)      \
  runLoadG64(sbOut, mkU64((uintptr_t)addr_const), guard)
IRExpr* runLoad32(IRSB* sbOut, IRExpr* address);
IRExpr* runLoad128(IRSB* sbOut, IRExpr* address);

#define addStore(sbOut, src_expr, dest_addr) \
  addStmtToIRSB(sbOut, IRStmt_Store(ENDIAN, dest_addr, src_expr))
#define addStoreG(sbOut, guard, src_expr, dest_addr)          \
  addStmtToIRSB(sbOut, IRStmt_StoreG(ENDIAN, dest_addr, src_expr, guard))
#define addStoreC(sbOut, src_expr, dest_addr_const) \
  addStore(sbOut, src_expr, mkU64((uintptr_t)dest_addr_const))
#define addStoreGC(sbOut, guard, src_expr, dest_addr_const)        \
  addStoreG(sbOut, guard, src_expr, mkU64((uintptr_t)dest_addr_const))

IRExpr* runGetC(IRSB* sbOut, IRType type, Int addr_const);
IRExpr* runGetI(IRSB* sbOut,
                IRExpr* varOffset, int constOffset,
                Int arrayBase, Int numElems, IRType elemType);
#define runGet64C(sbOut, addr_const)            \
  runGetC(sbOut, Ity_I64, addr_const)
#define runGet32C(sbOut, addr_const)            \
  runGetC(sbOut, Ity_I32, addr_const)
#define runGet256C(sbOut, addr_const)                                 \
  runGetC(sbOut, Ity_V256, addr_const)
#define runGetI64(sbOut, varOffset, constOffset, arrayBase, numElems) \
  runGetI(sbOut, varOffset, constOffset, arrayBase, numElems, Ity_I64)
#define runGetI32(sbOut, varOffset, constOffset, arrayBase, numElems) \
  runGetI(sbOut, varOffset, constOffset, arrayBase, numElems, Ity_I32)

IRExpr* runDirtyG_1_N(IRSB* sbOut, int nargs, const char* fname, void* f,
                     IRExpr** args, IRExpr* guard);
#define runDirtyG_1_0(sbOut, guard, f)                          \
  runDirtyG_1_N(sbOut, 0, #f, f, mkIRExprVec_0(), guard);
#define runDirtyG_1_1(sbOut, guard, f, arg)                     \
  runDirtyG_1_N(sbOut, 1, #f, f, mkIRExprVec_1(arg), guard)
#define runDirtyG_1_2(sbOut, guard, f, arg1, arg2)              \
  runDirtyG_1_N(sbOut, 2, #f, f, mkIRExprVec_2(arg1, arg2), guard)
#define runDirtyG_1_3(sbOut, guard, f, arg1, arg2, arg3)                \
  runDirtyG_1_N(sbOut, 3, #f, f, mkIRExprVec_3(arg1, arg2, arg3), guard)
IRStmt* mkDirtyG_0_N(int nargs, const char* fname, void* f,
                     IRExpr** args, IRExpr* guard);
#define mkDirtyG_0_1(f, arg, guard)                           \
  mkDirtyG_0_N(1,#f,f, mkIRExprVec_1(arg), guard)
#define mkDirtyG_0_2(f, arg1, arg2, guard)               \
  mkDirtyG_0_N(2,#f,f, mkIRExprVec_2(arg1, arg2), guard)
#define mkDirtyG_0_3(f, arg1, arg2, arg3, guard)    \
  mkDirtyG_0_N(3,#f,f, mkIRExprVec_3(arg1, arg2, arg3), guard)

#define mkDirty_0_N(nargs, f, args)             \
  IRStmt_Dirty(unsafeIRDirty_0_N(nargs, #f, VG_(fnptr_to_fnentry)(f), args))
#define mkDirty_0_1(f, arg)                     \
  mkDirty_0_N(1, f, mkIRExprVec_1(arg))
#define mkDirty_0_2(f, arg1, arg2)              \
  mkDirty_0_N(2, f, mkIRExprVec_2(arg1, arg2))
#define mkDirty_0_3(f, arg1, arg2, arg3)        \
  mkDirty_0_N(3, f, mkIRExprVec_3(arg1, arg2, arg3))

IRExpr* runUnop(IRSB* sbOut, IROp op_code, IRExpr* arg);

IRExpr* runBinop(IRSB* sbOut, IROp op_code, IRExpr* arg1, IRExpr* arg2);

IRExpr* runOr(IRSB* sbOut, IRExpr* arg1, IRExpr* arg2);
IRExpr* runAnd(IRSB* sbOut, IRExpr* arg1, IRExpr* arg2);
IRExpr* runAndto64(IRSB* sbOut, IRExpr* arg1, IRExpr* arg2);

#define runNonZeroCheck64(sbOut, check) \
  runBinop(sbOut, Iop_CmpNE64, check, mkU64(0))
#define runZeroCheck64(sbOut, check) \
  runBinop(sbOut, Iop_CmpEQ64, check, mkU64(0))

#define addPrintOp(op_code) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(1, "ppIROp", VG_(fnptr_to_fnentry)(ppIROp), mkIRExprVec_1(mkU64((uintptr_t)op_code)))))
#define addPrintOpG(guard, op_code)                                     \
  addStmtToIRSB(sbOut, mkDirtyG_0_1(VG_(fnptr_to_fnentry)(ppIROp), mkU64((uintptr_t)op_code), guard))
#define addPrintTy(ty_code) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(1, "ppIRType", VG_(fnptr_to_fnentry)(ppIRType), mkIRExprVec_1(mkU64((uintptr_t)ty_code)))))
#define addPrintExpr(expr) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(1, "ppIRExpr", VG_(fnptr_to_fnentry)(ppIRExpr), mkIRExprVec_1(mkU64((uintptr_t)expr)))))
#define addPrintFt(floattype)                                           \
  addStmtToIRSB(sbOut, mkDirty_0_1(ppFloatType, mkU64(floattype)))
#define addPrintFtG(guard, floattype)                                    \
  addStmtToIRSB(sbOut, mkDirtyG_0_1(ppFloatType, mkU64(floattype), guard))

#define addPrintFloat(float_expr) \
  addStmtToIRSB(sbOut, mkDirty_0_1(ppFloat_wrapper, float_expr))
#define addPrintFloatG(guard, float_expr)                        \
  addStmtToIRSB(sbOut, mkDirtyG_0_1(ppFloat_wrapper, float_expr, guard))

#define addPrint(string) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(1, "print", VG_(fnptr_to_fnentry)(VG_(printf)), mkIRExprVec_1(mkU64((uintptr_t)string)))))
#define addPrintG(guard, string)                                    \
  addStmtToIRSB(sbOut, mkDirtyG_0_1(VG_(fnptr_to_fnentry)(VG_(printf)), mkU64((uintptr_t)string), guard))

#define addPrint2(format, arg)                                          \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(2, "print", VG_(fnptr_to_fnentry)(VG_(printf)), mkIRExprVec_2(mkU64((uintptr_t)format), arg))))
#define addPrintG2(guard, format, arg)                              \
  addStmtToIRSB(sbOut, mkDirtyG_0_2(VG_(fnptr_to_fnentry)(VG_(printf)), mkU64((uintptr_t)format), arg, guard))
#define addPrint3(format, arg1, arg2)                                   \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "print", VG_(fnptr_to_fnentry)(VG_(printf)), mkIRExprVec_3(mkU64((uintptr_t)format), arg1, arg2))))
#define addPrintG3(guard, format, arg1, arg2)                      \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(VG_(fnptr_to_fnentry)(VG_(printf)), mkU64((uintptr_t)format), arg1, arg2, guard))

#define mkConvert(dest, input, conversion)                    \
  unsafeIRDirty_1_N(dest, 1, #conversion, VG_(fnptr_to_fnentry)(conversion), mkIRExprVec_1(input))

IRExpr* runITE(IRSB* sbOut, IRExpr* cond,
               IRExpr* true_branch, IRExpr* false_branch);

IRExpr* runPureCCall(IRSB* sbOut, IRCallee* callee, IRType retty,
                     IRExpr** args);
#define runPureCCall64(sbOut, f, arg)                                   \
  runPureCCall(sbOut,                                                   \
               mkIRCallee(1, #f, VG_(fnptr_to_fnentry)(f)),             \
               Ity_I64,                                                 \
               mkIRExprVec_1(arg))
#define runPureCCall64_2(sbOut, f, arg1, arg2)                  \
  runPureCCall(sbOut,                                           \
               mkIRCallee(2, #f, VG_(fnptr_to_fnentry)(f)),     \
               Ity_I64,                                         \
               mkIRExprVec_2(arg1, arg2))
#define runPureCCall64_3(sbOut, f, arg1, arg2, arg3)            \
  runPureCCall(sbOut,                                           \
               mkIRCallee(3, #f, VG_(fnptr_to_fnentry)(f)),     \
               Ity_I64,                                         \
               mkIRExprVec_3(arg1, arg2, arg3))
#define runArrowAddr(sbOut, struct_expr, struct_type, member)           \
  runBinop(sbOut, Iop_Add64, struct_expr, mkU64(offsetof(struct_type, member)))
#define runArrow(sbOut, struct_expr, struct_type, member)               \
  runLoad64(sbOut, runArrowAddr(sbOut, struct_expr, struct_type, member))
#define runArrowG(sbOut, guard, struct_expr, struct_type, member)  \
  runLoadG64(sbOut, \
             runArrowAddr(sbOut, struct_expr,              \
                          struct_type, member),           \
             guard)
#define addStoreArrow(sbOut, struct_expr, struct_type, \
                      member, new_value)          \
  addStore(sbOut, new_value, \
           runArrowAddr(sbOut, struct_expr, \
                        struct_type, member))
#define addStoreArrowG(sbOut, guard, struct_expr, struct_type,  \
                       member, new_value)                       \
  addStoreG(sbOut, guard,                                       \
            new_value,                                          \
            runArrowAddr(sbOut, struct_expr,                    \
                         struct_type, member))

#define runIndexAddr(sbOut, array_addr, element_type, index)        \
  runBinop(sbOut, Iop_Add64, array_addr, mkU64(sizeof(element_type) \
                                               * index))
#define runIndex(sbOut, array_addr, element_type, index)        \
  runLoad64(sbOut, runIndexAddr(sbOut, array_addr,              \
                                element_type, index))
#define runIndexG(sbOut, guard, array_addr,                             \
                  element_type, const_index)                            \
  runLoadG64(sbOut, runIndexAddr(sbOut, array_addr,                     \
                                 element_type,                          \
                                 const_index),                          \
             guard)
#define addStoreIndex(sbOut, array_addr, element_type,                  \
                      const_index, src_expr)                            \
  addStore(sbOut, src_expr,                                             \
           runIndexAddr(sbOut, array_addr, element_type, const_index))

#define addStoreIndexG(sbOut, guard, array_addr,            \
                       element_type, const_index, src_expr) \
  addStoreG(sbOut, guard,                                   \
            src_expr,                                       \
            runIndexAddr(sbOut,                             \
                         array_addr,                       \
                         element_type,                      \
                         const_index))
#define runF32toF64(sbOut, f32)                                 \
  runUnop(sbOut, Iop_ReinterpF64asI64,                          \
          runUnop(sbOut, Iop_F32toF64,                          \
                  runUnop(sbOut, Iop_ReinterpI32asF32, f32)))
#define runV128to1st32(sbOut, v)                                        \
  runF32toF64(sbOut,                                                    \
              runUnop(sbOut, Iop_64HIto32,                              \
                      runUnop(sbOut, Iop_V128HIto64, v)))
#define runV128to2nd32(sbOut, v)                        \
  runF32toF64(sbOut,                                    \
              runUnop(sbOut, Iop_64to32,                \
                      runUnop(sbOut, Iop_V128HIto64, v)))
#define runV128to3rd32(sbOut, v)                                        \
  runF32toF64(sbOut,                                                    \
              runUnop(sbOut, Iop_64HIto32,                              \
                      runUnop(sbOut, Iop_V128to64, v)))
#define runV128to4th32(sbOut, v)                        \
  runF32toF64(sbOut,                                    \
              runUnop(sbOut, Iop_64to32,                \
                      runUnop(sbOut, Iop_V128to64, v)))
void addPrintStore(IRSB* sbOut, IRExpr* val, const char* format, ...);
void addPrintStoreAlways(IRSB* sbOut, IRExpr* val, const char* format, ...);
void addPrintStoreValue(IRSB* sbOut, IRExpr* val, const char* format, ...);
void addPrintStoreValueF1(IRSB* sbOut, IRExpr* val, const char* format, IRExpr* pval);

#define runMod(sbOut, x, y)                             \
  runUnop(sbOut, Iop_32Uto64,                           \
          runUnop(sbOut, Iop_64HIto32,                  \
                  runBinop(sbOut, Iop_DivModU64to32,    \
                           x, y)))
#endif
