
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
static ShadowLocation* threadRegisters[MAX_THREADS][MAX_REGISTERS];

// fma and variants are the only libm function I can think of that
// goes over two (float) args. If you change this, you'll also want to
// change the code in setTS.
#define MAX_LIBM_ARGS 3
static ShadowLocation* savedArgs[MAX_LIBM_ARGS];

// Copy a shadow value from a temporary to a temporary.
VG_REGPARM(2) void copyShadowTmptoTmp(UWord src_tmp, UWord dest_tmp){
  if (!running && localTemps[src_tmp] != NULL) return;
  if (dest_tmp > maxTempUsed) maxTempUsed = dest_tmp;
  copySL(localTemps[src_tmp], &localTemps[dest_tmp]);

  if (localTemps[src_tmp] != NULL && print_moves){
    mpfr_exp_t shadowValexpt;
    char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                      localTemps[src_tmp]->values[0].value, MPFR_RNDN);
    VG_(printf)("Copying value %se%ld from temp %lu to temp %lu\n",
                shadowValstr, shadowValexpt, src_tmp, dest_tmp);
    mpfr_free_str(shadowValstr);
  }
}

// Copy a shadow value from a temporary to somewhere in the current
// threads state.
VG_REGPARM(3) void copyShadowTmptoTS(UWord src_tmp, UWord dest_reg, Addr instr_addr){
  if (!running && localTemps[src_tmp] != NULL) return;
  setTS(dest_reg, localTemps[src_tmp], instr_addr);

  if (localTemps[src_tmp] != NULL && print_moves){
    mpfr_exp_t shadowValexpt;
    char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                      localTemps[src_tmp]->values[0].value, MPFR_RNDN);
    VG_(printf)("Copying value %se%ld from temp %lu to thread state %lu\n",
                shadowValstr, shadowValexpt, src_tmp, dest_reg);
    mpfr_free_str(shadowValstr);
  }
}

// Copy a shadow value from somewhere in the thread state to a temporary.
VG_REGPARM(3) void copyShadowTStoTmp(UWord src_reg, IRType dest_type, UWord dest_tmp){
  if (!running && getTS(src_reg) != NULL) return;
  if (dest_tmp > maxTempUsed) maxTempUsed = dest_tmp;
  copyShadow___toTmp(src_reg, dest_type, dest_tmp, getTS);

  if (getTS(src_reg) != NULL && print_moves){
    mpfr_exp_t shadowValexpt;
    char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                      getTS(src_reg)->values[0].value, MPFR_RNDN);
    VG_(printf)("Copying value %se%ld from thread state %lu to temp %lu\n",
                shadowValstr, shadowValexpt, src_reg, dest_tmp);
    mpfr_free_str(shadowValstr);
  }
}

// Copy a shadow value from memory to a temporary
VG_REGPARM(3) void copyShadowMemtoTmp(Addr src_mem, IRType dest_type, UWord dest_tmp){
  if (!running && getMem(src_mem) != NULL) return;
  if (dest_tmp > maxTempUsed) maxTempUsed = dest_tmp;
  copyShadow___toTmp(src_mem, dest_type, dest_tmp, getMem);

  if (getMem(src_mem) != NULL && print_moves){
    mpfr_exp_t shadowValexpt;
    char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                      getMem(src_mem)->values[0].value, MPFR_RNDN);
    VG_(printf)("Copying value %se%ld from address %lx to temp %lu\n",
                shadowValstr, shadowValexpt, src_mem, dest_tmp);
    mpfr_free_str(shadowValstr);
  }
}

