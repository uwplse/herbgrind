
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie   hg_storage_runtime.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016 Alex Sanchez-Stern

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include <stdint.h>

// The prototypes for the functions we'll implement here.
#include "hg_storage_runtime.h"
// Some helper macros
#include "../include/hg_macros.h"
#include "../include/hg_options.h"
#include "hg_runtime.h"

// This gets us a hash table data structure that's safe to use with
// valgrind, so we can set up a memory map for shadowing values that
// leave our workbench area.
#include "pub_tool_hashtable.h"

// This get's us the line number information so that we can figure out
// if we're in client code.
#include "pub_tool_debuginfo.h"

// For matching object file names
#include "pub_tool_seqmatch.h"

// This header gets us the current running thread.
#include "pub_tool_threadstate.h"

// For asserts, mostly just for debugging.
#include "pub_tool_libcassert.h"

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
//
// This file is in major need of a refactor and rewrite.
static ShadowLocation* localTemps[MAX_TEMPS];
static size_t maxTempUsed = 0;
static VgHashTable* globalMemory = NULL;
static ShadowValue* threadRegisters[MAX_THREADS][MAX_REGISTERS];

// fma and variants are the only libm function I can think of that
// goes over two (float) args. If you change this, you'll also want to
// change the code in setTS.
#define MAX_LIBM_ARGS 3
static ShadowValue* savedArgs[MAX_LIBM_ARGS];

// Copy a shadow value from a temporary to a temporary.
VG_REGPARM(1) void copyShadowTmptoTmp(CpShadow_Info* info){
  ShadowLocation* loc = getTemp(info->src_idx);
  if (!running && loc != NULL) return;

  ShadowLocation* newLoc = NULL;
  copySL(loc, &newLoc);
  setTemp(info->dest_idx, newLoc);

  if (loc != NULL &&
      print_moves){
    VG_(printf)("Copied location ");
    printShadowLoc(getTemp(info->src_idx));
    VG_(printf)(" from temp %lu to temp %lu\n",
                info->src_idx, info->dest_idx);
  }
}

// Copy a shadow value from a temporary to somewhere in the current
// threads state.
VG_REGPARM(1) void copyShadowTmptoTS(CpShadow_Info* info){
  if (!running && getTemp(info->src_idx) != NULL) return;
  setLocTS(info->dest_idx, getTemp(info->src_idx),
           IRTypetoLocType(info->type), info->instr_addr);

  if (getTemp(info->src_idx) != NULL &&
      print_moves){
    VG_(printf)("Copying value ");
    printShadowLoc(getTemp(info->src_idx));
    VG_(printf)(" from temp %lu to thread state %lu\n",
                info->src_idx, info->dest_idx);
  }
}

// Copy a shadow value from somewhere in the thread state to a temporary.
VG_REGPARM(1) void copyShadowTStoTmp(CpShadow_Info* info){
  ShadowLocation* loc;
  loc = getLocTS(info->src_idx, IRTypetoLocType(info->type));
  if (!running && loc != NULL) return;
  setTemp(info->dest_idx, loc);

  if (loc != NULL &&
      print_moves){
    VG_(printf)("Copying value ");
    printShadowLoc(loc);
    VG_(printf)(" from thread state %lu to temp %lu\n",
                info->src_idx, info->dest_idx);
  }
}

// Copy a shadow value from memory to a temporary
VG_REGPARM(1) void copyShadowMemtoTmp(CpShadow_Info* info){
  ShadowLocation* loc;
  if (!running && getMem(info->src_idx) != NULL) return;
  loc = getLocMem(info->src_idx, IRTypetoLocType(info->type));
  setTemp(info->dest_idx, loc);

  if (loc != NULL && print_moves){
    VG_(printf)("Copying value ");
    printShadowLoc(loc);
    VG_(printf)(" from address %p to temp %lu\n",
                (void*)(uintptr_t)info->src_idx, info->dest_idx);
  }
}

