/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie            shadowval.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017Alex Sanchez-Stern

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
#ifndef _SHADOWVAL_H
#define _SHADOWVAL_H

#include "pub_tool_basics.h"
#include "pub_tool_xarray.h"

#include "exprs.hh"
#include "real.h"
#include "../../instrument/floattypes.h"
#include "../../helper/list.h"
#include "../op-shadowstate/shadowop-info.h"

List_H(ShadowOpInfo*, InfluenceList);

typedef struct _ShadowValue {
  // For the various data structures that will hold these values.
  struct _ShadowValue* next;

  UWord ref_count;
  Real real;
  ConcExpr* expr;
  InfluenceList influences;
  ValueType type;
} ShadowValue;

typedef struct _ShadowTemp {
  // For the shadow temp stacks.
  struct _ShadowTemp* next;

  ShadowValue** values;
  int num_vals;
} ShadowTemp;

// Don't assume that the new shadow temp will have NULL values!!!
VG_REGPARM(1) ShadowTemp* newShadowTemp(UWord num_vals);
ShadowTemp* copyShadowTemp(ShadowTemp* temp);
void changeSingleValueType(ShadowTemp* temp, ValueType type);

UWord hashDouble(double val);
ShadowValue* newShadowValue(ValueType type);
void updateRanges(RangeRecord* records, double* args, int nargs);
VG_REGPARM(2) void assertValValid(const char* label, ShadowValue* val);
VG_REGPARM(2) void assertTempValid(const char* label, ShadowTemp* temp);

VG_REGPARM(3) void assertValType(const char* label, ShadowValue* val, ValueType type);
VG_REGPARM(3) void assertTempType(const char* label, ShadowTemp* temp, ValueType type);
VG_REGPARM(3) void assertNumVals(const char* label, ShadowTemp* temp, int num_vals);
VG_REGPARM(3) void assertNumValsNot(const char* label,
                                    ShadowTemp* temp,
                                    int num_vals);
VG_REGPARM(3) void assertDynamicSize(const char* label,
                                     ShadowTemp* temp,
                                     int num_halfwords);

#endif