// Copy a shadow value from memory to a temporary, only if cond
// evaluates to true. Otherwise, copy the shadow value from another
// temporary, "alt_tmp".
VG_REGPARM(1) void copyShadowMemtoTmpIf(LoadG_Info* info){
  ShadowLocation* src = NULL;
  if (info->dest_tmp > maxTempUsed) maxTempUsed = info->dest_tmp;
  if (info->cond) {
    if (!running && getMem(info->src_mem) != NULL) return;
    copyShadow___toTmp(info->src_mem, info->dest_type, info->dest_tmp, getMem);

    if (getMem(info->src_mem) != NULL && print_moves){
      mpfr_exp_t shadowValexpt;
      char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                        getMem(info->src_mem)->values[0].value,
                                        MPFR_RNDN);
      VG_(printf)("Copying value %se%ld from address %lx to temp %lu\n",
                  shadowValstr, shadowValexpt, info->src_mem, info->dest_tmp);
      mpfr_free_str(shadowValstr);
    }
  } else {
    if (!running && localTemps[info->alt_tmp] != NULL) return;
    src = localTemps[info->alt_tmp];

    if (src != NULL && print_moves){
      mpfr_exp_t shadowValexpt;
      char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                        localTemps[info->alt_tmp]->values[0].value,
                                        MPFR_RNDN);
      VG_(printf)("Copying value %se%ld from temp %lu to temp %lu\n",
                  shadowValstr, shadowValexpt, info->alt_tmp, info->dest_tmp);
      mpfr_free_str(shadowValstr);
    }
  }
  copySL(src, &localTemps[info->dest_tmp]);
}

// Copy a shadow value from a temporary to memory.
VG_REGPARM(2) void copyShadowTmptoMem(UWord src_tmp, Addr dest_mem){
  if (!running && getTemp(src_tmp) != NULL) return;
  setMem(dest_mem, getTemp(src_tmp));

  if (getTemp(src_tmp) != NULL && print_moves){
    mpfr_exp_t shadowValexpt;
    char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                      localTemps[src_tmp]->values[0].value,
                                      MPFR_RNDN);
    VG_(printf)("Copying value %se%ld from temp %lu to address %lx\n",
                shadowValstr, shadowValexpt, src_tmp, dest_mem);
    mpfr_free_str(shadowValstr);
  }
}

// Copy a shadow value from a temporary to memory, only if cond
// evaluates to true. Otherwise, do nothing.
VG_REGPARM(3) void copyShadowTmptoMemG(UWord cond, UWord src_tmp, Addr dest_mem){
  if (!running && getTemp(src_tmp) != NULL) return;
  if (cond) copyShadowTmptoMem(src_tmp, dest_mem);

  if (getTemp(src_tmp) != NULL && cond && print_moves){
    mpfr_exp_t shadowValexpt;
    char* shadowValstr = mpfr_get_str(NULL, &shadowValexpt, 10, longprint_len,
                                      localTemps[src_tmp]->values[0].value,
                                      MPFR_RNDN);
    VG_(printf)("Copying value %se%ld from temp %lu to address %lx\n",
                shadowValstr, shadowValexpt, src_tmp, dest_mem);
    mpfr_free_str(shadowValstr);
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
      disownSL(localTemps[i]);
      localTemps[i] = NULL;
    }
  }
}

void cleanupStorage(void){
  // Clean up the thread state.
  for (int i = 0; i < MAX_THREADS; ++i)
    for (int j = 0; j < MAX_REGISTERS; ++j)
      if (threadRegisters[i][j] != NULL){
        disownSL(threadRegisters[i][j]);
        threadRegisters[i][j] = NULL;
      }
  // Clean up the memory shadowing table
  VG_(HT_ResetIter)(globalMemory);
  for (ShadowLocation_ptr* next = VG_(HT_Next)(globalMemory); next != NULL; next = VG_(HT_Next)(globalMemory)){
    disownSL(next->sl);
  }
}

void setTemp(Addr index, ShadowLocation* newLocation){
  if (index > maxTempUsed) maxTempUsed = index;
  copySL(newLocation, &localTemps[index]);
}

ShadowLocation* getTemp(Addr index){
  ShadowLocation* result = localTemps[index];
  return result;
}

