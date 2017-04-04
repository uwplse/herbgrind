/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie        exit-float-op.c ---*/
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

#include "exit-float-op.h"
#include "shadowop.h"
#include "pub_tool_libcprint.h"

VG_REGPARM(3) void checkCompare(FloatType argPrecision, IRTemp t1, IRTemp t2){
  ShadowTemp* args[2];
  for(int i = 0; i < 2; ++i){
    args[i] = getArg(i, 1, argPrecision, i == 0 ? t1 : t2);
  }
  double correctFst = getDouble(args[0]->values[0]->real);
  double correctSnd = getDouble(args[1]->values[0]->real);
  int actualResult;
  if (correctFst != correctFst ||
      correctSnd != correctSnd){
    actualResult = 0x45;
  } else if (correctFst < correctSnd){
    actualResult = 0x01;
  } else if (correctFst > correctSnd){
    actualResult = 0x00;
  } else {
    actualResult = 0x40;
  }
  int computedValue =
    *((int*)&computedResult.f[0]);
  ShadowValue* values[2];
  for(int i = 0; i < 2; ++i){
    values[i] = args[i]->values[0];
  }
  markEscapeFromFloat("Compare", actualResult != computedValue, 2, values);
  if (t1 == -1){
    disownShadowTemp_fast(args[0]);
  }
  if (t2 == -1){
    disownShadowTemp_fast(args[1]);
  }
}
VG_REGPARM(2) void checkConvert(FloatType argPrecision, IRTemp tmp){
  ShadowTemp* arg = getArg(0, 1, argPrecision, tmp);
  int correctResult = (int)getDouble(arg->values[0]->real);
  int computedValue =
    *((int*)&computedResult.f[0]);
  markEscapeFromFloat("Convert", correctResult != computedValue, 1, &(arg->values[0]));
  if (tmp == -1){
    disownShadowTemp_fast(arg);
  }
}
