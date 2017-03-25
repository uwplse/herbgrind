/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                exprs.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
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

#include "exprs.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "../../helper/ir-info.h"
#include "../shadowop/symbolic-op.h"
#include "../shadowop/mathreplace.h"
#include <math.h>

Stack* leafCExprs;
Stack* branchCExprs[MAX_BRANCH_ARGS];
Xarray_H(Stack*, StackArray);
Xarray_Impl(Stack*, StackArray);
Xarray_H(char*, VarList);
Xarray_Impl(char*, VarList);
StackArray concGraftStacks;

const char* varnames[] = {"x", "y", "z", "a", "b", "c",
                          "i", "j", "k", "l", "m", "n"};
#define MAX_FOLD_DEPTH 10

#define lambda(return_type, function_body)      \
  ({                                            \
    return_type __fn__ function_body            \
      __fn__;                                   \
  })

VarList extraVars;

List_Impl(NodePos, Group);
Xarray_Impl(Group, GroupList);
// This is unfortunate...
inline
int graftPointConc(ConcExpr* parent, ConcExpr* child){
  return 1;
  ShadowOpInfo* childOp = child->branch.op;
  ShadowOpInfo* parentOp = parent->branch.op;
  return child->type == Node_Leaf ||
    childOp->block_addr != parentOp->block_addr ||
    childOp->op_addr >= parentOp->op_addr;
}
inline
int graftPointSymb(SymbExpr* parent, SymbExpr* child){
  return 1;
  ShadowOpInfo* childOp = child->branch.op;
  ShadowOpInfo* parentOp = parent->branch.op;
  return child->type == Node_Leaf ||
    childOp->block_addr != parentOp->block_addr ||
    childOp->op_addr >= parentOp->op_addr;
}

#define FoldExpr_H_Named(T, N)                                        \
  T foldExpr_##N(T acc, SymbExpr* expr,                               \
                 T (*pre_f)(T, SymbExpr*, NodePos,                    \
                            VarMap*, int, int),                       \
                 T (*post_f)(T, SymbExpr*, NodePos,                   \
                             VarMap*, int, int))
#define FoldExpr_H(T) FoldExpr_H_Named(T, T)

#define FoldBlock_H_Named(T, N)                          \
  T foldBlock_##N(T acc, SymbExpr* expr,                 \
                  T (*pre_f)(T, SymbExpr*, NodePos,      \
                             VarMap*, int, int),         \
                  T (*post_f)(T, SymbExpr*, NodePos,     \
                              VarMap*, int, int))
#define FoldBlock_H(T) FoldBlock_H_Named(T, T)

/* So this is a complex macro, so let me try to write down what it
   does. Like some other generic libraries in this codebase, you pass
   FoldExpr_H a type to add a declaration for a foldExpr of the right
   type. You pass FoldExpr_Impl the same type in some source file to
   create a matching implementation. What you get is a function
   foldExpr of the type you want, that will do a sort of 'fold' over a
   Herbgrind symbolic expression. You pass it an initial value of type
   T, a symbolic expression to fold over, and two functions. They both
   take a value of type T, and produce another value of type T, and
   additionally have the current expression, the current position, the
   top level var map, the current depth, and whether or not it was a
   graft. They pre and post functions are both run for each node, in a
   long chan where each result feeds into the next function starting
   from the initial value, and the final value is returned from
   foldExpr. The order of functions is as follows: for every node,
   it's pre- function runs before any of it's children, and it's post-
   function gets the result of stringing together it's children's
   calls recursively in argument order from the output of the pre-
   function. */

