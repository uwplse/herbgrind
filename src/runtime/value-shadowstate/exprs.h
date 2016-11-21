/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                exprs.h ---*/
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

#ifndef _EXPRS_H
#define _EXPRS_H

#include "exprs.hh"

#include "../op-shadowstate/shadowop.h"

#include "pub_tool_basics.h"
#include "pub_tool_hashtable.h"

typedef enum {
  Node_Branch,
  Node_Leaf
} NodeType;

struct _ConcExpr {
  int ref_count;
  NodeType type;
  double value;
  struct {
    ShadowOp* op;
    int nargs;
    ConcExpr** args;
  } branch;
};

struct _SymbExpr {
  NodeType type;
  double constVal;
  Bool isConst;
  struct {
    ShadowOp* op;
    int nargs;
    SymbExpr** args;
    VgHashTable* equiv_map;
  } branch;
};

typedef struct {
  int* data;
  int len;
} NodePos;

void freeExpr(ConcExpr* expr);
#endif
