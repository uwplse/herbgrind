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

void init_runtime(){
  mpfr_set_default_prec(PRECISION);
  globalMemory = VG_(HT_construct)("memory_shadows");
}

void cleanup_runtime(){
  // Free up the MPFR cache.
  mpfr_free_cache();

  // This stuff doesn't work yet. Ideally what we'll do eventually is
  // reference count our shadow locations, so that we can clear them
  // up properly. We can kill temp references at the end of every
  // block, and we'll clear up the thread state and memory at the end
  // of the program in this function.
  /* // Free up the temps */
  /* for (int i = 0; i < MAX_TEMPS; ++i){ */
  /*   if (localTemps[i] != NULL){ */
  /*     for (int j = 0; j < localTemps[i]->numValues; ++j) */
  /*       mpfr_clear(localTemps[i]->values[j].value); */
  /*   } */
  /* } */
  /* // Free up the thread state. */
  /* for (int i = 0; i < MAX_THREADS; ++i){ */
  /*   for (int j = 0; j < MAX_REGISTERS; ++j){ */
  /*     if (threadRegisters[i][j] != NULL){ */
  /*       for (int k = 0; k < threadRegisters[i][j]->numValues; ++k) */
  /*         mpfr_clear(threadRegisters[i][j]->values[k].value); */
  /*     } */
  /*   } */
  /* } */
  /* // Free up the shadowed memory. */
  /* VG_(HT_ResetIter)(globalMemory); */
  /* for (ShadowLocation* next = VG_(HT_Next)(globalMemory); next != NULL; next = VG_(HT_Next)(globalMemory)){ */
  /*   for (int j = 0; j < next->numValues; ++j) */
  /*     mpfr_clear(next->values[j].value); */
  /* } */
}
