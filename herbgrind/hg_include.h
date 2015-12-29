#ifndef _HG_INCLUDE
#define _HG_INCLUDE

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

// For handling client requests. Those are those macros that you can
// put in your code to be instrumented, which will allow you to talk
// to the tool. For instance, HERBGRIND_BEGIN() and HERBGRIND_END()
// will turn the tool on an off for certain sections.
#include "herbgrind.h"

// For arbitrary precision arithmetic. Might have to mess with this a
// bit to get it to work.
#include "mpfr.h"
#include "gmp.h"

// Pull in this header file so that we can call the valgrind version
// of printf.
#include "pub_tool_libcprint.h"
// Pull in this header file so that we can pass memory allocation
// functions to gmp and mpfr.
#include "pub_tool_mallocfree.h"
// Pull in this header file so that we can set the strlen, strcpy,
// memmove, memcmp, and memset functions of mpfr to their valgrind
// library equivalents.
#include "pub_tool_libcbase.h"
// This header gets us the VG_N_THREADS macro, as well as some other
// misc thread data.
#include "pub_tool_threadstate.h"
// This gets us a hash table data structure that's safe to use with
// valgrind, so we can set up a memory map for shadowing values that
// leave our workbench area.
#include "pub_tool_hashtable.h"

// The maximum number of pre-instrumentation temporary values per
// block we're willing to deal with.
#define	MAX_TEMPS 1000
// The maximum number of per-thread registers we'll account for.
#define	MAX_REGISTERS 1000


// The value we're tracking for each floating point value in the
// program.
typedef struct _ShadowValue {
  mpfr_t value;
} ShadowValue;


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

// This disables the instrumentation of this tool.
static void stopHerbGrind(void);
// This enables the instrumentation of this tool.
static void startHerbGrind(void);

// Some memory allocation functions for gmp support
static void* gmp_alloc(size_t t);
static void* gmp_realloc(void* p, size_t t1, size_t t2);
static void gmp_free(void* p, size_t t);

#endif
