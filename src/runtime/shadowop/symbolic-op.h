/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          symbolic-op.h ---*/
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

#ifndef _SYMBOLIC_OP_H
#define _SYMBOLIC_OP_H

#include "../value-shadowstate/exprs.h"
#include "../op-shadowstate/shadowop-info.h"

#include "pub_tool_basics.h"
#include "pub_tool_hashtable.h"

typedef struct _varMapEntry {
  struct _varMapEntry* next;
  UWord positionHash;
  NodePos position;
  UWord varIdx;
} VarMapEntry;

typedef struct _valMapEntry {
  struct _valMapEntry* next;
  UWord valHash;
  double val;
  UWord groupIdx;
} ValMapEntry;

void execSymbolicOp(ShadowOpInfo* opinfo, ConcExpr** result, Real real, ShadowValue** args);
void generalizeSymbolicExpr(SymbExpr** symexpr, ConcExpr* cexpr);

void generalizeStructure(SymbExpr* symbexpr, ConcExpr* concExpr);
void intersectEqualities(SymbExpr* symbexpr, ConcExpr* concExpr);
GroupList getConcExprEquivGroups(ConcExpr* concExpr);
GroupList pruneSingletonGroups(GroupList list);
GroupList groupsWithoutNonLeaves(SymbExpr* structure, GroupList list);

VarMap* mkVarMap(GroupList groups);
int lookupVar(VarMap* map, NodePos pos);
void freeVarMap(VarMap* map);

void ppEquivGroups(GroupList groups);
void addValEntry(VgHashTable* valmap, double val, int groupIdx);
int lookupVal(VgHashTable* valmap, double val);
UWord hashValue(double val);
Word cmp_value(const void* node1, const void* node2);

ConcExpr* concGraftPosGet(ConcExpr* expr, NodePos pos);
SymbExpr* symbGraftPosGet(SymbExpr* expr, NodePos pos);
UWord hashPosition(NodePos node);
Word cmp_position(const void* node1, const void* node2);
NodePos appendPos(NodePos orig, unsigned char argIdx);
void freePos(NodePos pos);
NodePos copyPos(NodePos pos);
void ppNodePos(NodePos pos);
#define NULL_POS (NodePos){.len = 0, .data = NULL}

#endif
