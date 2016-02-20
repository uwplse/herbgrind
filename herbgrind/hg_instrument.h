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
