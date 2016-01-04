#ifndef _HG_INCLUDE
#define _HG_INCLUDE

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

// For handling client requests. Those are those macros that you can
// put in your code to be instrumented, which will allow you to talk
// to the tool. For instance, HERBGRIND_BEGIN() and HERBGRIND_END()
// will turn the tool on an off for certain sections.
#include "herbgrind.h"
// This file has all the functions and data structures that will be
// called by the instrumented program.
#include "hg_runtime.h"

// For arbitrary precision arithmetic. Might have to mess with this a
// bit to get it to work.
#include "mpfr.h"
#include "gmp.h"

// Pull in this header file so that we can call the valgrind version
// of printf.
#include "pub_tool_libcprint.h"
// Pull in this header file so that we can set the strlen, strcpy,
// memmove, memcmp, and memset functions of mpfr to their valgrind
// library equivalents.
#include "pub_tool_libcbase.h"
// This gets us the fnptr_to_fnentry function.
#include "pub_tool_machine.h"

// Some helpful macros to have around. Some of these are from FpDebug.
#define mkU32(_n) IRExpr_Const(IRConst_U32(_n))
#define mkU64(_n) IRExpr_Const(IRConst_U64(_n))

// This does any initialization that needs to be done after command
// line processing.
static void hg_post_clo_init(void);
// This is where the magic happens. This function gets called to
// instrument every superblock.
static IRSB* hg_instrument ( VgCallbackClosure* closure,
                             IRSB* bb,
                             const VexGuestLayout* layout, 
                             const VexGuestExtents* vge,
                             const VexArchInfo* archinfo_host,
                             IRType gWordTy, IRType hWordTy );
// This is called after the program exits, for cleanup and such.
static void hg_fini(Int exitcode);
// This is called after the program exits, for cleanup and such.
// This handles client requests, the macros that client programs stick
// in to send messages to the tool.
static Bool hg_handle_client_request(ThreadId tid, UWord* arg, UWord* ret);
// This is where we initialize everything
static void hg_pre_clo_init(void);

static void instrumentOpPut(IRSB* sb, Int offset, IRExpr* expr);

// Create an expression which calculates (base + ((idx + bias) % len)).
static IRExpr mkArrayLookupExpr(Int base, IRExpr idx, Int bias, Int len);

#endif