void setMem(Addr index, ShadowLocation* newLocation){
  ShadowLocation_ptr *newEntry, *existingEntry;
  existingEntry = VG_(HT_lookup)(globalMemory, index);
  ALLOC(newEntry, "hg.memoryentry.1", 1, sizeof(ShadowLocation_ptr));

  // If an entry already exists at that location, then we're going to
  // overwrite it, so clean it up properly.
  if (existingEntry != NULL){
    disownSL(existingEntry->sl);
    VG_(HT_remove)(globalMemory, index);
    VG_(free)(existingEntry);
  }
  // Now, add our new entry.
  if (newLocation != NULL){
    copySL(newLocation, &(newEntry->sl));
    newEntry->addr = index;
    VG_(HT_add_node)(globalMemory, newEntry);
  }
}

ShadowLocation* getMem(Addr index){
  ShadowLocation_ptr* entry = VG_(HT_lookup)(globalMemory, index);
  if (entry == NULL) return NULL;
  return entry->sl;
}

void setTS(Addr index, ShadowLocation* newLocation, Addr instr_addr){
  if (!running) return;

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

  // Next, only save stuff that we're going to overwrite.
  if (newLocation == NULL &&
      // The indexes of the thread state that the first, second, and
      // third arguments to a replaced libm function are passed
      // in. This is a pretty horrible hack, but I'm working around a
      // terrible limitation in valgrind. So, maybe that justfies
      // it. This might not be cross platform either, but as my
      // advisor says, we'll burn that bridge when we cross it.
      (index == 224 || index == 256 || index == 288)){
    int argIndex;
    switch(index){
    case 224:
      argIndex = 0;
      break;
    case 256:
      argIndex = 1;
      break;
    case 288:
      argIndex = 2;
      break;
    default:
      return;
    }

    const HChar* objname;
    VG_(get_objname)(instr_addr, &objname);
    if (!VG_(string_match)("?*ld-?*.so", objname)){
      // If not, then it's in user code and is trying to actually
      // overwrite the location because it's passing an argument
      // that has not yet determined it's a floating point value, so
      // doesn't have a shadow value.
      setSavedArg(argIndex, NULL);
    } else if (threadRegisters[VG_(get_running_tid)()][index] != NULL){
      // If we are in the linker code, and the value that we're
      // about to overwrite isn't null, then we want to save it in
      // our saved arg register.
      setSavedArg(argIndex, threadRegisters[VG_(get_running_tid)()][index]);
    }
    // Finally, actually do the overwrite.
    copySL(NULL, &threadRegisters[VG_(get_running_tid)()][index]);
  } else
    copySL(newLocation, &threadRegisters[VG_(get_running_tid)()][index]);
}

ShadowLocation* getTS(Addr index){
  if (!running) return NULL;
  return threadRegisters[VG_(get_running_tid)()][index];
}

void setSavedArg(Int index, ShadowLocation* newLocation){
  savedArgs[index] = newLocation;
}

ShadowLocation* getSavedArg(Int index){
  return savedArgs[index];
}

void initStorage(void){
  globalMemory = VG_(HT_construct)("memory_shadows");
}

