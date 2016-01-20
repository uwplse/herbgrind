#include "hg_runtime.h"

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
ShadowLocation* localTemps[MAX_TEMPS];
VgHashTable* globalMemory = NULL;
static ShadowLocation* threadRegisters[MAX_THREADS][MAX_REGISTERS];

// Copy a shadow value from a temporary to a temporary.
VG_REGPARM(2) void copyShadowTmptoTmp(UWord src_tmp, UWord dest_tmp){
  copySL(localTemps[src_tmp], &localTemps[dest_tmp]);
}

// Copy a shadow value from a temporary to somewhere in the current
// threads state.
VG_REGPARM(2) void copyShadowTmptoTS(UWord src_tmp, UWord dest_reg){
  copySL(localTemps[src_tmp], &threadRegisters[VG_(get_running_tid)()][dest_reg]);
}

// Copy a shadow value from somewhere in the thread state to a temporary.
VG_REGPARM(2) void copyShadowTStoTmp(UWord src_reg, IRType type, UWord dest_tmp){
  ShadowLocation* tsLoc = threadRegisters[VG_(get_running_tid)()][src_reg];
  // If we didn't think this location held a float before, then we
  // don't think that wherever we're assigning to does now.
  if (tsLoc == NULL){
    if (localTemps[dest_tmp] != NULL){
      disownSL(localTemps[dest_tmp]);
      localTemps[dest_tmp] = NULL;
    }
    return;
  }
  // Based on the type, we may have to pull only one of the shadow
  // values out of a multiple-value location.
  switch(type){
  case Ity_I64:
  case Ity_F64:
    switch(tsLoc->type){
    case Lt_Doublex4:
    case Lt_Doublex2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Double);
        tmpLoc->values[0] = *copySV(&tsLoc->values[0]);
      }
      break;
    case Lt_Double:
      copySL(tsLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size thread state get!\n");
      VG_(printf)("(the shadow value had type %d, but we're trying to get a double.)\n", tsLoc->type);
    }
    break;
  case Ity_I32:
  case Ity_F32:
    switch(tsLoc->type){
    case Lt_Floatx8:
    case Lt_Floatx4:
    case Lt_Floatx2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Float);
        tmpLoc->values[0] = *copySV(&tsLoc->values[0]);
      }
      break;
    case Lt_Float:
      copySL(tsLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size thread state get!\n");
      VG_(printf)("(the shadow value had type %d)", tsLoc->type);
      break;
    }
    break;
  case Ity_F128:
  case Ity_V128:
    switch (tsLoc->type){
    case Lt_Doublex2:
    case Lt_Floatx4:
      copySL(tsLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size thread state get!\n");
      VG_(printf)("(the shadow value had type %d, but we're trying to get at 128-bit location.)\n", tsLoc->type);
      break;
    }
    break;
  case Ity_V256:
    switch(tsLoc->type){
    case Lt_Doublex4:
    case Lt_Floatx8:
      copySL(tsLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size thread state get!\n");
      VG_(printf)("(the shadow value had type %d, but we're trying to get a 256-bit location.)\n", tsLoc->type);
      break;
    }
    break;
  default:
    VG_(dmsg)("We don't support that mixed size thread state get!\n");
    VG_(printf)("(we're trying to get a value of type: ");
    ppIRType(type);
    VG_(printf)(")\n");
    break;
  }
}

