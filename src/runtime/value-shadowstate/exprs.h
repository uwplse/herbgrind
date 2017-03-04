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

#include "../op-shadowstate/shadowop-info.h"

#include "pub_tool_basics.h"
#include "pub_tool_hashtable.h"

typedef enum {
  Node_Branch,
  Node_Leaf
} NodeType;

struct _ConcExpr {
  struct _ConcExpr* next;
  int ref_count;
  NodeType type;
  double value;
  struct {
    ShadowOpInfo* op;
    int nargs;
    ConcExpr** args;
  } branch;
};

struct _SymbExpr {
  NodeType type;
  double constVal;
  Bool isConst;
  struct {
    ShadowOpInfo* op;
    int nargs;
    SymbExpr** args;
    VgHashTable* equiv_map;
  } branch;
};

typedef struct {
  int* data;
  int len;
} NodePos;

typedef struct _NodeMapEntry {
  struct _NodeMapEntry* next;
  UWord positionHash;
  NodePos position;
  UWord groupIdx;
} NodeMapEntry;

void initExprAllocator(void);
ConcExpr* mkLeafConcExpr(double value);
ConcExpr* mkBranchConcExpr(double value, ShadowOpInfo* op, int nargs, ConcExpr** args);
void disownConcExpr(ConcExpr* expr);
void execSymbolicOp(ShadowOpInfo* opinfo, ConcExpr** result, Real real, ShadowValue** args);
void generalizeSymbolicExpr(SymbExpr** symexpr, ConcExpr* cexpr);
SymbExpr* concreteToSymbolic(ConcExpr* cexpr);
VgHashTable* getConcExprEquivalences(ConcExpr* cexpr);

int lookupPos(VgHashTable* varmap, NodePos pos);
UWord hashPosition(NodePos node);
Word cmp_position(const void* node1, const void* node2);
NodePos appendPos(NodePos orig, int argIdx);
void freePos(NodePos pos);
#define NULL_POS (NodePos){.len = 0, .data = NULL}

const char* opSym(ShadowOpInfo* op);
void ppConcExpr(ConcExpr* expr);
void ppSymbExpr(SymbExpr* expr);

// Free this when you're done with it.
char* symbExprToString(SymbExpr* expr);
int symbExprPrintLen(SymbExpr* expr, VgHashTable* varmap,
                     NodePos curPos);
// Returns the number of bytes written
int writeSymbExprToString(char* buf, SymbExpr* expr,
                          NodePos curpos, VgHashTable* varmap);
int floatPrintLen(double f);
#endif
