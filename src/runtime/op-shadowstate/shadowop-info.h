/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie             shadowop.h ---*/
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

#ifndef _SHADOWOP_INFO_H
#define _SHADOWOP_INFO_H

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_hashtable.h"

#include "../value-shadowstate/exprs.hh"
#include "../../instrument/floattypes.h"
#include "../../include/mathreplace-funcs.h"

extern VgHashTable* mathreplaceOpInfoMap;
extern VgHashTable* semanticOpInfoMap;
extern VgHashTable* markMap;

typedef struct _ErrorAggregate {
  double max_error;
  double total_error;
  long long int num_evals;
} ErrorAggregate;

typedef struct _ExtraInfo {
  int numSIMDOperands;
  int numChannels;
  int nargs;
  FloatType argPrecision;
} ExtraInfo;

typedef struct _ShadowOpInfo {
  // These two are mutually exclusive.
  IROp op_code;
  OpType op_type;

  Addr op_addr;
  Addr block_addr;
  ErrorAggregate eagg;
  ErrorAggregate local_eagg;
  SymbExpr* expr;
  ExtraInfo exinfo;
  int argTemps[4];
} ShadowOpInfo;

typedef struct _mrOpInfoEntry {
  struct _mrOpInfoEntry* next;
  UWord call_addr;
  OpType type;
  ShadowOpInfo* info;
} MrOpInfoEntry;
typedef struct _semOpInfoEntry {
  struct _semOpInfoEntry* next;
  UWord call_addr;
  IROp op_code;
  ShadowOpInfo* info;
} SemOpInfoEntry;


void initOpShadowState(void);
ShadowOpInfo* mkShadowOpInfo(IROp op_code, Addr op_addr, Addr block_addr,
                             int nargs);
void printOpInfo(ShadowOpInfo* opinfo);
void ppAddr(Addr addr);

#endif
