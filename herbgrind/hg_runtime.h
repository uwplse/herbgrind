#ifndef _HG_RUNTIME

// This gets us a hash table data structure that's safe to use with
// valgrind, so we can set up a memory map for shadowing values that
// leave our workbench area.
#include "pub_tool_hashtable.h"
// This header gets us the VG_N_THREADS macro, as well as some other
// misc thread data.
#include "pub_tool_threadstate.h"
// Pull in this header file so that we can pass memory allocation
// functions to gmp and mpfr.
#include "pub_tool_mallocfree.h"

// For size_t
#include <stdlib.h>
// For mpfr_t
#include "mpfr.h"

// The maximum number of pre-instrumentation temporary values per
// block we're willing to deal with.
#define	MAX_TEMPS 1000
// The maximum number of per-thread registers we'll account for.
#define	MAX_REGISTERS 1000
#define MAX_THREADS 16

void init_runtime(void);
void cleanup_runtime(void);

// This disables the instrumentation of this tool.
void stopHerbGrind(void);
// This enables the instrumentation of this tool.
void startHerbGrind(void);

// Some memory allocation functions for gmp support
void* gmp_alloc(size_t t);
void* gmp_realloc(void* p, size_t t1, size_t t2);
void gmp_free(void* p, size_t t);

// The functions that we'll insert into the program to move around
// shadow values at run time.
VG_REGPARM(2) void copyShadowTmptoTmp(UWord src_tmp, UWord dest_tmp);
VG_REGPARM(2) void copyShadowTmptoTS(UWord src_tmp, UWord dest_reg);
VG_REGPARM(2) void copyShadowTStoTmp(UWord src_reg, UWord dest_tmp);
VG_REGPARM(2) void copyShadowMemtoTmp(Addr src_mem, UWord dest_tmp);
VG_REGPARM(4) void copyShadowMemtoTmpIf(UWord cond, Addr src_mem, UWord alt_tmp, UWord dest_tmp);
VG_REGPARM(2) void copyShadowTmptoMem(UWord src_tmp, Addr dest_mem);
VG_REGPARM(3) void copyShadowTmptoMemG(UWord cond, UWord src_tmp, Addr dest_mem);

// The value we're tracking for each floating point value in the
// program.
typedef struct _ShadowValue {
  // This member is here to make this structure compatible with the
  // hash table implementation in pub_tool_hashtable. None of our code
  // will actually use it.
  struct _ShadowValue* next;
  // This part is also here for the hash table structure, but we'll
  // actually be messing with it as we'll set it to the address of any
  // memory location we want to store a shadow value for.
  UWord addr;
  // The actual high precision value shadowing a float.
  mpfr_t value;
} ShadowValue;

extern int running;

#endif
