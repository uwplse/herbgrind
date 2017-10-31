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
#include "../../helper/runtime-util.h"

VG_REGPARM(1) void checkCompare(ShadowCmpInfo* info){
  if (no_reals) return;
  ShadowTemp* args[2];
  for(int i = 0; i < 2; ++i){
    args[i] = getArg(i, numChannelsIn(info->op_code), info->precision, info->argTemps[i]);
  }
  int correctOutput;
  if (numSIMDOperands(info->op_code) == 1){
    if (double_comparisons){
      double correctFst = getDouble(args[0]->values[0]->real);
      double correctSnd = getDouble(args[1]->values[0]->real);
      switch(info->op_code){
      case Iop_CmpF64:
      case Iop_CmpF32:{
        if (correctFst != correctFst ||
            correctSnd != correctSnd){
          correctOutput = 0x45;
        } else if (correctFst < correctSnd){
          correctOutput = 0x01;
        } else if (correctFst > correctSnd){
          correctOutput = 0x00;
        } else {
          correctOutput = 0x40;
        }
      }
        break;
      case Iop_CmpLT32F0x4:
      case Iop_CmpLT64F0x2: {
        if (correctFst != correctFst ||
            correctSnd != correctSnd){
          correctOutput = 0x0;
        } else if (correctFst < correctSnd){
          correctOutput = 0x01;
        } else if (correctFst > correctSnd){
          correctOutput = 0x00;
        } else {
          correctOutput = 0x00;
        }
      }
        break;
      case Iop_CmpLE64F0x2: {
        if (correctFst != correctFst ||
            correctSnd != correctSnd) {
          correctOutput = 0x00;
        } else if (correctFst <= correctSnd) {
          correctOutput = 0x01;
        } else {
          correctOutput = 0x00;
        }
      }
        break;
      case Iop_CmpUN64F0x2:
      case Iop_CmpUN32F0x4:{
        if (correctFst == correctSnd){
          correctOutput = 0x01;
        } else {
          correctOutput = 0x00;
        }
      }
        break;
      default:
        tl_assert(0);
        return;
      }
    } else {
      Real realFst = args[0]->values[0]->real;
      Real realSnd = args[1]->values[0]->real;
      switch(info->op_code){
      case Iop_CmpF64:
      case Iop_CmpF32:{
        if (isNaN(realFst) || isNaN(realSnd)){
          correctOutput = 0x45;
        } else if (realCompare(realFst, realSnd) < 0){
          correctOutput = 0x01;
        } else if (realCompare(realFst, realSnd) > 0){
          correctOutput = 0x00;
        } else {
          correctOutput = 0x40;
        }
      }
      case Iop_CmpLT32F0x4:
      case Iop_CmpLT64F0x2: {
        if (isNaN(realFst) || isNaN(realSnd)){
          correctOutput = 0x0;
        } else if (realCompare(realFst, realSnd) < 0){
          correctOutput = 0x01;
        } else if (realCompare(realFst, realSnd) > 0){
          correctOutput = 0x00;
        } else {
          correctOutput = 0x00;
        }
      }
        break;
      case Iop_CmpLE64F0x2: {
        if (isNaN(realFst) || isNaN(realSnd)){
          correctOutput = 0x00;
        } else if (realCompare(realFst, realSnd) <= 0) {
          correctOutput = 0x01;
        } else {
          correctOutput = 0x00;
        }
      }
        break;
      case Iop_CmpUN64F0x2:
      case Iop_CmpUN32F0x4:{
        if (realCompare(realFst, realSnd) == 0){
          correctOutput = 0x01;
        } else {
          correctOutput = 0x00;
        }
      }
        break;
      default:
        tl_assert(0);
        return;
      }
    }
  } else {
    tl_assert(0);
  }
  if (numSIMDOperands(info->op_code) == 1){
    int computedOutput =
      *((int*)&computedResult.f[0]);
    ShadowValue* values[2];
    for(int i = 0; i < 2; ++i){
      values[i] = args[i]->values[0];
    }
    if (print_compares){
      if (correctOutput != computedOutput){
        VG_(printf)("Bad comparison!\n");
      } else {
        VG_(printf)("Good comparison.\n");
      }
      ppIROp_Extended(info->op_code);
      VG_(printf)(": ");
      VG_(printf)("Correct output: %X; Computed output: %X\n",
                  correctOutput, computedOutput);
    }
    markEscapeFromFloat("compare",
                        correctOutput != computedOutput,
                        2, values);
    if (follow_real_execution){
      computedResult.f[0] = *((float*)&correctOutput);
    }
  } else {
    ppIROp_Extended(info->op_code);
    tl_assert(0);
  }
  for(int i = 0; i < 2; ++i){
    if (info->argTemps[i] == -1){
      disownShadowTemp_fast(args[i]);
    }
  }
}
VG_REGPARM(3) void checkConvert(FloatType argPrecision, IRTemp tmp,
                                Addr curAddr){
  ShadowTemp* arg = getArg(0, 1, argPrecision, tmp);
  int correctResult = (int)getDouble(arg->values[0]->real);
  int computedValue =
    *((int*)&computedResult.f[0]);
  markEscapeFromFloat("convert",
                      correctResult != computedValue,
                      1, &(arg->values[0]));
  if (tmp == -1){
    disownShadowTemp_fast(arg);
  }
  if (follow_real_execution){
    computedResult.f[0] = *((float*)&correctResult);
  }
}
