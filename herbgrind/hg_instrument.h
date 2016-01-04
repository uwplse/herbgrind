#ifndef _HG_INSTRUMENT
#define _HG_INSTRUMENT

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

// This gets us the fnptr_to_fnentry function.
#include "pub_tool_machine.h"

// For arbitrary precision arithmetic. Might have to mess with this a
// bit to get it to work.
#include "mpfr.h"
#include "gmp.h"

// This file has all the functions and data structures that will be
// called by the instrumented program.
#include "hg_runtime.h"

// Some helpful macros to have around. Some of these are from FpDebug.
#define mkU32(_n) IRExpr_Const(IRConst_U32(_n))
#define mkU64(_n) IRExpr_Const(IRConst_U64(_n))

// Instrument a single statement, adding the instrumented statements
// to sbOut.
void instrumentStatement(IRStmt* st, IRSB* sbOut);

// Add instrumenting expressions to sb for an operation, storing the
// result in the temporary at offset.
static void instrumentOp(IRSB* sb, Int offset, IRExpr* expr);

// Create an expression which calculates (base + ((idx + bias) % len)).
static IRExpr* mkArrayLookupExpr(Int base, IRExpr* idx, Int bias, Int len);

#endif