// This needs a rewrite to actually support everything that could
// happen without completely going crazy.
void copyShadow___toTmp(UWord src_idx, IRType dest_type, UWord dest_tmp,
                        ShadowLocation* (*get)(Addr idx)){
  ShadowLocation* srcLoc = get(src_idx);
  // If we didn't think this location held a float before, then we
  // don't think that wherever we're assigning to does now.
  if (srcLoc == NULL){
    setTemp(dest_tmp, srcLoc);
    return;
  }
  // Based on the type, we may have to pull only one of the shadow
  // values out of a multiple-value location.
  switch(dest_type){
  case Ity_I32:
  case Ity_F32:
    switch(srcLoc->type){
    case Lt_Floatx8:
    case Lt_Floatx4:
    case Lt_Floatx2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Double);
        copySV(&srcLoc->values[0], &tmpLoc->values[0]);
        localTemps[dest_tmp] = tmpLoc;
      }
      break;
    case Lt_Float:
      copySL(srcLoc, &localTemps[dest_tmp]);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size get!\n");
      VG_(printf)("(we're trying to get a value of type: ");
      ppIRType(dest_type);
      VG_(printf)(", but the location has type %d)\n", srcLoc->type);
      break;
    }
    break;
  case Ity_I64:
  case Ity_F64:
    switch(srcLoc->type){
    case Lt_Doublex4:
    case Lt_Doublex2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Double);
        copySV(&srcLoc->values[0], &tmpLoc->values[0]);
        setTemp(dest_tmp, tmpLoc);
      }
      break;
    case Lt_Floatx2:
    case Lt_Double:
      setTemp(dest_tmp, srcLoc);
      break;
    case Lt_Float:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Floatx2);
        copySV(&srcLoc->values[0], &tmpLoc->values[0]);
        ShadowLocation* nextSrcLoc = get(src_idx + sizeof(float));
        if (nextSrcLoc != NULL)
          copySV(&nextSrcLoc->values[0], &tmpLoc->values[1]);
        setTemp(dest_tmp, tmpLoc);
      }
      break;
    default:
      VG_(dmsg)("We don't support that mixed size get!\n");
      VG_(printf)("(we're trying to get a value of type: ");
      ppIRType(dest_type);
      VG_(printf)(")\n");
      break;
    }
    break;
  case Ity_F128:
  case Ity_V128:
    switch (srcLoc->type){
    case Lt_Doublex2:
    case Lt_Floatx4:
      setTemp(dest_tmp, srcLoc);
      break;
      // This is a crazy thing to support. I mean, I want this tool to
      // be usable, so I'm going to try my best, but this is super
      // weird, so I'm only going to support it as-needed until this
      // file gets a much needed refactor. So, these cases will be
      // inconsistent for now. If you see this message in any version
      // of the code that is released in any sense, please yell at me
      // or send me an angry email, as needed. asnchstr@cs.washington.edu
    case Lt_Double:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Doublex2);
        copySV(&srcLoc->values[0], &tmpLoc->values[0]);
        ShadowLocation* nextSrcLoc = get(src_idx + sizeof(double));
        if (nextSrcLoc != NULL)
          copySV(&nextSrcLoc->values[0], &tmpLoc->values[1]);
        setTemp(dest_tmp, tmpLoc);
      }
      break;
    case Lt_Floatx2:
      {
        ShadowLocation* tmpLoc = mkShadowLocation(Lt_Floatx4);
        copySV(&srcLoc->values[0], &tmpLoc->values[0]);
        copySV(&srcLoc->values[1], &tmpLoc->values[1]);
        ShadowLocation* nextSrcLoc = get(src_idx + sizeof(float)*2);
        if (nextSrcLoc != NULL){
          switch (nextSrcLoc->type){
          case Lt_Floatx2:
            copySV(&nextSrcLoc->values[0], &tmpLoc->values[2]);
            copySV(&nextSrcLoc->values[1], &tmpLoc->values[3]);
            break;
          default:
            VG_(dmsg)("Oh shit that's a weird read. I don't know how to handle that.\n");
            break;
          }
        }
        setTemp(dest_tmp, tmpLoc);
      }
      break;
    default:
      VG_(dmsg)("We don't support that mixed size thread state get!\n");
      VG_(printf)("(we're trying to get a value of type: ");
      ppIRType(dest_type);
      VG_(printf)(")\n");
      break;
    }
    break;
  case Ity_V256:
    switch(srcLoc->type){
    case Lt_Doublex4:
    case Lt_Floatx8:
      setTemp(dest_tmp, srcLoc);
      break;
    default:
      VG_(dmsg)("We don't support that mixed size thread state get!\n");
      VG_(printf)("(we're trying to get a value of type: ");
      ppIRType(dest_type);
      VG_(printf)(")\n");
      break;
    }
    break;
  default:
    VG_(dmsg)("We don't support that mixed size thread state get!\n");
    VG_(printf)("(we're trying to get a value of type: ");
    ppIRType(dest_type);
    VG_(printf)(")\n");
    break;
  }
}
