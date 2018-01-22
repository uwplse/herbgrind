/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie                debug.h ---*/
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

#define addValTypeAssert(sbOut, label, shadow_val_expr, val_type)     \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertValType", VG_(fnptr_to_fnentry)(assertValType), mkIRExprVec_3(mkU64((uintptr_t)label), shadow_val_expr, mkU64(val_type)))))
#define addValTypeAssertG(sbOut, guard, label, shadow_val_expr, val_type) \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(assertValType, mkU64((uintptr_t)label), shadow_val_expr, mkU64(val_type), guard))
#define addTempTypeAssert(sbOut, label, shadow_temp_expr, val_type)     \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertTempType", VG_(fnptr_to_fnentry)(assertTempType), mkIRExprVec_3(mkU64((uintptr_t)label), shadow_temp_expr, mkU64(val_type)))))
#define addTempTypeAssertG(sbOut, guard, label, shadow_temp_expr, val_type) \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(assertTempType, mkU64((uintptr_t)label), shadow_temp_expr, mkU64(val_type), guard))
#define addNumValsAssert(sbOut, label, shadow_expr, num_vals)     \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertNumVals", VG_(fnptr_to_fnentry)(assertNumVals), mkIRExprVec_3(mkU64((uintptr_t)label), shadow_expr, mkU64(num_vals)))))
#define addNumValsAssertNot(sbOut, label, shadow_expr, num_vals)     \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertNumValsNot", VG_(fnptr_to_fnentry)(assertNumValsNot), mkIRExprVec_3(mkU64((uintptr_t)label), shadow_expr, mkU64(num_vals)))))
#define addNumValsAssertG(sbOut, guard, label, shadow_expr, num_vals) \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(assertNumVals, mkU64((uintptr_t)label), shadow_expr, mkU64(num_vals), guard))
#define addNumValsAssertNotG(sbOut, guard, label, shadow_expr, num_vals) \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(assertNumValsNot, mkU64((uintptr_t)label), shadow_expr, mkU64(num_vals), guard))

#define addAssertValValid(sbOut, label, val) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(2, "assertValValid", assertValValid, mkIRExprVec_2(mkU64((uintptr_t)label), val))))

#define addAssertTempValid(sbOut, label, temp) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(2, "assertValValid", assertTempValid, mkIRExprVec_2(mkU64((uintptr_t)label), temp))))
#define addAssertTempValidG(sbOut, guard, label, temp)                  \
  addStmtToIRSB(sbOut, mkDirtyG_0_2(assertTempValid, \
                                    mkU64((uintptr_t)label), \
                                    temp, guard))

#define addAssertDynamicSize(sbOut, label, temp, num_halfwords) \
  addStmtToIRSB(sbOut, mkDirty_0_3(assertDynamicSize,           \
                                   mkU64((uintptr_t)label),     \
                                   temp, mkU64(num_halfwords)))

#define addAssertDynamicSizeG(sbOut, guard, label, temp, num_halfwords)  \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(assertDynamicSize,           \
                                    mkU64((uintptr_t)label),     \
                                    temp, mkU64(num_halfwords),  \
                                    guard))

static inline
void assertNEQ(char* message, int val1, int val2){
  tl_assert2(val1 != val2, "%s: %X vs. %X", message, val1, val2);
}
static inline
void assertEQ(char* message, int val1, int val2){
  tl_assert2(val1 == val2, "%s: %X vs. %X", message, val1, val2);
}
static inline
void assertValOwned(char* message, ShadowValue* val){
  tl_assert2(val == NULL || val->ref_count > 0, "%s: %p is freed!\n",
             message, val);
}

static inline
void assertTempValsOwned(char* message, ShadowTemp* temp){
  for(int i = 0; i < temp->num_vals; ++i){
    assertValOwned(message, temp->values[i]);
  }
}

#define addAssertNEQ(sbOut, message, val1, val2) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertNEQ", \
                                                      VG_(fnptr_to_fnentry)(assertNEQ), mkIRExprVec_3(mkU64((uintptr_t)message), val1, val2))))

#define addAssertNEQG(sbOut, guard, message, val1, val2) \
  addStmtToIRSB(sbOut, mkDirtyG_0_3(assertNEQ, mkU64((uintptr_t)message), val1, val2, guard));

#define addAssertEQ(sbOut, message, val1, val2) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "assertEQ", \
                                                      VG_(fnptr_to_fnentry)(assertEQ), mkIRExprVec_3(mkU64((uintptr_t)message), val1, val2))))
#define addAssertValOwned(sbOut, message, val) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(2, "assertValOwned", \
                                                      VG_(fnptr_to_fnentry)(assertValOwned), mkIRExprVec_2(mkU64((uintptr_t)message), val))))
#define addAssertValOwnedG(sbOut, guard, message, val)                  \
  addStmtToIRSB(sbOut, mkDirtyG_0_2(assertValOwned, mkU64((uintptr_t)message), val, guard))
#define addAssertTempValsOwned(sbOut, message, temp) \
  addStmtToIRSB(sbOut, \
    IRStmt_Dirty(unsafeIRDirty_0_N(2, "assertTempValsOwned",            \
                                   VG_(fnptr_to_fnentry)(assertTempValsOwned), \
                                   mkIRExprVec_2(mkU64((uintptr_t)message), val))))
#define addAssertTempValsOwnedG(sbOut, guard, message, temp)            \
  addStmtToIRSB(sbOut,                                                  \
                mkDirtyG_0_2(assertTempValsOwned,                       \
                             mkU64((uintptr_t)message), val, guard))
#define addPrintFloat(float_expr) \
  addStmtToIRSB(sbOut, mkDirty_0_1(ppFloat_wrapper, float_expr))
#define addPrintFloatG(guard, float_expr)                        \
  addStmtToIRSB(sbOut, mkDirtyG_0_1(ppFloat_wrapper, float_expr, guard))


static inline
void fail(void){
  tl_assert(0);
}
#define addFail(sbOut) \
  addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(0, "fail", VG_(fnptr_to_fnentry)(fail), mkIRExprVec_0())));

/* #define DEBUG */
