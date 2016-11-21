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

#ifdef VG_LITTLEENDIAN
#define ENDIAN Iend_LE
#else
#define ENDIAN Iend_BE;
#endif

#define mkU64(x) IRExpr_Const(IRConst_U64(x))
#define mkLoad64C(dest_temp, src_addr) \
  IRStmt_WrTmp(dest_temp, IRExpr_Load(ENDIAN, Ity_I64, mkU64((uintptr_t)src_addr)))
#define addLoad64C(sbOut, dest_temp, src_addr) \
  addStmtToIRSB(sbOut, mkLoad64C(dest_temp, src_addr))
#define mkStoreC(src_temp, dest_addr) \
  IRStmt_Store(ENDIAN, mkU64((uintptr_t)dest_addr), IRExpr_RdTmp(src_temp))
#define addStoreC(sbOut, src_temp, dest_addr) \
  addStmtToIRSB(sbOut, mkStoreC(src_temp, dest_addr));
#define mkStoreGC(src_temp, dest_addr, guard_temp)                      \
  IRStmt_StoreG(ENDIAN, mkU64((uintptr_t)dest_addr), IRExpr_RdTmp(src_temp), IRExpr_RdTmp(guard_temp))
#define addStoreGC(sbOut, src_temp, dest_addr, guard_temp) \
  addStmtToIRSB(sbOut, mkStoreGC(src_temp, dest_addr, guard_temp))

#define mkNonZeroCheck64(dest_temp, check_temp) \
  IRStmt_WrTmp(dest_temp, IRExpr_Binop(Iop_CmpNE64, IRExpr_RdTmp(check_temp), mkU64(0)))
#define addNonZeroCheck64(sbOut, dest_temp, check_temp) \
  addStmtToIRSB(sbOut, mkNonZeroCheck64(dest_temp, check_temp))
