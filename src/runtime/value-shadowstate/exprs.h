/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie                exprs.h ---*/
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

#ifndef _EXPRS_H
#define _EXPRS_H

#include "exprs.hh"
#include "pos-tree.h"

#include "../op-shadowstate/shadowop-info.h"
#include "range.h"

#include "../../helper/list.h"
#include "../../helper/xarray.h"
#include "../../helper/stack.h"

#include "pub_tool_basics.h"
#include "pub_tool_hashtable.h"

typedef enum {
  Node_Branch,
  Node_Leaf
} NodeType;

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

extern Stack* leafCExprs;

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
    VgHashTable* varProblematicRanges;
    VgHashTable* exampleProblematicArgs;
  } branch;
};

typedef struct _varMap {
  VgHashTable* existingEntries;
  int nextVarIdx;
} VarMap;

void recursivelyOwnConcExpr(ConcExpr* expr, int depth);
void recursivelyDisownConcExpr(ConcExpr* expr, int depth);
void initExprAllocator(void);
ConcExpr* mkLeafConcExpr(double value);
ConcExpr* mkBranchConcExpr(double value, ShadowOpInfo* op, int nargs, ConcExpr** args);
VG_REGPARM(1) void freeBranchConcExpr(ConcExpr* expr);
void disownConcExpr(ConcExpr* expr);
SymbExpr* mkFreshSymbolicLeaf(Bool isConst, double constVal);
SymbExpr* concreteToSymbolic(ConcExpr* cexpr);

void pushConcGraftStack(ConcGraft* graft, int count);
ConcGraft* popConcGraftStack(int count);

int hasRepeatedVars(SymbExpr* expr);
SymbExpr* varSwallow(SymbExpr* expr);

void initializeProblematicRangesAndExample(SymbExpr* symbExpr);
void addRangeEntryCopy(VgHashTable* rangeMap, NodePos position, RangeRecord* original);
void addInitialRangeEntry(VgHashTable* rangeMap, NodePos position);
void addExampleEntryCopy(VgHashTable* exampleTable,
                         NodePos position, double original);
void addInitialExampleEntry(VgHashTable* exampleMap, NodePos position);
void updateProblematicRanges(SymbExpr* symbExpr, ConcExpr* cexpr);
RangeRecord* lookupRangeRecord(VgHashTable* rangeMap, NodePos position);
double lookupExampleInput(VgHashTable* exampleMap, NodePos position);
void getRangesAndExample(RangeRecord** totalRangesOut,
                         RangeRecord** problematicRangesOut,
                         double** exampleInputOut,
                         SymbExpr* expr, int num_vars);
void ppRangeTable(VgHashTable* rangeTable);

void ppEquivGroup(Group group);
void ppEquivGroups(GroupList groups);

const char* opSym(ShadowOpInfo* op);
void ppConcExprBounded(ConcExpr* expr, int max_depth);
void ppConcExpr(ConcExpr* expr);
void ppConcExprNoGrafts(ConcExpr* expr);
void ppSymbExpr(SymbExpr* expr);
void ppSymbExprNoVars(SymbExpr* expr);
void ppSymbExprNoGrafts(SymbExpr* expr);

int numVarNodes(SymbExpr* expr);
int numRepeatedVars(SymbExpr* expr, GroupList trimmedGroups);
int numExprVars(SymbExpr* expr);
int countVars(VarMap* map);
char* symbExprVarString(int num_vars);

const char* getVar(int idx);
int varLengthLookup(VarMap* map, NodePos pos);
// Free this when you're done with it.
char* symbExprToString(SymbExpr* expr, int* outNumVars);
// Returns the number of bytes written
int writeSymbExprToString(char* buf, SymbExpr* expr,
                          NodePos curpos, VarMap* varmap,
                          int depth);

char* symbExprToStringNoVars(SymbExpr* expr);
int noVarsVarLength(VarMap* map, NodePos pos);
int symbExprNoVarsPrintLenGraft(SymbExpr* expr, VarMap* varMap,
                                NodePos curPos, int depth);
int symbExprNoVarsPrintLen(SymbExpr* expr);
int writeVarBlank(char* buf, VarMap* varMap, NodePos curPos);
int writeGraftsNoVars(char* buf, SymbExpr* expr,
                      NodePos curPos, VarMap* varMap,
                      int depth);
void writeSymbExprToStringNoVars(int buflen, char* buf, SymbExpr* expr);


// Without grafts
char* symbExprToStringNoGrafts(SymbExpr* expr);
int symbExprGraftPrintLenBlankGrafts(SymbExpr* expr, VarMap* varMap,
                                     NodePos curPos, int depth);
int symbExprPrintLenBlankGrafts(SymbExpr* expr, VarMap* varMap,
                                NodePos curPos, int depth);
int writeGraftsBlankGrafts(char* buf, SymbExpr* expr,
                           NodePos curPos, VarMap* varMap, int depth);
int writeSymbExprToStringBlankGrafts(char* buf, SymbExpr* expr,
                                     NodePos curPos, VarMap* varMap,
                                     int depth);
int floatPrintLen(double f);
#define MAX_BRANCH_ARGS 3
#endif
