#include "hg_runtime.h"

// Whether or not the tool is currently "turned on".
int running = 0;

// This disables the instrumentation of this tool.
void stopHerbGrind(void){ running = 0; }

// This enables the instrumentation of this tool.
void startHerbGrind(void){ running = 1; }

// Some memory allocation functions for gmp support
void* gmp_alloc(size_t t){ return VG_(malloc)("hg.gmp_alloc.1", t); }
void* gmp_realloc(void* p, size_t t1, size_t t2){ return VG_(realloc)("hg.gmp_realloc.1", p, t2); }
void gmp_free(void* p, size_t t){ VG_(free)(p); }

void init_runtime(void){
  mpfr_set_default_prec(PRECISION);
  initStorage();
}
void cleanup_runtime(void){
  // Clean up the mpfr cache
  mpfr_free_cache();
  cleanupStorage();
}
