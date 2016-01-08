#ifndef _HG_RUNTIME
#define _HG_RUNTIME

#include "pub_tool_tooliface.h"
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
// Pull in this header file so that we can call the valgrind version
// of printf and dmsg.
#include "pub_tool_libcprint.h"

// For evaluating the error between shadow values and non-shadow
// values.
#include "hg_evaluate.h"
#include "hg_types.h"

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
#define PRECISION 1000

extern int running;

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
VG_REGPARM(1) void copyShadowMemtoTmpIf(LoadG_Info* info);
VG_REGPARM(2) void copyShadowTmptoMem(UWord src_tmp, Addr dest_mem);
VG_REGPARM(3) void copyShadowTmptoMemG(UWord cond, UWord src_tmp, Addr dest_mem);

// The functions that we'll insert into the program to execute shadow
// operations alongside the normal operations.
VG_REGPARM(3) void executeUnaryShadowOp(UWord op, UWord* args, UWord dest_tmp);
VG_REGPARM(1) void executeBinaryShadowOp(BinaryOp_Info* opInfo);
VG_REGPARM(3) void executeTriShadowOp(UWord op, UWord* args, UWord dest_tmp);
VG_REGPARM(3) void executeQuadShadowOp(UWord op, UWord* args, UWord dest_tmp);

// Get's the shadow location associated with a particular tmp
// index. If there is no shadow location currently associated with
// that type, constructs one which contains a single shadow value,
// constructed from float_val, assuming float_val is of type 'type'.
ShadowLocation* getShadowLocation(UWord tmp_num, LocType type, UWord* float_val);

// Frees the memory associated with a shadow locatin.
void cleanupShadowLocation(ShadowLocation* loc);

#endif
