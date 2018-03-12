/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie              ir-info.h ---*/
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

#ifndef _IR_INFO_H
#define _IR_INFO_H

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

typedef enum {
  IEop_Neg32F0x4=Iop_LAST+1,
  IEop_Neg64F0x2,
  IEop_REALLY_LAST_FOR_REAL_GUYS,
} IROp_Extended;

int numChannelsIn(IROp op_code);
int numChannelsOut(IROp op_code);
int numSIMDOperands(IROp op_code);
int numArgs(IROp_Extended op_code);
int inferOtherNumChannels(int inferIndex, IRExpr* arg, IROp op_code);
const char* getOpcodeSymbol(IROp op_code);
double runEmulatedOp(IROp op_code, double* args);
void ppIROp_Extended(int op_code);

Bool isSpecialOp(IROp op_code);
Bool isFloatOp(IROp op);
Bool isExitFloatOp(IROp op);
Bool isConversionOp(IROp op_code);

#define MAX_TEMP_SHADOWS 8

#define IEop_INVALID (IROp_Extended)(Iop_INVALID)

#endif
