/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                debug.h ---*/
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
#include "pub_tool_libcassert.h"

#define addNumValsAssert(sbOut, label, shadow_temp, num_vals)     \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertNumVals", VG_(fnptr_to_fnentry)(assertNumVals), mkIRExprVec_3(mkU64((uintptr_t)label), shadow_temp, mkU64(num_vals)))))
#define addNumValsAssertNot(sbOut, label, shadow_temp, num_vals)     \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertNumValsNot", VG_(fnptr_to_fnentry)(assertNumValsNot), mkIRExprVec_3(mkU64((uintptr_t)label), IRExpr_RdTmp(shadow_temp), mkU64(num_vals)))))
#define addNumValsAssertG(sbOut, guard_temp, label, shadow_temp, num_vals) \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(assertNumVals, mkU64((uintptr_t)label), IRExpr_RdTmp(shadow_temp), mkU64(num_vals), guard_temp))

static inline
void assertNEQ(char* message, int val1, int val2){
  tl_assert2(val1 != val2, message);
}

#define addAssertNEQ(sbOut, message, val1, val2) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertNEQ", \
                                                      VG_(fnptr_to_fnentry)(assertNEQ), mkIRExprVec_3(mkU64((uintptr_t)message), val1, val2))))

static inline
void fail(void){
  tl_assert(0);
}
#define addFail(sbOut) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(0, "fail", VG_(fnptr_to_fnentry)(fail), mkIRExprVec_0())));

