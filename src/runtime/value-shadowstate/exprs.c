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
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "../../helper/stack.h"
#include "../../helper/ir-info.h"
#include "../shadowop/symbolic-op.h"
#include <math.h>

Stack* leafCExprs;
Stack* branchCExprs[MAX_BRANCH_ARGS];

const char* varnames[] = {"x", "y", "z", "a", "b", "c",
                          "i", "j", "k", "l", "m", "n"};

void initExprAllocator(void){
  leafCExprs = mkStack();
  for(int i = 0; i < MAX_BRANCH_ARGS; ++i){
    branchCExprs[i] = mkStack();
  }
}
void disownConcExpr(ConcExpr* expr){
  (expr->ref_count)--;
  if (expr->ref_count == 0){
    if (expr->type == Node_Leaf){
      stack_push(leafCExprs, (void*)expr);
    } else {
      for(int i = 0; i < expr->branch.nargs; ++i){
        disownConcExpr(expr->branch.args[i]);
      }
      stack_push(branchCExprs[expr->branch.nargs], (void*)expr);
    }
  }
}
ConcExpr* mkLeafConcExpr(double value){
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

ConcExpr* mkBranchConcExpr(double value, ShadowOpInfo* op,
                           int nargs, ConcExpr** args){
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

SymbExpr* concreteToSymbolic(ConcExpr* cexpr){
  SymbExpr* result = VG_(malloc)("symbolic expr", sizeof(SymbExpr));
  result->isConst = True;
  result->constVal = cexpr->value;
  if (cexpr->type == Node_Leaf){
    result->type = Node_Leaf;
    result->branch.groups = NULL;
    result->branch.grafts = NULL;
  } else {
    result->type = Node_Branch;
    result->branch.op = cexpr->branch.op;
    result->branch.nargs = cexpr->branch.nargs;
    result->branch.groups = getConcExprEquivGroups(cexpr);
    result->branch.args =
      VG_(malloc)("symbolic expr args",
                  sizeof(SymbExpr*) * cexpr->branch.nargs);
    for(int i = 0; i < cexpr->branch.nargs; ++i){
      result->branch.args[i] = concreteToSymbolic(cexpr->branch.args[i]);
    }
  }
  return result;
}
const char* opSym(ShadowOpInfo* op){
  if (op->op_code == 0x0){
    return op->name;
  } else {
    return getOpcodeSymbol(op->op_code);
  }
}

void ppConcExpr(ConcExpr* expr){
  if (expr->type == Node_Leaf){
    VG_(printf)("%f", expr->value);
  } else {
    VG_(printf)("(%s", opSym(expr->branch.op));
    for (int i = 0; i < expr->branch.nargs; ++i){
      VG_(printf)(" ");
      ppConcExpr(expr->branch.args[i]);
    }
    VG_(printf)(")");
  }
}

void ppSymbExpr(SymbExpr* expr){
  char* stringRep = symbExprToString(expr);
  VG_(printf)("%s", stringRep);
  VG_(free)(stringRep);
}

char* symbExprToString(SymbExpr* expr){
  if (expr->type == Node_Leaf && !(expr->isConst)){
    int len = VG_(strlen)(varnames[0]);
    char* buf = VG_(malloc)("expr string", len);
    VG_(memcpy)(buf, varnames[0], len);
    return buf;
  }
  VarMap* varMap = mkVarMap(expr->branch.groups);
  int len = symbExprPrintLen(expr, varMap, NULL_POS);
  char* buf = VG_(malloc)("expr string", len + 1);
  writeSymbExprToString(buf, expr, NULL_POS, varMap);
  freeVarMap(varMap);
  return buf;
}
int floatPrintLen(double f){
  return ((int)log10(f)) + 8;
}
int symbExprPrintLen(SymbExpr* expr, VarMap* varmap,
                     NodePos curPos){
  if (expr->type == Node_Leaf){
    if (expr->isConst){
      if (isnan(expr->constVal)){
        return 3;
      } else {
        return floatPrintLen(expr->constVal);
      }
    } else {
      return VG_(strlen)(varnames[lookupVar(varmap, curPos)]);
    }
  } else {
    int count = 2 + VG_(strlen)(opSym(expr->branch.op));
    for(int i = 0; i < expr->branch.nargs; ++i){
      NodePos newPos = appendPos(curPos, i);
      count += symbExprPrintLen(expr->branch.args[i], varmap,
                                newPos) + 1;
      freePos(newPos);
    }
    return count;
  }
}
int writeSymbExprToString(char* buf, SymbExpr* expr,
                          NodePos curpos, VarMap* varmap){
  if (expr->type == Node_Leaf){
    if (expr->isConst){
      if (isnan(expr->constVal)){
        return VG_(sprintf)(buf, "NaN");
      } else {
        return VG_(sprintf)(buf, "%f", expr->constVal);
      }
    } else {
      return VG_(sprintf)(buf, "%s", varnames[lookupVar(varmap, curpos)]);
    }
  } else {
    int count = VG_(sprintf)(buf, "(%s", opSym(expr->branch.op));
    for(int i = 0; i < expr->branch.nargs; ++i){
      count += VG_(sprintf)(buf + count, " ");
      NodePos newPos = appendPos(curpos, i);
      count += writeSymbExprToString(buf + count,
                                     expr->branch.args[i],
                                     newPos,
                                     varmap);
      freePos(newPos);
    }
    count += VG_(sprintf)(buf + count, ")");
    return count;
  }
}
