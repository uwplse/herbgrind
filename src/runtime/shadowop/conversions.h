/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie          conversions.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017 Alex Sanchez-Stern

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
#ifndef _CONVERSIONS_H
#define _CONVERSIONS_H

#include "../value-shadowstate/shadowval.h"

// Some conversion operators
VG_REGPARM(1)
ShadowTemp* zeroHi96ofV128(ShadowTemp* input);
VG_REGPARM(1)
ShadowTemp* zeroHi64ofV128(ShadowTemp* input);
VG_REGPARM(1)
ShadowTemp* v128to32(ShadowTemp* input);
VG_REGPARM(1)
ShadowTemp* v128to64(ShadowTemp* input);
VG_REGPARM(1)
ShadowTemp* v128Hito64(ShadowTemp* input);
VG_REGPARM(1)
ShadowTemp* i64UtoV128(ShadowTemp* t);
VG_REGPARM(1)
ShadowTemp* f128Loto64(ShadowTemp* input);
VG_REGPARM(1)
ShadowTemp* f128Hito64(ShadowTemp* input);
VG_REGPARM(2)
ShadowTemp* setV128lo32(ShadowTemp* topThree, ShadowTemp* bottomOne);
VG_REGPARM(2)
ShadowTemp* setV128lo64(ShadowTemp* topOne, ShadowTemp* bottomOne);
VG_REGPARM(3)
ShadowTemp* setV128lo64Dynamic1(ShadowTemp* bottom,
                                IRTemp topIdx, UWord* topVal);
VG_REGPARM(3)
ShadowTemp* setV128lo64Dynamic2(ShadowTemp* top,
                                IRTemp bottomIdx, UWord bottomVal);
VG_REGPARM(2)
ShadowTemp* i64HLtoV128NoFirstShadow(UWord hi, ShadowTemp* lo);
VG_REGPARM(2)
ShadowTemp* i64HLtoV128NoSecondShadow(ShadowTemp* hi, UWord lo);
VG_REGPARM(2)
ShadowTemp* i64HLtoV128(ShadowTemp* hi, ShadowTemp* low);
VG_REGPARM(2)
ShadowTemp* i32UtoV128(ShadowTemp* t);
VG_REGPARM(2)
ShadowTemp* f64HLtoF128(ShadowTemp* hi, ShadowTemp* low);
VG_REGPARM(2)
ShadowTemp* i32Uto64(ShadowTemp* t);
VG_REGPARM(2)
ShadowTemp* i64to32(ShadowTemp* t);
#endif