#define FoldExpr_Impl_Named(T, N)                                       \
  T foldExpr_##N##_(T acc, SymbExpr* expr,                              \
                    NodePos pos, VarMap* map,                           \
                    int depth, int isGraft,                             \
                    T (*pre_f)(T, SymbExpr*, NodePos,                   \
                               VarMap*, int, int),                      \
                    T (*post_f)(T, SymbExpr*, NodePos,                  \
                                VarMap*, int, int));                    \
                                                                        \
  T foldExpr_##N##_(T acc, SymbExpr* expr,                              \
                    NodePos pos, VarMap* map,                           \
                    int depth, int isGraft,                             \
                    T (*pre_f)(T, SymbExpr*, NodePos,                   \
                               VarMap*, int, int),                      \
                    T (*post_f)(T, SymbExpr*, NodePos,                  \
                                VarMap*, int, int)){                    \
    if (expr->type == Node_Leaf){                                       \
      T result = pre_f(acc, expr, pos, map, depth, 1);                  \
      return post_f(result, expr, pos, map, depth, 1);                  \
    } else {                                                            \
      T local_acc = acc;                                                \
      local_acc = pre_f(local_acc, expr, pos, map, depth, isGraft);     \
      for(int i = 0; i < expr->branch.nargs; ++i){                      \
        NodePos newPos = rconsPos(pos, i);                              \
        SymbExpr* childNode = expr->branch.args[i];                     \
        int linkIsGraft = graftPointSymb(expr, childNode);              \
        if (depth < MAX_FOLD_DEPTH){                                    \
          local_acc = foldExpr_##N##_(local_acc, childNode,             \
                                    newPos, map, depth + 1,             \
                                    linkIsGraft,                        \
                                    pre_f, post_f);                     \
        } else {                                                        \
          local_acc = pre_f(local_acc, childNode, newPos,               \
                            map, depth + 1,                             \
                            linkIsGraft);                               \
          local_acc = post_f(local_acc, childNode, newPos,              \
                             map, depth + 1,                            \
                             linkIsGraft);                              \
        }                                                               \
      }                                                                 \
      return post_f(local_acc, expr, pos, map, depth, isGraft);         \
    }                                                                   \
  }                                                                     \
T foldExpr_##N(T initial, SymbExpr* expr,                               \
               T (*pre_f)(T, SymbExpr*, NodePos,                        \
                          VarMap*, int, int),                           \
               T (*post_f)(T, SymbExpr*, NodePos,                       \
                           VarMap*, int, int)){                         \
                                                                        \
  return foldExpr_##N##_                                                \
    (initial, expr, null_pos,                                           \
     mkVarMap(expr->type == Node_Branch ?                               \
              groupsWithoutNonLeaves(expr, expr->branch.groups) :        \
              mkXA(GroupList)()),                                       \
     0, 0, pre_f, post_f);                                              \
}                                                                       \
                                                                        \
T foldId_##N(T v, SymbExpr* e, NodePos p,                               \
             VarMap* m, int d, int g);                                  \
T foldId_##N(T v, SymbExpr* e, NodePos p,                               \
             VarMap* m, int d, int g){                                  \
  return v;                                                             \
}
#define FoldBlock_Impl_Named(T, N)                                      \
  T foldBlock_##N##_(T acc, SymbExpr* expr,                             \
                     NodePos pos, VarMap* map,                          \
                     int depth, int isGraft,                            \
                    T (*pre_f)(T, SymbExpr*, NodePos,                   \
                               VarMap*, int, int),                      \
                    T (*post_f)(T, SymbExpr*, NodePos,                  \
                                VarMap*, int, int));                    \
                                                                        \
  T foldBlock_##N##_(T acc, SymbExpr* expr,                             \
                     NodePos pos, VarMap* map,                          \
                     int depth, int isGraft,                            \
                     T (*pre_f)(T, SymbExpr*, NodePos,                  \
                                VarMap*, int, int),                     \
                     T (*post_f)(T, SymbExpr*, NodePos,                 \
                                 VarMap*, int, int)){                   \
    if (expr->type == Node_Leaf){                                       \
      T result = pre_f(acc, expr, pos, map, depth, 1);                  \
      return post_f(result, expr, pos, map, depth, 1);                  \
    } else {                                                            \
      T local_acc = acc;                                                \
      local_acc = pre_f(local_acc, expr, pos, map, depth, 0);           \
      for(int i = 0; i < expr->branch.nargs; ++i){                      \
        NodePos newPos = rconsPos(pos, i);                              \
        SymbExpr* childNode = expr->branch.args[i];                     \
        int linkIsGraft = graftPointSymb(expr, childNode);              \
        if (depth < MAX_FOLD_DEPTH && !linkIsGraft){                    \
          local_acc = foldBlock_##N##_(local_acc, childNode,             \
                                      newPos, map, depth + 1,           \
                                      0,                                \
                                      pre_f, post_f);                   \
        } else {                                                        \
          local_acc = pre_f(local_acc, childNode, newPos,               \
                            map, depth + 1,                             \
                            0);                                         \
          local_acc = post_f(local_acc, childNode, newPos,              \
                             map, depth + 1,                            \
                             0);                                        \
        }                                                               \
      }                                                                 \
      return post_f(local_acc, expr, pos, map, depth, 0);               \
    }                                                                   \
  }                                                                     \
  T foldBlock_##N(T initial, SymbExpr* expr,                            \
                  T (*pre_f)(T, SymbExpr*, NodePos,                     \
                             VarMap*, int, int),                        \
                  T (*post_f)(T, SymbExpr*, NodePos,                    \
                              VarMap*, int, int)){                      \
                                                                        \
    return foldBlock_##N##_                                             \
      (initial, expr, null_pos,                                         \
       mkVarMap(expr->type == Node_Branch ?                             \
                groupsWithoutNonLeaves(expr, expr->branch.groups) :     \
                mkXA(GroupList)()),                                     \
       0, 0, pre_f, post_f);                                            \
  }                                                                     \
                                                                        \
 T foldBlockId_##N(T v, SymbExpr* e, NodePos p,                         \
             VarMap* m, int d, int g);                                  \
 T foldBlockId_##N(T v, SymbExpr* e, NodePos p,                         \
             VarMap* m, int d, int g){                                  \
  return v;                                                             \
}