// Copy a shadow value from memory to a temporary, only if cond
// evaluates to true. Otherwise, copy the shadow value from another
// temporary, "alt_tmp".
VG_REGPARM(1) void copyShadowMemtoTmpIf(LoadG_Info* info){
  if (info->cond) {
    if (!running && getMem(info->src_mem) != NULL) return;
    ShadowLocation* loc;
    loc = getLocMem(info->src_mem, IRTypetoLocType(info->dest_type));
    setTemp(info->dest_tmp, loc);

    if (getMem(info->src_mem) != NULL && print_moves){
      VG_(printf)("Copying value ");
      printShadowLoc(loc);
      VG_(printf)(" from address %lx to temp %lu\n",
                  info->src_mem, info->dest_tmp);
    }
  } else {
    if (!running && getTemp(info->alt_tmp) != NULL) return;
    setTemp(info->dest_tmp, getTemp(info->alt_tmp));

    if (getTemp(info->alt_tmp) != NULL && print_moves){
      VG_(printf)("Copying value ");
      printShadowLoc(getTemp(info->alt_tmp));
      VG_(printf)(" from temp %lu to temp %lu\n",
                  info->alt_tmp, info->dest_tmp);
    }
  }
}

// Copy a shadow value from a temporary to memory.
VG_REGPARM(1) void copyShadowTmptoMem(CpShadow_Info* info){
  if (!running && getTemp(info->src_idx) != NULL) return;
  setLocMem(info->dest_idx, getTemp(info->src_idx), IRTypetoLocType(info->type));

  if (getTemp(info->src_idx) != NULL && print_moves){
    VG_(printf)("Copying value ");
    printShadowLoc(getTemp(info->src_idx));
    VG_(printf)(" from temp %lu to address %p\n",
                info->src_idx, (void*)(uintptr_t)info->dest_idx);
  }
}

// Copy a shadow value from a temporary to memory, only if cond
// evaluates to true. Otherwise, do nothing.
VG_REGPARM(2) void copyShadowTmptoMemG(UWord cond, CpShadow_Info* info){
  if (!running && getTemp(info->src_idx) != NULL) return;
  if (cond) copyShadowTmptoMem(info);

  if (getTemp(info->src_idx) != NULL &&
      cond && print_moves){
    VG_(printf)("Copying value ");
    printShadowLoc(getTemp(info->src_idx));
    VG_(printf)(" from temp %lu to address %lx\n",
                info->src_idx, info->dest_idx);
  }
}

VG_REGPARM(0) void initBlock(void){
  maxTempUsed = 0;
}

VG_REGPARM(0) void cleanupBlock(void){
  // This runs at the end of every block, no matter how small. For
  // this reason, it's pretty important that it be fast. Even making
  // this loop loop to a MAX_TEMPS of 1000 every time slows down the
  // program by orders of magnitude, which is why I added this
  // maxTempsUsed variable.
  for(size_t i = 0; i <= maxTempUsed; ++i){
    if (localTemps[i] != NULL){
      CHECK_PTR(localTemps[i]);
      freeSL(localTemps[i]);
      localTemps[i] = NULL;
    }
  }
}

void cleanupStorage(void){
  // Clean up the thread state.
  for (int i = 0; i < MAX_THREADS; ++i)
    for (int j = 0; j < MAX_REGISTERS; ++j)
      if (threadRegisters[i][j] != NULL){
        disownSV(threadRegisters[i][j]);
        threadRegisters[i][j] = NULL;
      }
  // Clean up the memory shadowing table
  VG_(HT_ResetIter)(globalMemory);
  for (ShadowValue_ptr* next = VG_(HT_Next)(globalMemory); next != NULL; next = VG_(HT_Next)(globalMemory)){
    disownSV(next->sv);
  }
  VG_(HT_destruct)(globalMemory, VG_(free));
  for (int i = 0; i < MAX_LIBM_ARGS; ++i)
    setSavedArg(i, NULL);
}

