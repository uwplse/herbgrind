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

#include "../../helper/list.h"
#include "../../helper/xarray.h"

#include "../op-shadowstate/shadowop-info.h"

#include "pub_tool_basics.h"
#include "pub_tool_hashtable.h"

typedef enum {
  Node_Branch,
  Node_Leaf
} NodeType;

typedef struct {
  unsigned char* data;
  int len;
} NodePos;

typedef struct _ConcGraft {
  struct _Graft* next;
  ConcExpr* parent;
  int childIndex;
} ConcGraft;

struct _ConcExpr {
  struct _ConcExpr* next;
  int ref_count;
  NodeType type;
  double value;
  int ngrafts;
  ConcGraft* grafts;
  struct {
    ShadowOpInfo* op;
    int nargs;
    ConcExpr** args;
  } branch;
};

typedef struct _SymbGraft {
  SymbExpr* parent;
  int childIndex;
} SymbGraft;

List_H(NodePos, Group);
Xarray_H(Group, GroupList);

struct _SymbExpr {
  NodeType type;
  double constVal;
  Bool isConst;
  int ngrafts;
  SymbGraft* grafts;
  struct {
    ShadowOpInfo* op;
    int nargs;
    SymbExpr** args;
    GroupList groups;
  } branch;
};

typedef struct _varMap {
  VgHashTable* existingEntries;
  int nextVarIdx;
} VarMap;

void initExprAllocator(void);
ConcExpr* mkLeafConcExpr(double value);
ConcExpr* mkBranchConcExpr(double value, ShadowOpInfo* op, int nargs, ConcExpr** args);
void disownConcExpr(ConcExpr* expr);
SymbExpr* mkFreshSymbolicLeaf(Bool isConst, double constVal);
SymbExpr* concreteToSymbolic(ConcExpr* cexpr);

void pushConcGraftStack(ConcGraft* graft, int count);
ConcGraft* popConcGraftStack(int count);

const char* opSym(ShadowOpInfo* op);
void ppConcExpr(ConcExpr* expr);
void ppSymbExpr(SymbExpr* expr);

// Free this when you're done with it.
char* symbExprToString(SymbExpr* expr);
int symbExprPrintLen(SymbExpr* expr, VarMap* varmap,
                     NodePos curPos);
// Returns the number of bytes written
int writeSymbExprToString(char* buf, SymbExpr* expr,
                          NodePos curpos, VarMap* varmap);
int floatPrintLen(double f);
#define MAX_BRANCH_ARGS 3
#define MAX_EXPR_BLOCK_DEPTH 4
#endif
