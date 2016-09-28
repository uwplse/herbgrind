
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie   hg_storage_runtime.h ---*/
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

#ifndef _HG_STORAGE_RUNTIME
#define _HG_STORAGE_RUNTIME

#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"
#include "../types/hg_shadowvals.h"
#include "../types/hg_opinfo.h"

// The maximum number of pre-instrumentation temporary values per
// block we're willing to deal with.
#define	MAX_TEMPS 1000
// The maximum number of per-thread registers we'll account for.
#define	MAX_REGISTERS 1000
#define MAX_THREADS 16

// The functions that we'll insert into the program to move around
// shadow values at run time.
VG_REGPARM(1) void copyShadowTmptoTmp(CpShadow_Info* info);
VG_REGPARM(1) void copyShadowTmptoTS(CpShadow_Info* info);
VG_REGPARM(1) void copyShadowTStoTmp(CpShadow_Info* info);
VG_REGPARM(1) void copyShadowMemtoTmp(CpShadow_Info* info);
VG_REGPARM(1) void copyShadowMemtoTmpIf(LoadG_Info* info);
VG_REGPARM(1) void copyShadowTmptoMem(CpShadow_Info* info);
VG_REGPARM(2) void copyShadowTmptoMemG(UWord cond, CpShadow_Info* info);
// Disown all shadow values held in temporaries and set the
// temporaries array to nulls.
VG_REGPARM(0) void initBlock(void);
VG_REGPARM(0) void cleanupBlock(void);
void initStorage(void);
void cleanupStorage(void);

// Getters and setters for temporaries, memory, and thread
// state. Handles reference counting properly.
void setTemp(Addr index, ShadowLocation* newLocation);
ShadowLocation* getTemp(Addr index);

void setMem(Addr index, ShadowValue* newValue);
ShadowValue* getMem(Addr index);
void setLocMem(Addr index, ShadowLocation* newLocation, LocType move_type);
ShadowLocation* getLocMem(Addr index, LocType type);

void setTS(Addr index, ShadowValue* newValue);
ShadowValue* getTS(Addr index);
void setLocTS(Addr index, ShadowLocation* newLocation, LocType move_type, Addr instr_addr);
ShadowLocation* getLocTS(Addr index, LocType type);

void setLoc__(Addr index, ShadowLocation* newLoc, LocType move_type,
              void (*setter)(Addr index, ShadowValue* val));
ShadowLocation* getLoc__(Addr index, ShadowValue* (*getter)(Addr index), LocType type);

void setSavedArg(Int index, ShadowValue* newLocation);
ShadowValue* getSavedArg(Int index);

// A helper function that does most of the work for copyShadowMemtoTmp
// and copyShadowTStoTmp.
void copyShadow___toTmp(UWord src_idx, IRType dest_type, UWord dest_tmp, ShadowLocation* (*get)(Addr idx));

#define CHECK_LOC(loc) for (SizeT i = 0; i < capacity(loc->type); ++i){ CHECK_PTR(loc->values[i]); }

#endif
