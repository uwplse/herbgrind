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
#include <math.h>

#define MAX_BRANCH_ARGS 3

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

void execSymbolicOp(ShadowOpInfo* opinfo, ConcExpr** result,
                    Real real, ShadowValue** args){
  ConcExpr* exprArgs[MAX_BRANCH_ARGS];
  for(int i = 0; i < opinfo->exinfo.nargs; ++i){
    exprArgs[i] = args[i]->expr;
  }
  *result = mkBranchConcExpr(getDouble(real), opinfo, opinfo->exinfo.nargs, exprArgs);
  generalizeSymbolicExpr(&(opinfo->expr), *result);
  if (print_errors){
    ppSymbExpr(opinfo->expr);
    VG_(printf)("\n");
  }
}
void generalizeSymbolicExpr(SymbExpr** symexpr, ConcExpr* cexpr){
  if (*symexpr == NULL){
    *symexpr = concreteToSymbolic(cexpr);
  } else {
    tl_assert2(0,
               "We don't support updating symbolic expressions yet!\n");
  }
}

SymbExpr* concreteToSymbolic(ConcExpr* cexpr){
  SymbExpr* result = VG_(malloc)("symbolic expr", sizeof(SymbExpr));
  result->isConst = True;
  result->constVal = cexpr->value;
  if (cexpr->type == Node_Leaf){
    result->type = Node_Leaf;
    result->branch.equiv_map = NULL;
  } else {
    result->type = Node_Branch;
    result->branch.op = cexpr->branch.op;
    result->branch.nargs = cexpr->branch.nargs;
    result->branch.equiv_map = getConcExprEquivalences(cexpr);
    result->branch.args =
      VG_(malloc)("symbolic expr args",
                  sizeof(SymbExpr*) * cexpr->branch.nargs);
    for(int i = 0; i < cexpr->branch.nargs; ++i){
      result->branch.args[i] = concreteToSymbolic(cexpr->branch.args[i]);
    }
  }
  return result;
}

VgHashTable* getConcExprEquivalences(ConcExpr* cexpr){
  // TODO
  return NULL;
}

UWord hashPosition(NodePos node){
  UWord hash = 0;
  for(int i = 0; i < node.len; ++i){
    hash = 31 * hash + node.data[i] + 1;
  }
  return hash;
}
Word cmp_position(const void* node1, const void* node2){
  const NodeMapEntry* entry1 = (const NodeMapEntry*)node1;
  const NodeMapEntry* entry2 = (const NodeMapEntry*)node2;
  if (entry1->position.len != entry2->position.len){
    return 1;
  }
  for(SizeT i = 0; i < entry1->position.len; ++i){
    if (entry1->position.data[i] != entry2->position.data[i]){
      return 1;
    }
  }
  return 0;
}
int lookupPos(VgHashTable* varmap, NodePos pos){
  NodeMapEntry key = {.position = pos,
                      .positionHash = hashPosition(pos)};
  NodeMapEntry* entry = VG_(HT_gen_lookup)(varmap, &key, cmp_position);
  tl_assert(entry != NULL);
  return entry->groupIdx;
}
NodePos appendPos(NodePos orig, int argIdx){
  NodePos newPos;
  newPos.len = orig.len + 1;
  newPos.data = VG_(malloc)("pos data", newPos.len * sizeof(int));
  VG_(memcpy)(newPos.data + 1, orig.data, orig.len * sizeof(int));
  newPos.data[0] = argIdx;
  return newPos;
}
void freePos(NodePos pos){
  VG_(free)(pos.data);
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
  int len = symbExprPrintLen(expr, expr->branch.equiv_map,
                             NULL_POS);
  char* buf = VG_(malloc)("expr string", len + 1);
  writeSymbExprToString(buf, expr, NULL_POS, expr->branch.equiv_map);
  return buf;
}
int floatPrintLen(double f){
  return ((int)log10(f)) + 8;
}
int symbExprPrintLen(SymbExpr* expr, VgHashTable* varmap,
                     NodePos curPos){
  if (expr->type == Node_Leaf){
    if (expr->isConst){
      if (isnan(expr->constVal)){
        return 3;
      } else {
        return floatPrintLen(expr->constVal);
      }
    } else {
      return VG_(strlen)(varnames[lookupPos(varmap, curPos)]);
    }
  } else {
    int count = 2 + VG_(strlen)(opSym(expr->branch.op));
    for(int i = 0; i < expr->branch.nargs; ++i){
      count += symbExprPrintLen(expr->branch.args[i], varmap,
                                appendPos(curPos, i)) + 1;
    }
    return count;
  }
}
int writeSymbExprToString(char* buf, SymbExpr* expr,
                          NodePos curpos, VgHashTable* varmap){
  if (expr->type == Node_Leaf){
    if (expr->isConst){
      if (isnan(expr->constVal)){
        return VG_(sprintf)(buf, "NaN");
      } else {
        return VG_(sprintf)(buf, "%f", expr->constVal);
      }
    } else {
      return VG_(sprintf)(buf, "%s", varnames[lookupPos(varmap, curpos)]);
    }
  } else {
    int count = VG_(sprintf)(buf, "(%s", opSym(expr->branch.op));
    for(int i = 0; i < expr->branch.nargs; ++i){
      count += VG_(sprintf)(buf + count, " ");
      count += writeSymbExprToString(buf + count,
                                     expr->branch.args[i],
                                     appendPos(curpos, i),
                                     varmap);
    }
    count += VG_(sprintf)(buf + count, ")");
    return count;
  }
}
