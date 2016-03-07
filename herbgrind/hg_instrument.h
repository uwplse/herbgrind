
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie        hg_instrument.h ---*/
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

#ifndef _HG_INSTRUMENT
#define _HG_INSTRUMENT

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

// This gets us the fnptr_to_fnentry function.
#include "pub_tool_machine.h"
// Pull in this header file so that we can call the valgrind version
// of printf and dmsg.
#include "pub_tool_libcprint.h"

// For arbitrary precision arithmetic. Might have to mess with this a
// bit to get it to work.
#include "mpfr.h"
#include "gmp.h"

// This file has all the functions and data structures that will be
// called by the instrumented program.
#include "runtime/hg_runtime.h"

// For pointer ints
#include <stdint.h>

// Some helpful macros to have around. Some of these are from FpDebug.
#define mkU32(_n) IRExpr_Const(IRConst_U32(_n))
#define mkU64(_n) IRExpr_Const(IRConst_U64(_n))

#define addStore(sb, src_expr, dest_addr) \
  addStmtToIRSB(sb, IRStmt_Store(ENDIAN, mkU64((uintptr_t)dest_addr), src_expr));

// Instrument a single statement, adding the instrumented statements
// to sbOut.
void instrumentStatement(IRStmt* st, IRSB* sbOut, Addr stAddr);

// Add instrumenting expressions to sb for an operation, storing the
// result in the temporary at offset.
void instrumentOp(IRSB* sb, Int offset, IRExpr* expr, Addr opAddr);

// Get the plain name of the op, like "subtraction"
const HChar* getPlainOpname(IROp op);
const HChar* getOpSymbol(IROp op);

// Get a temporary number where the given argument expression is
// stored.
IRTemp getArgTmp(IRExpr* arg, IRSB* sbOut);

// Create an expression which calculates (base + ((idx + bias) % len)).
IRExpr* mkArrayLookupExpr(Int base, IRExpr* idx, Int bias, Int len, IRSB* sbOut);

// Finalize the block
void startBlock(IRSB* sbOut);
void finalizeBlock(IRSB* sbOut);

// Determine whether a temp could contain a shadow location using
// VEX's type system.
int isFloat(IRTypeEnv* env, IRTemp temp);
int isFloatType(IRType type);
#endif