void setTemp(Addr index, ShadowLocation* newLocation){
  tl_assert(index >= 0 && index < MAX_TEMPS);
  if (index > maxTempUsed) maxTempUsed = index;
  if (localTemps[index] != NULL){
    if (print_moves)
      VG_(printf)("Overwriting temp at %lu\n", index);
    freeSL(localTemps[index]);
  }
  if (newLocation != NULL){
    for(int i = 0; i < capacity(newLocation->type); i++){
      if (newLocation->values[i] != NULL){
        addRef(newLocation->values[i]);
      }
    }
  }
  localTemps[index] = newLocation;
}

ShadowLocation* getTemp(Addr index){
  return localTemps[index];
}

void setMem(Addr index, ShadowValue* newValue){
  ShadowValue_ptr *newEntry, *existingEntry;
  existingEntry = VG_(HT_lookup)(globalMemory, index);
  // If an entry already exists at that location, then we're going to
  // overwrite it, so clean it up properly.
  if (existingEntry != NULL){
    disownSV(existingEntry->sv);
    VG_(HT_remove)(globalMemory, index);
    VG_(free)(existingEntry);
    if (print_moves)
      VG_(printf)("Overwriting memory shadow at %p\n", (void*)(uintptr_t)index);
  }
  // Now, add our new entry.
  if (newValue != NULL){
    ALLOC(newEntry, "hg.memoryentry.1", 1, sizeof(ShadowValue_ptr));
    copySV(newValue, &(newEntry->sv));
    newEntry->addr = index;
    VG_(HT_add_node)(globalMemory, newEntry);
  }
}

ShadowValue* getMem(Addr index){
  ShadowValue_ptr* entry = VG_(HT_lookup)(globalMemory, index);
  if (entry == NULL) return NULL;
  return entry->sv;
}

void setLocMem(Addr index, ShadowLocation* newLocation, LocType move_type){
  setLoc__(index, newLocation, move_type, setMem);
}
ShadowLocation* getLocMem(Addr index, LocType type){
  return getLoc__(index, getMem, type);
}

void setTS(Addr index, ShadowValue* val){
  if (threadRegisters[VG_(get_running_tid)()][index] != NULL && print_moves)
    VG_(printf)("Overwriting thread state at %lu\n", index);
  tl_assert(VG_(get_running_tid)() > 0 && VG_(get_running_tid)() < MAX_THREADS &&
            index > 0 && index < MAX_REGISTERS);
  copySV(val, &(threadRegisters[VG_(get_running_tid)()][index]));
}

ShadowValue* getTS(Addr index){
  if (!running) return NULL;
  return threadRegisters[VG_(get_running_tid)()][index];
}

void setLocTS(Addr index, ShadowLocation* newLocation, LocType move_type, Addr instr_addr){
  // Okay, so this is a weird bit of code, that fixes a very specific
  // problem. The problem is, the first time we hit a replaced
  // function call, we go through the linker to patch up the
  // connection, and somewhere in there we lose track of the shadow
  // value being passed into a replaced libm function. So, what we're
  // going to do here is store libm arguments in a special savedArg
  // storage location. Then, when libm functions are looking for their
  // arguments and don't find anything, they'll look in savedArg for
  // the arguments. This involves several parts. We need to save every
  // storage into an arg register that could be a libm call. And we
  // need to overwrite it at the right times with NULL, so that libm
  // function calls that get constant arguments don't pull the wrong
  // shadow values. This all happens in this block. First, we match on
  // the thread state locations which arguments are passed in.

  /* // Next, only save stuff that we're going to overwrite. */
  /* if (newLocation == NULL && */
  /*     // The indexes of the thread state that the first, second, and */
  /*     // third arguments to a replaced libm function are passed */
  /*     // in. This is a pretty horrible hack, but I'm working around a */
  /*     // terrible limitation in valgrind. So, maybe that justfies */
  /*     // it. This might not be cross platform either, but as my */
  /*     // advisor says, we'll burn that bridge when we cross it. */
  /*     (index == 224 || index == 256 || index == 288)){ */
  /*   int argIndex; */
  /*   switch(index){ */
  /*   case 224: */
  /*     argIndex = 0; */
  /*     break; */
  /*   case 256: */
  /*     argIndex = 1; */
  /*     break; */
  /*   case 288: */
  /*     argIndex = 2; */
  /*     break; */
  /*   default: */
  /*     return; */
  /*   } */

  /*   const HChar* objname; */
  /*   VG_(get_objname)(instr_addr, &objname); */
  /*   if (!VG_(string_match)("?*ld-?*.so", objname)){ */
  /*     // If not, then it's in user code and is trying to actually */
  /*     // overwrite the location because it's passing an argument */
  /*     // that has not yet determined it's a floating point value, so */
  /*     // doesn't have a shadow value. */
  /*     setSavedArg(argIndex, NULL); */
  /*   } else if (threadRegisters[VG_(get_running_tid)()][index] != NULL){ */
  /*     // If we are in the linker code, and the value that we're */
  /*     // about to overwrite isn't null, then we want to save it in */
  /*     // our saved arg register. */
  /*     setSavedArg(argIndex, threadRegisters[VG_(get_running_tid)()][index]); */
  /*   } */
  /* } */
  // Finally, actually do the overwrite.
  setLoc__(index, newLocation, move_type, setTS);
}