#define FoldBlock_Impl(T) FoldBlock_Impl_Named(T, T)
#define FoldExpr_Impl(T) FoldExpr_Impl_Named(T, T)
#define foldExpr(T) foldExpr_##T
#define foldBlock(T) foldBlock_##T
#define foldId(T) foldId_##T

FoldExpr_H(int);
FoldExpr_Impl(int);

typedef struct {
  int bound;
  char* buf;
} BBuf;
FoldExpr_H_Named(BBuf*, BBuf);
FoldExpr_Impl_Named(BBuf*, BBuf);


void initExprAllocator(void){
  leafCExprs = mkStack();
  concGraftStacks = mkXA(StackArray)();
  for(int i = 0; i < MAX_BRANCH_ARGS; ++i){
    branchCExprs[i] = mkStack();
  }
  extraVars = mkXA(VarList)();
  initializePositionTree();
}
void pushConcGraftStack(ConcGraft* graft, int count){
  while(concGraftStacks->size < count){
    Stack* newGStack = mkStack();
    XApush(StackArray)(concGraftStacks, newGStack);
  }
  Stack* curStack = concGraftStacks->data[count - 1];
  stack_push(curStack, (void*)graft);
}
ConcGraft* popConcGraftStack(int count){
  if (concGraftStacks->size < count){
    return NULL;
  } else {
    Stack* curStack = concGraftStacks->data[count - 1];
    if (stack_empty(curStack)){
      return NULL;
    }
    return (void*)stack_pop(curStack);
  }
}
VG_REGPARM(1) void freeBranchConcExpr(ConcExpr* expr){
  for(int i = 0; i < expr->branch.nargs; ++i){
    disownConcExpr(expr->branch.args[i]);
  }
  stack_push(branchCExprs[expr->branch.nargs], (void*)expr);
  pushConcGraftStack(expr->grafts, expr->ngrafts);
}
void disownConcExpr(ConcExpr* expr){
  (expr->ref_count)--;
  if (expr->ref_count == 0){
    if (expr->type == Node_Leaf){
      stack_push(leafCExprs, (void*)expr);
    } else {
      freeBranchConcExpr(expr);
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

  // Grafts
  result->ngrafts = 0;
  result->grafts = NULL;
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

  // Grafts
  result->ngrafts = 0;
  for(int i = 0; i < nargs; ++i){
    ConcExpr* child = result->branch.args[i];
    if (graftPointConc(result, child)){

      result->ngrafts += 1;
    } else {
      result->ngrafts += child->ngrafts;
    }
  }
  result->grafts = popConcGraftStack(result->ngrafts);
  if (result->grafts == NULL){
    result->grafts =
      VG_(perm_malloc)(sizeof(ConcGraft) * result->ngrafts,
                       vg_alignof(ConcGraft));
  }
  int grafti = 0;
  for(int i = 0; i < nargs; ++i){
    ConcExpr* child = result->branch.args[i];
    if (graftPointConc(result, child)){
      result->grafts[grafti].parent = result;
      result->grafts[grafti].childIndex = i;
      grafti++;
    } else {
      VG_(memcpy)(&(result->grafts[grafti]),
                  child->grafts,
                  sizeof(ConcGraft) * child->ngrafts);
      grafti += child->ngrafts;
    }
  }
  return result;
}

SymbExpr* mkFreshSymbolicLeaf(Bool isConst, double constVal){
  SymbExpr* result = VG_(perm_malloc)(sizeof(SymbExpr),
                                      vg_alignof(SymbExpr));

  result->isConst = isConst;
  result->constVal = constVal;
  result->type = Node_Leaf;
  result->branch.groups = NULL;
  result->ngrafts = 0;
  result->grafts = NULL;
  return result;
}

SymbExpr* concreteToSymbolic(ConcExpr* cexpr){
  // This is only necessarily sound if you are updating expr trees
  // with every operation, not if you only pick some, like the
  // erroneous operations. Because, you might be going down your tree,
  // see a node that was already created, and use it, when actually it
  // mismatches your concrete expression farther down, because it
  // wasn't updated recently.
  if (cexpr->type == Node_Branch &&
      cexpr->branch.op->expr != NULL){
    SymbExpr* existingExpr = cexpr->branch.op->expr;
    return existingExpr;
  }

  SymbExpr* result = VG_(perm_malloc)(sizeof(SymbExpr),
                                      vg_alignof(SymbExpr));
  if (cexpr->type == Node_Branch){
    cexpr->branch.op->expr = result;
  }

  result->isConst = True;
  result->constVal = cexpr->value;
  if (cexpr->type == Node_Leaf){
    result->type = Node_Leaf;
    result->branch.groups = NULL;

    result->ngrafts = 0;
    result->grafts = NULL;
  } else {
    result->type = Node_Branch;
    result->branch.op = cexpr->branch.op;
    result->branch.nargs = cexpr->branch.nargs;
    result->branch.args =
      VG_(perm_malloc)(sizeof(SymbExpr*) * cexpr->branch.nargs,
                       vg_alignof(SymbExpr*));
    for(int i = 0; i < cexpr->branch.nargs; ++i){
      result->branch.args[i] = concreteToSymbolic(cexpr->branch.args[i]);
    }
    result->ngrafts = cexpr->ngrafts;
    result->grafts =
      VG_(perm_malloc)(sizeof(SymbGraft) * cexpr->ngrafts,
                       vg_alignof(SymbGraft));
    for(int i = 0; i < cexpr->ngrafts; ++i){
      if (cexpr->grafts[i].parent == cexpr){
        result->grafts[i].parent = result;
      } else {
        result->grafts[i].parent =
          concreteToSymbolic(cexpr->grafts[i].parent);
      }
      result->grafts[i].childIndex = cexpr->grafts[i].childIndex;
    }
    result->branch.groups = getExprsEquivGroups(cexpr, result);
  }
  return result;
}
const char* opSym(ShadowOpInfo* op){
  if (op->op_code == 0x0){
    return getWrappedName(op->op_type);
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
      ConcExpr* childNode = expr->branch.args[i];
      if (graftPointConc(expr, childNode)){
        VG_(printf)("[");
        ppConcExpr(expr->branch.args[i]);
        VG_(printf)("]");
      } else {
        ppConcExpr(expr->branch.args[i]);
      }
    }
    VG_(printf)(")");
  }
}
void ppConcExprNoGrafts(ConcExpr* expr){
  if (expr->type == Node_Leaf){
    VG_(printf)("%f", expr->value);
  } else {
    VG_(printf)("{%lX}(%s",
                expr->branch.op->op_addr,
                opSym(expr->branch.op));
    for (int i = 0; i < expr->branch.nargs; ++i){
      VG_(printf)(" ");
      ConcExpr* childNode = expr->branch.args[i];
      if (graftPointConc(expr, childNode)){
        VG_(printf)("[_]");
      } else {
        ppConcExprNoGrafts(expr->branch.args[i]);
      }
    }
    VG_(printf)(")");
  }
}

void ppSymbExpr(SymbExpr* expr){
  char* stringRep = symbExprToString(expr);
  tl_assert(stringRep != NULL);
  VG_(printf)("%s", stringRep);
  VG_(free)(stringRep);
}
void ppSymbExprNoVars(SymbExpr* expr){
  char* stringRep = symbExprToStringNoVars(expr);
  VG_(printf)("%s", stringRep);
  VG_(free)(stringRep);
}
void ppSymbExprNoGrafts(SymbExpr* expr){
  char* stringRep = symbExprToStringNoGrafts(expr);
  VG_(printf)("%s", stringRep);
  VG_(free)(stringRep);
}
void ppSymbExprMarkSources(SymbExpr* expr){
  char* stringRep = symbExprToStringMarkSources(expr);
  VG_(printf)("%s", stringRep);
  VG_(free)(stringRep);
}

BBuf* mkBBuf(int bound, char* buf);
BBuf* mkBBuf(int bound, char* buf){
  BBuf* res = VG_(malloc)("bounded buffer", sizeof(BBuf));
  res->bound = bound;
  res->buf = buf;
  return res;
}
void printBBuf(BBuf* bbuf, const char* format, ...);
void printBBuf(BBuf* bbuf, const char* format, ...){
  va_list arglist;
  va_start(arglist, format);
  int printLength = VG_(vsnprintf)(bbuf->buf, bbuf->bound, format,
                                   arglist);
  if (printLength >= bbuf->bound){
    tl_assert2(printLength < bbuf->bound,
               "trying to print %d character past bound!\n",
               printLength - bbuf->bound);
  }
  va_end(arglist);
  bbuf->bound -= printLength;
  bbuf->buf += printLength;
}
inline
int symbExprConstPrintLen(SymbExpr* expr){
  if (isnan(expr->constVal)){
    return 3;
  } else {
    if (expr->constVal < 0){
      return floatPrintLen(expr->constVal) + 1;
    } else {
      return floatPrintLen(expr->constVal);
    }
  }
}
char* symbExprToStringNoVars(SymbExpr* expr){
  if (expr->type == Node_Leaf && !(expr->isConst)){
    int len = VG_(strlen)(varnames[0]);
    char* buf = VG_(malloc)("expr string", len);
    VG_(memcpy)(buf, varnames[0], len);
    return buf;
  }
  int expr_len = foldExpr(int)
    (0, expr,
     lambda (int, (int acc, SymbExpr* curExpr,
                   NodePos curPos, VarMap* varMap,
                   int depth, int isGraft) {
               if (curExpr->type == Node_Leaf){
                 if (curExpr->isConst){
                   int len = symbExprConstPrintLen(curExpr)
                     + 2 // brackets
                     + 1;// space
                   return acc + len;
                 } else {
                   return acc
                     + 1 // underscore
                     + 2 // brackets
                     + 1;// space
                 }
               } else if (depth > MAX_FOLD_DEPTH){
                 return acc
                   + 2; // "* "
               } else if (isGraft) {
                 int len = VG_(strlen)(opSym(curExpr->branch.op))
                   + 2 // brackets
                   + 2 // parenthesis
                   + 1;// space
                 return acc + len;
               } else {
                 int len = VG_(strlen)(opSym(curExpr->branch.op))
                   + 2 // parenthesis
                   + 1;// space
                 return acc + len;
               }
             }),
     foldId(int));
  char* buf = VG_(malloc)("expr string", expr_len + 1);
  VG_(free)(foldExpr(BBuf)
            (mkBBuf(expr_len + 1, buf), expr,
             lambda (BBuf*, (BBuf* acc, SymbExpr* curExpr,
                             NodePos curPos, VarMap* varMap,
                             int depth, int isGraft) {
                       if (curExpr->type == Node_Leaf){
                         if (curExpr->isConst){
                           printBBuf(acc, " [%f]", curExpr->constVal);
                           return acc;
                         } else {
                           printBBuf(acc, " [_]", curExpr->constVal);
                           return acc;
                         }
                       } else if (depth > MAX_FOLD_DEPTH){
                         printBBuf(acc, " *");
                         return acc;
                       } else if (isGraft) {
                         printBBuf(acc, " [(%s",
                                   opSym(curExpr->branch.op));
                         return acc;
                       } else {
                         printBBuf(acc, " (%s",
                                   opSym(curExpr->branch.op));
                         return acc;
                       }
                     }),
             lambda (BBuf*, (BBuf* acc, SymbExpr* curExpr,
                             NodePos curpos, VarMap* varMap,
                             int depth, int isGraft) {
                       if (curExpr->type == Node_Leaf){
                         return acc;
                       } else if (depth > MAX_FOLD_DEPTH){
                         return acc;
                       } else if (isGraft) {
                         printBBuf(acc, ")]");
                         return acc;
                       } else {
                         printBBuf(acc, ")");
                         return acc;
                       }
                     })));
  return buf;
}
char* symbExprToString(SymbExpr* expr){
  if (expr->type == Node_Leaf && !(expr->isConst)){
    int len = VG_(strlen)(varnames[0]);
    char* buf = VG_(malloc)("expr string", len);
    VG_(memcpy)(buf, varnames[0], len);
    return buf;
  }
  int expr_len = foldExpr(int)
    (0, expr,
     lambda (int, (int acc, SymbExpr* curExpr,
                   NodePos curPos, VarMap* varMap,
                   int depth, int isGraft) {
               if (curExpr->type == Node_Leaf){
                 if (curExpr->isConst){
                   int len = symbExprConstPrintLen(curExpr)
                     + 2 // brackets
                     + 1;// space
                   return acc + len;
                 } else {
                   int len =
                     VG_(strlen)(getVar(lookupVar(varMap, curPos)))
                     + 2 // brackets
                     + 1;// space
                   return acc + len;
                 }
               } else if (depth > MAX_FOLD_DEPTH){
                 return acc
                   + 2; // "* "
               } else if (isGraft) {
                 int len = VG_(strlen)(opSym(curExpr->branch.op))
                   + 2 // brackets
                   + 2 // parenthesis
                   + 1;// space
                 return acc + len;
               } else {
                 int len = VG_(strlen)(opSym(curExpr->branch.op))
                   + 2 // parenthesis
                   + 1;// space
                 return acc + len;
               }
             }),
     foldId(int));
  char* buf = VG_(malloc)("expr string", expr_len + 1);
  VG_(free)(foldExpr(BBuf)
            (mkBBuf(expr_len + 1, buf), expr,
             lambda (BBuf*, (BBuf* acc, SymbExpr* curExpr,
                             NodePos curPos, VarMap* varMap,
                             int depth, int isGraft) {
                       if (curExpr->type == Node_Leaf){
                         if (curExpr->isConst){
                           printBBuf(acc, " [%f]", curExpr->constVal);
                           return acc;
                         } else {
                           printBBuf(acc, " [%s]",
                                     getVar(lookupVar(varMap, curPos)));
                           return acc;
                         }
                       } else if (depth > MAX_FOLD_DEPTH){
                         printBBuf(acc, " *");
                         return acc;
                       } else if (isGraft) {
                         printBBuf(acc, " [(%s",
                                   opSym(curExpr->branch.op));
                         return acc;
                       } else {
                         printBBuf(acc, " (%s",
                                   opSym(curExpr->branch.op));
                         return acc;
                       }
                     }),
             lambda (BBuf*, (BBuf* acc, SymbExpr* curExpr,
                             NodePos curpos, VarMap* varMap,
                             int depth, int isGraft) {
                       if (curExpr->type == Node_Leaf){
                         return acc;
                       } else if (depth > MAX_FOLD_DEPTH){
                         return acc;
                       } else if (isGraft) {
                         printBBuf(acc, ")]");
                         return acc;
                       } else {
                         printBBuf(acc, ")");
                         return acc;
                       }
                     })));
  return buf;
}
char* symbExprToStringMarkSources(SymbExpr* expr){
  if (expr->type == Node_Leaf && !(expr->isConst)){
    int len = VG_(strlen)(varnames[0]);
    char* buf = VG_(malloc)("expr string", len);
    VG_(memcpy)(buf, varnames[0], len);
    return buf;
  }
  int expr_len = foldExpr(int)
    (0, expr,
     lambda (int, (int acc, SymbExpr* curExpr,
                   NodePos curPos, VarMap* varMap,
                   int depth, int isGraft) {
               if (curExpr->type == Node_Leaf){
                 if (curExpr->isConst){
                   int len = symbExprConstPrintLen(curExpr)
                     + 2 // brackets
                     + 1;// space
                   return acc + len;
                 } else {
                   int len =
                     VG_(strlen)(getVar(lookupVar(varMap, curPos)))
                     + 2 // brackets
                     + 1;// space
                   return acc + len;
                 }
               } else if (depth > MAX_FOLD_DEPTH){
                 return acc
                   + 2; // "* "
               } else if (isGraft) {
                 int len = 13
                   + VG_(strlen)(opSym(curExpr->branch.op))
                   + 2 // brackets
                   + 2 // parenthesis
                   + 1;// space
                 return acc + len;
               } else {
                 int len = 13
                   + VG_(strlen)(opSym(curExpr->branch.op))
                   + 2 // parenthesis
                   + 1;// space
                 return acc + len;
               }
             }),
     foldId(int));
  char* buf = VG_(malloc)("expr string", expr_len + 1);
  VG_(free)(foldExpr(BBuf)
            (mkBBuf(expr_len + 1, buf), expr,
             lambda (BBuf*, (BBuf* acc, SymbExpr* curExpr,
                             NodePos curPos, VarMap* varMap,
                             int depth, int isGraft) {
                       if (curExpr->type == Node_Leaf){
                         if (curExpr->isConst){
                           printBBuf(acc, " [%f]", curExpr->constVal);
                           return acc;
                         } else {
                           printBBuf(acc, " [%s]",
                                     getVar(lookupVar(varMap, curPos)));
                           return acc;
                         }
                       } else if (depth > MAX_FOLD_DEPTH){
                         printBBuf(acc, " *");
                         return acc;
                       } else if (isGraft) {
                         printBBuf(acc, " {%lX}[(%s",
                                   curExpr->branch.op->op_addr,
                                   opSym(curExpr->branch.op));
                         return acc;
                       } else {
                         printBBuf(acc, " {%lX}(%s",
                                   curExpr->branch.op->op_addr,
                                   opSym(curExpr->branch.op));
                         return acc;
                       }
                     }),
             lambda (BBuf*, (BBuf* acc, SymbExpr* curExpr,
                             NodePos curpos, VarMap* varMap,
                             int depth, int isGraft) {
                       if (curExpr->type == Node_Leaf){
                         return acc;
                       } else if (depth > MAX_FOLD_DEPTH){
                         return acc;
                       } else if (isGraft) {
                         printBBuf(acc, ")]");
                         return acc;
                       } else {
                         printBBuf(acc, ")");
                         return acc;
                       }
                     })));
  return buf;
}
FoldBlock_H(int);
FoldBlock_Impl(int);
FoldBlock_H_Named(BBuf*, BBuf);
FoldBlock_Impl_Named(BBuf*, BBuf);
char* symbExprToStringNoGrafts(SymbExpr* expr){
  if (expr->type == Node_Leaf && !(expr->isConst)){
    int len = VG_(strlen)(varnames[0]);
    char* buf = VG_(malloc)("expr string", len);
    VG_(memcpy)(buf, varnames[0], len);
    return buf;
  }
  int expr_len = foldBlock(int)
    (0, expr,
     lambda (int, (int acc, SymbExpr* curExpr,
                   NodePos curPos, VarMap* varMap,
                   int depth, int isGraft) {
               if (curExpr->type == Node_Leaf){
                 if (curExpr->isConst){
                   int len = symbExprConstPrintLen(curExpr)
                     + 2 // brackets
                     + 1;// space
                   return acc + len;
                 } else {
                   int len =
                     VG_(strlen)(getVar(lookupVar(varMap, curPos)))
                     + 2 // brackets
                     + 1;// space
                   return acc + len;
                 }
               } else if (depth > MAX_FOLD_DEPTH){
                 return acc
                   + 2; // "* "
               } else if (isGraft) {
                 int len = VG_(strlen)(opSym(curExpr->branch.op))
                   + 2;
                 return acc + len;
               } else {
                 int len = VG_(strlen)(opSym(curExpr->branch.op))
                   + 2 // parenthesis
                   + 1;// space
                 return acc + len;
               }
             }),
     foldId(int));
  char* buf = VG_(malloc)("expr string", expr_len + 1);
  VG_(free)(foldBlock(BBuf)
            (mkBBuf(expr_len + 1, buf), expr,
             lambda (BBuf*, (BBuf* acc, SymbExpr* curExpr,
                             NodePos curPos, VarMap* varMap,
                             int depth, int isGraft) {
                       if (curExpr->type == Node_Leaf){
                         if (curExpr->isConst){
                           printBBuf(acc, " [%f]", curExpr->constVal);
                           return acc;
                         } else {
                           printBBuf(acc, " [%s]",
                                     getVar(lookupVar(varMap, curPos)));
                           return acc;
                         }
                       } else if (depth > MAX_FOLD_DEPTH){
                         printBBuf(acc, " *");
                         return acc;
                       } else if (isGraft) {
                         printBBuf(acc, " _",
                                   opSym(curExpr->branch.op));
                         return acc;
                       } else {
                         printBBuf(acc, " (%s",
                                   opSym(curExpr->branch.op));
                         return acc;
                       }
                     }),
             lambda (BBuf*, (BBuf* acc, SymbExpr* curExpr,
                             NodePos curpos, VarMap* varMap,
                             int depth, int isGraft) {
                       if (curExpr->type == Node_Leaf){
                         return acc;
                       } else if (depth > MAX_FOLD_DEPTH){
                         return acc;
                       } else if (isGraft) {
                         printBBuf(acc, ")]");
                         return acc;
                       } else {
                         printBBuf(acc, ")");
                         return acc;
                       }
                     })));
  return buf;
}
char* symbExprVarString(SymbExpr* expr){
  char* buf = NULL;
  GroupList trimmedGroups =
    groupsWithoutNonLeaves(expr, expr->branch.groups);
  int numVars =
    trimmedGroups->size;
  tl_assert(numVars <= sizeof(varnames));
  freeXA(GroupList)(trimmedGroups);
  switch(numVars){
  case 0:{
    char noVars[] = "()";
    buf = VG_(malloc)("var string", sizeof(noVars));
  }
    break;
  case 1:{
    int buflen = VG_(strlen)(varnames[0] + 3);
    buf = VG_(malloc)("var string", sizeof(char) * buflen);
    VG_(snprintf)(buf, buflen, "(%s)", varnames[0]);
  }
    break;
  default:{
    int buflen = 2 * numVars + 2;
    buf = VG_(malloc)("var string", buflen * sizeof(char));
    tl_assert(VG_(strlen)(varnames[0]) == 1);
    int count = 0;
    count += VG_(snprintf)(buf + count, buflen - count,
                           "(%s ", varnames[0]);
    for(int i = 0; i < numVars - 1; ++i){
      count += VG_(snprintf)(buf + count, buflen - count, "%s ", getVar(i));
    }
    VG_(snprintf)(buf + count, buflen - count, "%s)",
                  getVar(numVars - 1));
  }
    break;
  }
  return buf;
}
int floatPrintLen(double f){
  int wholeDigits = ((int)log10(f));
  if (wholeDigits < 0){
    wholeDigits = 0;
  }
  return wholeDigits + 8;
}
const char* getVar(int idx){
  int numStaticVars = (sizeof(varnames) / sizeof(char*));
  if (idx < numStaticVars){
    return varnames[idx];
  } else {
    tl_assert(idx < 100000);
    while(idx >= numStaticVars + extraVars->size){
      char* newVar = VG_(perm_malloc)(sizeof(char) * 5, vg_alignof(char));
      VG_(snprintf)(newVar, 5, "x%d", extraVars->size);
      XApush(VarList)(extraVars, newVar);
    }
    return extraVars->data[idx - numStaticVars];
  }
}
