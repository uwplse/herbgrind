/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            shadowval.h ---*/
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
#ifndef _SHADOWVAL_H
#define _SHADOWVAL_H

#include "pub_tool_basics.h"
#include "pub_tool_xarray.h"

#include "exprs.hh"
#include "real.h"

typedef enum {
  Ft_Invalid,
  Ft_Single,
  Ft_Double
} FloatType;

typedef struct _ShadowValue {
  UWord ref_count;
  Real real;
  ConcExpr* expr;
  XArray* influences;
  FloatType type;
} ShadowValue;

typedef struct _ShadowTemp {
  ShadowValue** values;
  int num_vals;
} ShadowTemp;

void initShadowValueSystem(void);
ShadowTemp* newShadowTemp(UWord num_vals);
void freeShadowTemp(ShadowTemp* temp);
ShadowTemp* copyShadowTemp(ShadowTemp* temp);
ShadowTemp* deepCopyShadowTemp(ShadowTemp* temp);
void changeSingleValueType(ShadowTemp* temp, FloatType type);

UWord hashDouble(double val);
VG_REGPARM(1) ShadowValue* newShadowValue(FloatType type, double value);
void freeShadowValue(ShadowValue* val);
ShadowValue* copyShadowValue(ShadowValue* val);

#endif
