/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                exprs.c ---*/
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

#include "exprs.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"
#include "../../helper/stack.h"
#include "../../helper/ir-info.h"

#define MAX_BRANCH_ARGS 3

Stack* leafCExprs;
Stack* branchCExprs[MAX_BRANCH_ARGS];

void initExprAllocator(void){
  leafCExprs = mkStack();
  for(int i = 0; i < MAX_BRANCH_ARGS; ++i){
    branchCExprs[i] = mkStack();
  }
}
void disownExpr(ConcExpr* expr){
  (expr->ref_count)--;
  if (expr->ref_count == 0){
    if (expr->type == Node_Leaf){
      stack_push(leafCExprs, (void*)expr);
    } else {
      stack_push(branchCExprs[expr->branch.nargs], (void*)expr);
    }
  }
}
ConcExpr* mkLeafExpr(double value){
  ConcExpr* result;
  if (stack_empty(leafCExprs)){
    result = VG_(malloc)("expr", sizeof(ConcExpr));
    result->type = Node_Leaf;
  } else {
    result = (void*)stack_pop(leafCExprs);
  }
  result->ref_count = 1;
  result->value = value;
  return result;
}

ConcExpr* mkBranchExpr(double value, ShadowOpInfo* op, int nargs, ConcExpr** args){
  ConcExpr* result;
  if (stack_empty(branchCExprs[nargs])){
    result = VG_(malloc)("expr", sizeof(ConcExpr));
    result->branch.args = VG_(malloc)("expr args", sizeof(ConcExpr*) * nargs);
    result->branch.nargs = nargs;
    result->type = Node_Branch;
  } else {
    result = (void*)stack_pop(branchCExprs[nargs]);
  }
  result->ref_count = 1;
  result->value = value;
  result->branch.op = op;

  for(int i = 0; i < nargs; ++i){
    result->branch.args[i] = args[i];
    (args[i]->ref_count)++;
  }
  return result;
}

void execSymbolicOp(ShadowOpInfo* opinfo, ConcExpr** result, Real real, ShadowValue** args){
  ConcExpr* exprArgs[MAX_BRANCH_ARGS];
  for(int i = 0; i < opinfo->exinfo.nargs; ++i){
    exprArgs[i] = args[i]->expr;
  }
  *result = mkBranchExpr(getDouble(real), opinfo, opinfo->exinfo.nargs, exprArgs);
}
void ppConcExpr(ConcExpr* expr){
  if (expr->type == Node_Leaf){
    VG_(printf)("%f", expr->value);
  } else {
    if (expr->branch.op->op_code == 0x0){
      VG_(printf)("(%s", expr->branch.op->name);
    } else {
      VG_(printf)("(%s", getOpcodeSymbol(expr->branch.op->op_code));
    }
    for (int i = 0; i < expr->branch.nargs; ++i){
      VG_(printf)(" ");
      ppConcExpr(expr->branch.args[i]);
    }
    VG_(printf)(")");
  }
}
