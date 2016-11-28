/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie              helpers.h ---*/
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

#include <stdint.h>
#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "../runtime/value-shadowstate/shadowval.h"

#ifdef VG_LITTLEENDIAN
#define ENDIAN Iend_LE
#else
#define ENDIAN Iend_BE;
#endif

#define mkU64(x) IRExpr_Const(IRConst_U64(x))
#define mkU32(x) IRExpr_Const(IRConst_U32(x))
#define mkLoad64C(dest_temp, src_addr) \
  IRStmt_WrTmp(dest_temp, IRExpr_Load(ENDIAN, Ity_I64, mkU64((uintptr_t)src_addr)))
#define mkLoad64E(dest_temp, src_expr)
#define addLoad64C(sbOut, dest_temp, src_addr) \
  addStmtToIRSB(sbOut, mkLoad64C(dest_temp, src_addr))
#define mkStoreC(src_temp, dest_addr) \
  IRStmt_Store(ENDIAN, mkU64((uintptr_t)dest_addr), IRExpr_RdTmp(src_temp))
#define addStoreC(sbOut, src_temp, dest_addr) \
  addStmtToIRSB(sbOut, mkStoreC(src_temp, dest_addr));
#define addStoreE(sbOut, src_expr, dest_addr) \
  addStmtToIRSB(sbOut, IRStmt_Store(ENDIAN, mkU64((uintptr_t)dest_addr), src_expr))
#define mkStoreGE(src_expr, dest_addr, guard_temp)                       \
  IRStmt_StoreG(ENDIAN, mkU64((uintptr_t)dest_addr), src_expr, IRExpr_RdTmp(guard_temp))
#define addStoreGE(sbOut, src_expr, dest_addr, guard_temp)              \
  addStmtToIRSB(sbOut, IRStmt_StoreG(ENDIAN, mkU64((uintptr_t)dest_addr), src_expr, IRExpr_RdTmp(guard_temp)))
#define mkStoreGC(src_temp, dest_addr, guard_temp)                      \
  mkStoreGE(IRExpr_RdTmp(src_temp), dest_addr, guard_temp)
#define addStoreGC(sbOut, src_temp, dest_addr, guard_temp) \
  addStmtToIRSB(sbOut, mkStoreGC(src_temp, dest_addr, guard_temp))

#define mkNonZeroCheck64(dest_temp, check_temp) \
  IRStmt_WrTmp(dest_temp, IRExpr_Binop(Iop_CmpNE64, IRExpr_RdTmp(check_temp), mkU64(0)))
#define addNonZeroCheck64(sbOut, dest_temp, check_temp) \
  addStmtToIRSB(sbOut, mkNonZeroCheck64(dest_temp, check_temp))
#define mkZeroCheck64(dest_temp, check_temp) \
  IRStmt_WrTmp(dest_temp, IRExpr_Binop(Iop_CmpEQ64, IRExpr_RdTmp(check_temp), mkU64(0)))
#define addZeroCheck64(sbOut, dest_temp, check_temp) \
  addStmtToIRSB(sbOut, mkZeroCheck64(dest_temp, check_temp))

#define addGetX(sbOut, dest_temp, arg, mkFunc)                 \
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest_temp, IRExpr_CCall(mkIRCallee(1, #mkFunc, VG_(fnptr_to_fnentry)(mkFunc)), Ity_I64, mkIRExprVec_1(arg))))

IRStmt* mkGetXG(IRTemp dest_temp,
                IRExpr* arg, IRExpr* guard,
                void* getFunc, const char* getFuncLabel);

#define addGetXG(sbOut, dest_temp, arg, getFunc, guard_temp)  \
  addStmtToIRSB(sbOut, mkGetXG(dest_temp, arg, IRExpr_RdTmp(guard_temp), getFunc, #getFunc))

#define addGetLeaf(sbOut, dest_temp, arg)                \
  addGetX(sbOut, dest_temp, arg, mkLeafExpr)

#define addGetLeafF(sbOut, dest_temp, arg)               \
  addGetX(sbOut, dest_temp, arg, mkLeafExprF)

#define addGetReal(sbOut, dest_temp, arg)        \
  addGetX(sbOut, dest_temp, arg, mkReal)

#define addGetRealF(sbOut, dest_temp, arg)       \
  addGetX(sbOut, dest_temp, arg, mkRealF)

#define addUnop(sbOut, op, dest_temp, arg) \
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest_temp, IRExpr_Unop(op, arg)))
#define addBinop(sbOut, op, dest_temp, arg1, arg2)                   \
  addStmtToIRSB(sbOut, IRStmt_WrTmp(dest_temp, IRExpr_Binop(op, arg1, arg2)))

#define mkConvert(dest_temp, input_temp, conversion)       \
  unsafeIRDirty_1_N(dest_temp, 1, #conversion, VG_(fnptr_to_fnentry)(conversion), mkIRExprVec_1(IRExpr_RdTmp(input_temp)));

void addOr(IRSB* sbOut, IRTemp dest_temp,
           IRTemp arg1_temp, IRTemp arg2_temp);
void addAnd(IRSB* sbOut, IRTemp dest_temp,
            IRTemp arg1_temp, IRTemp arg2_temp);
void addMakeInputG(IRSB* sbOut, IRTemp dest_temp,
                   IRExpr* argExpr, IRTemp guard,
                   FloatType type);

VG_REGPARM(1) ShadowTemp* mkShadowTempOneDouble(double val);
VG_REGPARM(1) ShadowTemp* mkShadowTempOneSingle(double val);
VG_REGPARM(1) ShadowTemp* mkShadowTempFourSingles(float* vals);
VG_REGPARM(1) ShadowTemp* mkShadowTempTwoDoubles(double* vals);
