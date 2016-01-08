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
static ShadowLocation* localTemps[MAX_TEMPS];
static ShadowLocation* threadRegisters[MAX_THREADS][MAX_REGISTERS];
static VgHashTable* globalMemory = NULL;

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
VG_REGPARM(2) void copyShadowTStoTmp(UWord src_reg, IRType type, UWord dest_tmp){
  ShadowLocation* tsLoc = threadRegisters[VG_(get_running_tid)()][src_reg];
  // If we didn't think this location held a float before, then we
  // don't think that wherever we're assigning to does now.
  if (tsLoc == NULL){
    localTemps[dest_tmp] = NULL;
    return;
  }
  // Based on the type, we may have to pull only one of the shadow
  // values out of a multiple-value location.
  switch(type){
  case Ity_I64:
    switch(tsLoc->type){
    case Lt_Doublex2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Double);
        tmpLoc->values[0] = tsLoc->values[0];
      }
      break;
    case Lt_Double:
    case Lt_Floatx2:
      localTemps[dest_tmp] = tsLoc;
    default:
      //TODO: Fill in these cases
      VG_(dmsg)("We don't support that mixed size thread state get!\n");
    }
    break;
  default:
    VG_(dmsg)("We don't support that mixed size thread state get!\n");
    break;
  }
}

// Copy a shadow value from memory to a temporary
VG_REGPARM(3) void copyShadowMemtoTmp(Addr src_mem, IRType type, UWord dest_tmp){
  ShadowLocation* memoryLoc = VG_(HT_lookup)(globalMemory, src_mem);
  // If we didn't think this location held a float before, then we
  // don't think that wherever we're assigning to does now.
  if (memoryLoc == NULL) {
    localTemps[dest_tmp] = NULL;
    return;
  }
  // Based on the type, we may have to pull only one of the shadow
  // values out of a multiple-value location.
  switch(type){
  case Ity_I64:
    switch(memoryLoc->type){
    case Lt_Doublex2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Double);
        tmpLoc->values[0] = memoryLoc->values[0];
      }
      break;
    case Lt_Double:
    case Lt_Floatx2:
      localTemps[dest_tmp] = memoryLoc;
    default:
      // TODO: Fill in these cases
      VG_(dmsg)("We don't support that mixed size memory get!\n");
    }
    break;
  default:
    VG_(dmsg)("We don't support that mixed size memory get!\n");
    break;
  }
}

// Copy a shadow value from memory to a temporary, only if cond
// evaluates to true. Otherwise, copy the shadow value from another
// temporary, "alt_tmp".
VG_REGPARM(1) void copyShadowMemtoTmpIf(LoadG_Info* info){
  if (info->cond) {
    localTemps[info->dest_tmp] = VG_(HT_lookup)(globalMemory, info->src_mem);
  } else {
    localTemps[info->dest_tmp] = localTemps[info->alt_tmp];
  }
}