ShadowLocation* getLocTS(Addr index, LocType type){
  return getLoc__(index, getTS, type);
}

void setLoc__(Addr index, ShadowLocation* newLoc, LocType move_type,
              void (*setter)(Addr index, ShadowValue* val)){
  // This potentially exposes a bug in herbgrind, where the move type
  // and location type don't match.
  /* tl_assert2(newLoc == NULL || capacity(move_type) <= capacity(newLoc->type), */
  /*            "Type mismatch! Tried to move shadow location %p to thread state, " */
  /*            "but the move had a type with capacity %lu, " */
  /*            "and the location only has capacity %lu\n", */
  /*            newLoc, capacity(move_type), capacity(newLoc->type)); */
  if (newLoc == NULL || move_type != newLoc->type){
    for (SizeT i = 0; i < capacity(move_type); ++i){
      setter(index + el_size(move_type) * i, NULL);
    }
  } else {
    for (SizeT i = 0; i < capacity(newLoc->type); ++i){
      ShadowValue* val;
      val = newLoc->values[i];
      CHECK_PTR(val);
      setter(index + el_size(newLoc->type) * i, val);
    }
  }
}
ShadowLocation* getLoc__(Addr index, ShadowValue* (*getter)(Addr index), LocType type){
  ShadowLocation* result = mkShadowLocation_bare(type);
  Bool allNull = True;
  for (SizeT i = 0; i < capacity(type); ++i){
    result->values[i] = getter(index + el_size(type) * i);
    if (result->values[i] != NULL){
      CHECK_PTR(result->values[i]);
      allNull = False;
    }
  }
  if (allNull){
    freeSL(result);
    return NULL;
  } else {
    return result;
  }
}

void setSavedArg(Int index, ShadowValue* newValue){
  copySV(newValue, &(savedArgs[index]));
}

ShadowValue* getSavedArg(Int index){
  return savedArgs[index];
}

void initStorage(void){
  globalMemory = VG_(HT_construct)("memory_shadows");
}

// This is super unsound. Let's find out if it's safe.
LocType IRTypetoLocType(IRType ty){
  switch(ty){
  case Ity_I32:
  case Ity_F32:
    return Lt_Float;
  case Ity_I64:
  case Ity_F64:
    return Lt_Double;
  case Ity_V128:
    return Lt_Doublex2;
  case Ity_D128:
  case Ity_F128:
  case Ity_V256:
  case Ity_INVALID:
  case Ity_I1:
  case Ity_I8:
  case Ity_I16:
  case Ity_I128:
  case Ity_F16:
  case Ity_D32:
  case Ity_D64:
  default:
    VG_(dmsg)("Unrecognized IRType!\n");
    return 0;
  }
}
