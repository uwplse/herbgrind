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

// The shadow value for each logical floating point value
typedef struct _ShadowValue {
  mpfr_t value;
} ShadowValue;

// The value we're tracking for each floating point storage location
// in the program.
typedef struct _ShadowLocation {
  // This member is here to make this structure compatible with the
  // hash table implementation in pub_tool_hashtable. None of our code
  // will actually use it.
  struct _ShadowLocation* next;
  // This part is also here for the hash table structure, but we'll
  // actually be messing with it as we'll set it to the address of any
  // memory location we want to store a shadow value for.
  UWord addr;
  // The actual high precision values shadowing a float. In most cases
  // this should be a pointer to a single value, but in cases where we
  // move, for instance, two 64-bit floats into a 128 bit location, we
  // might need to store multiple shadow values in a single
  // location. The lower indices refer to lower order bits.
  ShadowValue* values;
  // Stores how many shadow values are packed into this shadow
  // location. As described above, this should be one most of the
  // time.
  UWord numValues;
} ShadowLocation;

// When I was looking through the FpDebug source as inspiration for
// this project, I kept seeing these structures all over the place
// that looked like they only were really meant to be used and passed
// around in one place, and I kept wondering why you would use them
// when there are easier ways to pass things around, like usually as
// seperate arguments. Now that I've messed around with valgrind
// enough, I finally understand that these weird structures are used
// to get around the three argument restriction that valgrind puts on
// inserting client calls. It's possible that by the time I'm done
// here, I'll have recreated everything I didn't like about the
// FpDebug code base.
typedef struct _LoadG_Info {
  UWord cond;
  Addr src_mem;
  UWord alt_tmp;
  UWord dest_tmp;
} LoadG_Info;

typedef struct _BinaryOp_Info {
  UWord op;
  // These are the indicies of the temporaries that hold these
  // values. We'll use these indices to index into the shadow value
  // array too.
  UWord arg1_tmp;
  UWord arg2_tmp;
  // If we don't have a shadow value yet for these arguments, we're
  // going to use the existing float values to create one. These float
  // values can be as big as 256 bits, since we account for SIMD
  // locations, so we're going to store them as an array that's
  // malloc'd when we know how big they're going to be.
  UWord* arg1_value;
  UWord* arg2_value;
  // This is where we're putting the result.
  UWord dest_tmp;
} BinaryOp_Info;

// The type of a floating point location. Many locations contain just
// a single float, but SIMD locations can contain multiple floats or
// doubles in a single location.
typedef enum {
  Lt_Float,
  Lt_Floatx2,
  Lt_Floatx4,
  Lt_Double,
  Lt_Doublex2
} LocType;

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