// Copy a shadow value from a temporary to memory.
VG_REGPARM(2) void copyShadowTmptoMem(UWord src_tmp, Addr dest_mem){
  ShadowLocation* val = localTemps[src_tmp];
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
  ShadowLocation* val = localTemps[src_tmp];
  if (val != NULL && cond){
    val->addr = dest_mem;
    VG_(HT_add_node)(globalMemory, val);
  }
}
// Execute a shadow operation, storing the result of the high
// precision operation applied to the shadow value at the arg_tmp
// offset, into the shadow value at the dest_tmp offset. Depending on
// the value of op, we'll know which operation to apply. op will be
// mapped to operations by the enumeration at libvex_ir.h:415.
VG_REGPARM(3) void executeUnaryShadowOp(UWord op, UWord* args, UWord dest_tmp){
}
VG_REGPARM(1) void executeBinaryShadowOp(BinaryOp_Info* opInfo){
  // The shadowing locations for the first argument, second argument,
  // and destination respectively.
  ShadowLocation* arg1Location;
  ShadowLocation* arg2Location;
  ShadowLocation* destLocation;
  switch(opInfo->op){
  case Iop_Add64F0x2:
    // Pull the shadow values for the arguments. If we don't already
    // have shadow values for these arguments, we'll generate fresh
    // ones from the runtime float values.
    arg1Location =
      getShadowLocation(opInfo->arg1_tmp, Lt_Doublex2, opInfo->arg1_value);
    arg2Location =
      getShadowLocation(opInfo->arg2_tmp, Lt_Doublex2, opInfo->arg2_value);

    // Now we'll allocate memory for the shadowed result of this
    // operation, which is a 128-bit SIMD value. The high order
    // 64-bits are taken from the first argument, while the low order
    // 64-bits are the result of the operation.
    destLocation = mkShadowLocation(Lt_Doublex2);

    // Copy across the high order bits shadow value.
    destLocation->values[1] = arg1Location->values[1];

    // Set the low order bits to the result of the addition, but in
    // higher precision.
    mpfr_init(destLocation->values[0].value);
    mpfr_add(destLocation->values[0].value, arg1Location->values[0].value,
             arg2Location->values[0].value, MPFR_RNDN);

    // Put the resulting location in the space for the dest temp.
    localTemps[opInfo->dest_tmp] = destLocation;

    // Now, we'll evaluate the low order shadow value against the low
    // order 64-bits of the result value.
    evaluateOpError(&(destLocation->values[0]), ((double*)opInfo->dest_value)[0]);
    break;
  case Iop_SetV128lo64:
    // Pull the shadow values for the arguments. If we don't already
    // have shadow values for these arguments, we'll generate fresh
    // ones from the runtime float values.
    arg1Location =
      getShadowLocation(opInfo->arg1_tmp, Lt_Doublex2, opInfo->arg1_value);
    arg2Location =
      getShadowLocation(opInfo->arg2_tmp, Lt_Doublex2, opInfo->arg2_value);

    // Now we'll allocate memory for the shadowed result of this
    // operation, which is a 128-bit SIMD value. The high order
    // 64-bits are taken from the first argument, while the low order
    // 64-bits are the result of the operation.
    destLocation = mkShadowLocation(Lt_Doublex2);

    // Copy across the high order bits shadow value from the first
    // argument.
    destLocation->values[1] = arg1Location->values[1];

    // Copy the low order bits shadow value from the second argument.
    destLocation->values[0] = arg2Location->values[0];

    // Put the resulting location in the space for the dest temp.
    localTemps[opInfo->dest_tmp] = destLocation;

    // This isn't really a "real" op in the math-y sense, so let's not
    // evaluate it's error.
    break;
  default:
    break;
  }
}
VG_REGPARM(3) void executeTriShadowOp(UWord op, UWord* args, UWord dest_tmp){
}
VG_REGPARM(3) void executeQuadShadowOp(UWord op, UWord* args, UWord dest_tmp){
}

ShadowLocation* getShadowLocation(UWord tmp_num, LocType type, UWord* float_vals){
  // If we already have a shadow location here, just return it.
  ShadowLocation* location = localTemps[tmp_num];
  if (location != NULL) return location;

  // Otherwise we need to create a new one. How we do this will depend
  // on the expected type of the location, passed as "type".
  location = mkShadowLocation(type);
  switch(type){
  case Lt_Doublex2:
    // Intialize the shadow values from the float_vals we were
    // given.
    //
    // The casts here are actually super important. Without casting
    // the array, we'll pull out the bits that make up the double as
    // basically an int, and then do a semantic conversion of that int
    // to a double, instead of reinterpreting the bytes as they should
    // have always been interpreted, as double bytes.
    mpfr_init_set_d(location->values[0].value, ((double*)float_vals)[0], MPFR_RNDN);
    mpfr_init_set_d(location->values[1].value, ((double*)float_vals)[1], MPFR_RNDN);
    return location;
  default:
    VG_(dmsg)("We don't know how to initialize shadow locations of that type!");
    return NULL;
  }
}