// Copy a shadow value from memory to a temporary
VG_REGPARM(3) void copyShadowMemtoTmp(Addr src_mem, IRType type, UWord dest_tmp){
  ShadowLocation* memoryLoc = VG_(HT_lookup)(globalMemory, src_mem);
  // If we didn't think this location held a float before, then we
  // don't think that wherever we're assigning to does now.
  if (memoryLoc == NULL) {
    if (localTemps[dest_tmp] != NULL){
      disownSL(localTemps[dest_tmp]);
      localTemps[dest_tmp] = NULL;
    }
    return;
  }
  // Based on the type, we may have to pull only one of the shadow
  // values out of a multiple-value location.
  switch(type){
  case Ity_I64:
  case Ity_F64:
    switch(memoryLoc->type){
    case Lt_Doublex4:
    case Lt_Doublex2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Double);
        tmpLoc->values[0] = *copySV(&memoryLoc->values[0]);
      }
      break;
    case Lt_Double:
      copySL(memoryLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size memory get!\n");
      break;
    }
    break;
  case Ity_I32:
  case Ity_F32:
    switch(memoryLoc->type){
    case Lt_Floatx8:
    case Lt_Floatx4:
    case Lt_Floatx2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Double);
        tmpLoc->values[0] = *copySV(&memoryLoc->values[0]);
      }
      break;
    case Lt_Float:
      copySL(memoryLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size memory get!\n");
      break;
    }
    break;
  case Ity_F128:
  case Ity_V128:
    switch (memoryLoc->type){
    case Lt_Doublex2:
    case Lt_Floatx4:
      copySL(memoryLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size memory get!\n");
      break;
    }
    break;
  case Ity_V256:
    switch(memoryLoc->type){
    case Lt_Doublex4:
    case Lt_Floatx8:
      copySL(memoryLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size memory get!\n");
      break;
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
    copySL(VG_(HT_lookup)(globalMemory, info->src_mem), &localTemps[info->dest_tmp]);;
  } else {
    copySL(localTemps[info->alt_tmp], &localTemps[info->dest_tmp]);
  }
}

// Copy a shadow value from a temporary to memory.
VG_REGPARM(2) void copyShadowTmptoMem(UWord src_tmp, Addr dest_mem){
  ShadowLocation_ptr* lookup_result = VG_(HT_lookup)(globalMemory, dest_mem);
  ShadowLocation_ptr* val = VG_(malloc)("hg.shadowlocptr.1", sizeof(ShadowLocation_ptr));
  // In all of the above cases, we don't bother checking for null,
  // since it there's no harm in passing a null through. In this case
  // though, and the ones below it, we don't want to bloat the hash
  // table with a bunch of null entries, so we check if the val is
  // null before adding a node. Null indicates that the value shadowed
  // is not believed to be a float.
  if (lookup_result != NULL){
    disownSL(lookup_result->sl);
    lookup_result->sl = NULL;
    VG_(free)(lookup_result);
    VG_(HT_remove)(globalMemory, dest_mem);
  }
  if (localTemps[src_tmp] != NULL){
    copySL(localTemps[src_tmp], &(val->sl));
    val->addr = dest_mem;
    VG_(HT_add_node)(globalMemory, val);
  }
}

// Copy a shadow value from a temporary to memory, only if cond
// evaluates to true. Otherwise, do nothing.
VG_REGPARM(3) void copyShadowTmptoMemG(UWord cond, UWord src_tmp, Addr dest_mem){
  if (cond) copyShadowTmptoMem(src_tmp, dest_mem);
}

VG_REGPARM(0) void cleanupBlock(void){
  for(int i = 0; i < MAX_TEMPS; ++i){
    if (localTemps[i] != NULL){
      disownSL(localTemps[i]);
      localTemps[i] = NULL;
    }
  }
}

void cleanup_runtime(void){
  // Clean up the mpfr cache
  mpfr_free_cache();
  // Clean up the thread state.
  for (int i = 0; i < MAX_THREADS; ++i)
    for (int j = 0; j < MAX_REGISTERS; ++j)
      if (threadRegisters[i][j] != NULL){
        disownSL(threadRegisters[i][j]);
        threadRegisters[i][j] = NULL;
      }
  // Clean up the memory shadowing table
  VG_(HT_ResetIter)(globalMemory);
  for (ShadowLocation* next = VG_(HT_Next)(globalMemory); next != NULL; next = VG_(HT_Next)(globalMemory)){
    disownSL(next);
  }
}
