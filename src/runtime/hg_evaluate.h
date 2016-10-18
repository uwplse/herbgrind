
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          hg_evaluate.h ---*/
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

#ifndef _HG_EVALUATE
#define _HG_EVALUATE

#include "pub_tool_basics.h"
#include "pub_tool_libcprint.h"
#include "../types/hg_opinfo.h"
#include "../types/hg_shadowvals.h"

// Get the number of floating point values between x and y, including
// y.
unsigned long long ulpd(double x, double y);

// Evaluate the error of an executed operation.
void evaluateOpError(ShadowValue* shadowVal, double actualVal, Op_Info* opinfo, double localResult);
// A helper that makes it easier to extract the value from some arg
// bytes with the type and index. This will reinterpret valbytes as an
// array of whatever datatype bytestype indicates, and then index into
// it with el_index to get the actual floating point value that we'll
// test against the shadow value.
void evaluateOpError_helper(ShadowValue* shadowVal, LocType bytestype, int el_index, Op_Info* opinfo, double localResult);

#endif
