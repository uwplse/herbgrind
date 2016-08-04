
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          hg_shadowop.h ---*/
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

#ifndef _HG_SHADOWOP
#define _HG_SHADOWOP

#include "mpfr.h"
#include "hg_evaluate.h"
#include "hg_hiprec_ops.h"
#include "../types/hg_shadowvals.h"
#include "../types/hg_opinfo.h"

// The functions that we'll insert into the program to execute shadow
// operations alongside the normal operations.
VG_REGPARM(1) void executeUnaryShadowOp(Op_Info* opInfo);
VG_REGPARM(1) void executeBinaryShadowOp(Op_Info* opInfo);
VG_REGPARM(1) void executeTernaryShadowOp(Op_Info* opInfo);
VG_REGPARM(1) void executeQuadnaryShadowOp(Op_Info* opInfo);

// Get's the current shadow location in the given temporary, or makes
// a new bare one with the given type if it doesn't exist.
ShadowLocation* getShadowLocation(UWord tmp_num, LocType type);
// Get a shadow value for the given temporary. If the temporary does
// not have a shadow value, create one with the given type using the
// given bytes to initialize the value.
ShadowValue* getShadowValue(ShadowLocation* loc, UWord index,
                            UWord* loc_bytes);
// Convert IR rounding mode codes to MPFR rounding mode codes.
mpfr_rnd_t roundmodeIRtoMPFR(IRRoundingMode round);
#endif //_HG_SHADOWOP
