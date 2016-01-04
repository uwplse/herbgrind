#include "hg_runtime.h"

// Whether or not the tool is currently "turned on".
int running = 0;

// Here are the data structures we set up to hold shadow values. They
// take three forms:
//
// * Values that are being worked on currently are held in temps by
//   VEX, so we have an array of shadow values to match each temp
//   register, up to a limit set in the .h file.
//
// * Values that persist between blocks (I think this is how it
//   works), are held in a per thread data structure by VEX, so we set
//   up another array for every thread to hold those, also up to a
//   limit set in the .h file.
//
// * Finally, values might be written to memory, and then read out
//   later at some arbitrary point. For these, we'll maintain a hash
//   table that maps addresses to shadow values, so we don't have to
//   maintain a vast array of shadow values for all of memory.
static ShadowValue* localTemps[MAX_TEMPS];
static ShadowValue* threadRegisters[MAX_THREADS][MAX_REGISTERS];
static VgHashTable* globalMemory = NULL;

// This disables the instrumentation of this tool.
void stopHerbGrind(void){ running = 0; }

// This enables the instrumentation of this tool.
void startHerbGrind(void){ running = 1; }

// Some memory allocation functions for gmp support
void* gmp_alloc(size_t t){ return VG_(malloc)("hg.gmp_alloc.1", t); }
void* gmp_realloc(void* p, size_t t1, size_t t2){ return VG_(realloc)("hg.gmp_realloc.1", p, t1); }
void gmp_free(void* p, size_t t){ VG_(free)(p); }

void init_runtime(){
  globalMemory = VG_(HT_construct)("memory_shadows");
}

void cleanup_runtime(){
}

// Copy a shadow value from a temporary to a temporary.
VG_REGPARM(2) void copyShadowTmptoTmp(UWord src_tmp, UWord dest_tmp){
  localTemps[dest_tmp] = localTemps[src_tmp];
}

// Copy a shadow value from a temporary to somewhere in the current
// threads state.
VG_REGPARM(2) void copyShadowTmptoTS(UWord src_tmp, UWord dest_reg){
  threadRegisters[VG_(get_running_tid)()][dest_reg] = localTemps[src_tmp];
}

// Copy a shadow value from somewhere in the thread state to a temporary.
VG_REGPARM(2) void copyShadowTStoTmp(UWord src_reg, UWord dest_tmp){
  localTemps[dest_tmp] = threadRegisters[VG_(get_running_tid)()][src_reg];
}

// Copy a shadow value from memory to a temporary
VG_REGPARM(2) void copyShadowMemtoTmp(Addr src_mem, UWord dest_tmp){
  localTemps[dest_tmp] = VG_(HT_lookup)(globalMemory, src_mem);
}

// Copy a shadow value from memory to a temporary, only if cond
// evaluates to true. Otherwise, copy the shadow value from another
// temporary, "alt_tmp".
VG_REGPARM(4) void copyShadowMemtoTmpIf(UWord cond, Addr src_mem, UWord alt_tmp, UWord dest_tmp){
  if (cond) {
    localTemps[dest_tmp] = VG_(HT_lookup)(globalMemory, src_mem);
  } else {
    localTemps[dest_tmp] = localTemps[alt_tmp];
  }
}

// Copy a shadow value from a temporary to memory.
VG_REGPARM(2) void copyShadowTmptoMem(UWord src_tmp, Addr dest_mem){
  ShadowValue* val = localTemps[src_tmp];
  // In all of the above cases, we don't bother checking for null,
  // since it there's no harm in passing a null through. In this case
  // though, and the ones below it, we don't want to bloat the hash
  // table with a bunch of null entries, so we check if the val is
  // null before adding a node. Null indicates that the value shadowed
  // is not believed to be a float.
  if (val != NULL){
    val->addr = dest_mem;
    VG_(HT_add_node)(globalMemory, val);
  }
}

// Copy a shadow value from a temporary to memory, only if cond
// evaluates to true. Otherwise, do nothing.
VG_REGPARM(3) void copyShadowTmptoMemG(UWord cond, UWord src_tmp, Addr dest_mem){
  ShadowValue* val = localTemps[src_tmp];
  if (val != NULL && cond){
    val->addr = dest_mem;
    VG_(HT_add_node)(globalMemory, val);
  }
}
