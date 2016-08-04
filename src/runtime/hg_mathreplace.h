
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie       hg_mathreplace.h ---*/
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

#ifndef _HG_MATHREPLACE
#define _HG_MATHREPLACE

#include "../include/hg_mathreplace_funcs.h"
#include "../types/hg_shadowvals.h"

// This gets us a hash table data structure that's safe to use with
// valgrind, so we can map specific calls to wrapped functions to
// their meta-data about the operation.
#include "pub_tool_hashtable.h"

extern VgHashTable* callToOpInfoMap;

void performOp(OpType op, double* result, double* args);
ShadowValue* getShadowValMem(Addr addr, double float_arg,
                             Int argIndex);

// This function, called from within a math replacement function,
// get's the address of the call site for that function in client
// code.
Addr getCallAddr(void);

#endif
