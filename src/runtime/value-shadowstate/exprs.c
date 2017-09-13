/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie                exprs.c ---*/
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

#include "exprs.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_oset.h"
#include "../../helper/ir-info.h"
#include "../../helper/bbuf.h"
#include "../../helper/runtime-util.h"
#include "../shadowop/symbolic-op.h"
#include "../shadowop/mathreplace.h"
#include <math.h>
#include <inttypes.h>

Stack* leafCExprs;
Stack* branchCExprs[MAX_BRANCH_ARGS];
Xarray_H(Stack*, StackArray);
Xarray_Impl(Stack*, StackArray);
Xarray_H(char*, VarList);
Xarray_Impl(char*, VarList);
StackArray concGraftStacks;

const char* varnames[] = {"x", "y", "z", "a", "b", "c",
                          "i", "j", "k", "l", "m", "n"};
#define MAX_FOLD_DEPTH (max_expr_block_depth * 2)

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
              groupsWithoutNonVars(expr, expr->branch.groups) :        \
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
                groupsWithoutNonVars(expr, expr->branch.groups) :     \
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
  stack_push(branchCExprs[expr->branch.nargs - 1], (void*)expr);
  pushConcGraftStack(expr->grafts, expr->ngrafts);
}
void recursivelyDisownConcExpr(ConcExpr* expr, int depth){
  if (depth == 0) return;
  tl_assert2(expr->ref_count > 0,
             "The ref count of %p is already zero, and we're trying to decrease it!\n",
             expr);
  if (print_expr_refs){
    VG_(printf)("Decreasing ref count of expr %p from %d to %d\n",
                expr, expr->ref_count, expr->ref_count - 1);
  }
  (expr->ref_count)--;
  if (expr->ref_count == 0){
    if (print_expr_refs){
      VG_(printf)("No references left for expr %p! Freeing...\n", expr);
    }
    if (expr->type == Node_Leaf){
      stack_push(leafCExprs, (void*)expr);
    } else {
      freeBranchConcExpr(expr);
    }
  }
  if (expr->type == Node_Branch){
    for(int i = 0; i < expr->branch.nargs; ++i){
      recursivelyDisownConcExpr(expr->branch.args[i], depth - 1);
    }
  }
}
void disownConcExpr(ConcExpr* expr){
  recursivelyDisownConcExpr(expr, max_expr_block_depth * 2);
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

void recursivelyOwnConcExpr(ConcExpr* expr, int depth){
  if (depth == 0) return;
  if (print_expr_refs){
    VG_(printf)("Increasing ref count of expr %p from %d to %d\n",
                expr, expr->ref_count, expr->ref_count + 1);
  }
  (expr->ref_count)++;
  if (expr->type == Node_Branch){
    for(int i = 0; i < expr->branch.nargs; ++i){
      recursivelyOwnConcExpr(expr->branch.args[i], depth - 1);
    }
  }
}

ConcExpr* mkBranchConcExpr(double value, ShadowOpInfo* op,
                           int nargs, ConcExpr** args){
  ConcExpr* result;
  if (stack_empty(branchCExprs[nargs-1])){
    result = VG_(malloc)("expr", sizeof(ConcExpr));
    result->branch.args = VG_(malloc)("expr args", sizeof(ConcExpr*) * nargs);
    result->branch.nargs = nargs;
    result->type = Node_Branch;
  } else {
    result = (void*)stack_pop(branchCExprs[nargs-1]);
  }
  // We'll do ownership stuff at the end, leave it at 0 refs for now.
  if (print_expr_refs){
    VG_(printf)("Making new expression %p with 0 references\n", result);
  }
  result->ref_count = 0;
  result->value = value;
  result->branch.op = op;

  for(int i = 0; i < nargs; ++i){
    result->branch.args[i] = args[i];
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
  recursivelyOwnConcExpr(result, max_expr_block_depth * 2);
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

  if (cexpr->type == Node_Leaf){
    result = mkFreshSymbolicLeaf(True, cexpr->value);
  } else {
    result->isConst = True;
    result->constVal = cexpr->value;
    result->type = Node_Branch;
    result->branch.op = cexpr->branch.op;
    tl_assert2(result->branch.op->op_code == 0 ||
               (result->branch.op->op_code > IEop_INVALID &&
                result->branch.op->op_code < IEop_REALLY_LAST_FOR_REAL_GUYS),
               "Bad IR Op %d on op %p",
               result->branch.op->op_code);
    result->branch.nargs = cexpr->branch.nargs;
    result->branch.args =
      VG_(perm_malloc)(sizeof(SymbExpr*) * cexpr->branch.nargs,
                       vg_alignof(SymbExpr*));
    for(int i = 0; i < cexpr->branch.nargs; ++i){
      ConcExpr* arg = cexpr->branch.args[i];
      if (arg->type == Node_Leaf){
        result->branch.args[i] = mkFreshSymbolicLeaf(True, arg->value);
      } else {
        tl_assert(arg->branch.op->expr != NULL);
        result->branch.args[i] = arg->branch.op->expr;
      }
    }
    result->ngrafts = cexpr->ngrafts;
    result->grafts =
      VG_(perm_malloc)(sizeof(SymbGraft) * cexpr->ngrafts,
                       vg_alignof(SymbGraft));
    for(int i = 0; i < cexpr->ngrafts; ++i){
      if (cexpr->grafts[i].parent == cexpr){
        result->grafts[i].parent = result;
      } else {
        ConcExpr* cparent = cexpr->grafts[i].parent;
        if (cparent->type == Node_Leaf){
          result->grafts[i].parent =
            mkFreshSymbolicLeaf(True, cparent->value);
        } else {
          tl_assert(cparent->branch.op->expr != NULL);
          result->grafts[i].parent = cparent->branch.op->expr;
        }
      }
      result->grafts[i].childIndex = cexpr->grafts[i].childIndex;
    }
    result->branch.groups = getExprsEquivGroups(cexpr, result);
    initializeProblematicRanges(result);
  }
  return result;
}

int hasRepeatedVars(SymbExpr* expr){
  tl_assert(expr->type == Node_Branch);
  GroupList trimmedGroups =
    groupsWithoutNonVars(expr, expr->branch.groups);
  int result = numRepeatedVars(expr, trimmedGroups);
  freeXA(GroupList)(trimmedGroups);
  return result;
}

int isUnderneathGroupMember(NodePos position, Group group);
int isUnderneathGroupMember(NodePos position, Group group){
  for(Group curNode = group; curNode != NULL; curNode = curNode->next){
    NodePos memberPosition = curNode->item;
    if (position->len <= memberPosition->len) continue;
    int isUnderneath = 1;
    for(int i = 0; i < memberPosition->len; ++i){
      if (position->data[i] != memberPosition->data[i]){
        isUnderneath = 0;
        break;
      }
    }
    if (!isUnderneath) {
      continue;
    }
    return 1;
  }
  return 0;
}

Group pruneGroupToTop(Group group);
Group pruneGroupToTop(Group group){
  Group result = NULL;
  for(Group curNode = group; curNode != NULL; curNode = curNode->next){
    if (!isUnderneathGroupMember(curNode->item, group)){
      lpush(Group)(&result, curNode->item);
    }
  }
  return result;
}

Group pruneGroupToValidVars(Group group, SymbExpr* structure);
Group pruneGroupToValidVars(Group group, SymbExpr* structure){
  Group result = NULL;
  for(Group curNode = group; curNode != NULL; curNode = curNode->next){
    SymbExpr* curExpr = structure;
    NodePos pos = curNode->item;
    for(int i = 0; i < pos->len; ++i){
      if (curExpr->type == Node_Leaf){
        goto next;
      }
      if (curExpr->branch.nargs <= pos->data[i]){
        goto next;
      }
      if (sound_simplify){
        switch((int)curExpr->branch.op->op_code){
        case Iop_Mul32F0x4:
        case Iop_Mul64F0x2:
        case Iop_Mul32Fx8:
        case Iop_Mul64Fx4:
        case Iop_Mul32Fx4:
        case Iop_Mul64Fx2:
        case Iop_MulF64:
        case Iop_MulF128:
        case Iop_MulF32:
        case Iop_MulF64r32:{
          SymbExpr* arg0 = curExpr->branch.args[0];
          SymbExpr* arg1 = curExpr->branch.args[1];
          if ((arg0->isConst && arg0->constVal == 0.0) ||
              (arg1->isConst && arg1->constVal == 0.0)){
            goto next;
          }
        }
          break;
        default:
          break;
        }
      }
      curExpr = curExpr->branch.args[pos->data[i]];
    }
    if (!curExpr->isConst){
      lpush(Group)(&result, curNode->item);
    }
  next:;
  }
  return result;
}

SymbExpr* varSwallow(SymbExpr* expr){
  for(int i = 0; i < expr->branch.groups->size; ++i){
    Group curGroup = pruneGroupToValidVars(pruneGroupToTop(expr->branch.groups->data[i]),
                                           expr);
    if (length(Group)(&curGroup) < 1) continue;
    int num_nontrivial_group_members = 0;
    for(int j = 0; j < expr->branch.groups->size; ++j){
      if (i == j) continue;
      Group otherGroup = pruneGroupToValidVars(expr->branch.groups->data[j], expr);
      int num_members_underneath_cur_group = 0;
      int num_other_valid_group_members = 0;
      for(Group otherNode = otherGroup; otherNode != NULL; otherNode = otherNode->next){
        SymbExpr** target = symbExprPosGetRef(&expr, otherNode->item);
        if (target == NULL) continue;
        if ((*target)->isConst) continue;
        num_other_valid_group_members++;
        if (isUnderneathGroupMember(otherNode->item, curGroup)){
          num_members_underneath_cur_group++;
        }
      }
      if (num_members_underneath_cur_group > 0 &&
          num_members_underneath_cur_group < num_other_valid_group_members){
        num_nontrivial_group_members++;
      }
    }
    if (length(Group)(&curGroup) > 1 &&
        (num_nontrivial_group_members < 1 || unsound_var_swallow)){
      for(Group curNode = curGroup; curNode != NULL; curNode = curNode->next){
        SymbExpr** target = symbExprPosGetRef(&expr, curNode->item);
        if (target == NULL) continue;
        if ((*target)->type == Node_Leaf) continue;
        *target =
          mkFreshSymbolicLeaf((*target)->isConst, (*target)->constVal);
      }
    }
  }
  return expr;
}

// This function initializes a range table for the given symbolic
// expression. Range tables are maps from node positions to
// RangeRecord's, which should be an up-to-date record of the inputs
// at that position. Range tables should maintain the following
// invariant: there is an entry for exactly one member of each
// equivalence group in the symbolic expression, INCLUDING implicit
// equivalence groups. That means that if a node exists in the
// symbolic expression, and it's not in an explicit equivalence group
// (e.g. isn't in the GroupList), it should have it's own range map
// entry. For nodes that are part of explicit equivalence groups, the
// first node in the group will act as a stand-in for all nodes in the
// group. Keep in mind this means that when groups are split, new
// entries must be added to the range table.
void initializeProblematicRanges(SymbExpr* symbExpr){
  // This should be called after all other initialization of the
  // symbolic expression has been done, and only on symbolic
  // expressions which represent branch nodes. Leaf node expressions
  // don't have range maps or equivalence groups, since they would be
  // trivial.
  tl_assert(symbExpr->type == Node_Branch);


  // To make sure we properly adhere to the invariant mentioned above,
  // we'll keep track of which nodes have been seen in equivalence
  // groups, and should therefore only be added by group, not
  // seperately, to the range table.
  OSet* nodesInGroups = VG_(OSetWord_Create)(VG_(malloc), "varset", VG_(free));
  // Initialize the range table.
  symbExpr->branch.varProblematicRanges =
    VG_(HT_construct)("Variable problematic ranges table");

  // Part (a)

  // First thing we'll do is go through each group and add the first
  // members position to the range table.
  for(int i = 0; i < symbExpr->branch.groups->size; ++i){
    // Create a range map entry with an embedded range record for this
    // group. We won't actually give the range record any inputs yet,
    // because we don't know if the current inputs are problematic,
    // and if they are we'll update the ranges later in the symbolic
    // op with that info.
    addInitialRangeEntry(symbExpr->branch.varProblematicRanges,
                         symbExpr->branch.groups->data[i]->item);

    // Add every node in this group to the set of nodes in groups, so
    // that we don't add them again.
    for(Group curNode = symbExpr->branch.groups->data[i];
        curNode != NULL; curNode = curNode->next){
      VG_(OSetWord_Insert)(nodesInGroups, (UWord)(uintptr_t)curNode->item);
    }
  }

  // Part (b)

  // Now, there are some nodes not captured by groups, because
  // singleton groups get pruned (and sometimes not created in the
  // first place). So we're going to look at the problematic range
  // tables of our children, and use those to populate the rest of our
  // range table. This works for a kind of subtle reason. We can
  // assume inductively that our children have range tables which
  // satisfy the invariant, since they called this function before
  // us. This means they have an entry their range table for every
  // node below them, EXCEPT if that node is part of an equivalence
  // group and not the first element of that group. In this case we
  // know that every time our subexpression saw that node, it had an
  // identical value to a node that IS in our range table. Since the
  // times we see nodes underneath this subexpression are a subset of
  // the times that the subexpression sees them, that means that the
  // node is already part of a group that this expression has
  // too. Therefore, we would have already added it to our range table
  // in the step above.
  for (int i = 0; i < symbExpr->branch.nargs; ++i){
    SymbExpr* childNode = symbExpr->branch.args[i];
    // We also need to add our children themselves directly to our
    // range table, since they aren't in their own range tables.

    // We'll add them indexed by the singleton position of their child
    // index, since they are only one level down from us.
    NodePos childPos = rconsPos(null_pos, i);
    // If they are equivalent to something else and we already added
    // them, don't add them again.
    if (!(VG_(OSetWord_Contains)(nodesInGroups, (UWord)(uintptr_t)childPos))){
      addInitialRangeEntry(symbExpr->branch.varProblematicRanges, childPos);
    }
    // If our child is a branch, pull in it's range table without the
    // actual ranges, since the ranges themselves are
    // context-sensitive.
    if (childNode->type == Node_Branch){
      VgHashTable* childMap = childNode->branch.varProblematicRanges;
      VG_(HT_ResetIter)(childMap);
      RangeMapEntry* entry;
      while((entry = VG_(HT_Next)(childMap)) != NULL){
        NodePos childEntryPos = rconsPos(entry->position, i);

        // Again, only pull entries in if we DIDN'T add them in part (a).
        if (!(VG_(OSetWord_Contains)(nodesInGroups, (UWord)(uintptr_t)childEntryPos))){
          addInitialRangeEntry(symbExpr->branch.varProblematicRanges, childEntryPos);
        }
      }
    }
  }
}

void addRangeEntryCopy(VgHashTable* rangeMap, NodePos position, RangeRecord* original){
  RangeMapEntry* newEntry =
    VG_(malloc)("variable range map entry", sizeof(RangeMapEntry));
  copyRangeRecordInPlace(&(newEntry->range_rec), original);

  newEntry->positionHash = hashPosition(position);
  newEntry->position = position;
  VG_(HT_add_node)(rangeMap, newEntry);
}
void addInitialRangeEntry(VgHashTable* rangeMap, NodePos position){
  RangeMapEntry* newEntry =
    VG_(malloc)("variable range map entry", sizeof(RangeMapEntry));
  initRangeRecord(&(newEntry->range_rec));

  newEntry->positionHash = hashPosition(position);
  newEntry->position = position;
  VG_(HT_add_node)(rangeMap, newEntry);
}

List_H(RangeMapEntry*, RangeEntryList);
List_Impl(RangeMapEntry*, RangeEntryList);

void updateProblematicRanges(SymbExpr* symbExpr, ConcExpr* cexpr){
  VgHashTable* rangeTable = symbExpr->branch.varProblematicRanges;
  RangeEntryList expiredEntries = NULL;

  VG_(HT_ResetIter)(rangeTable);
  RangeMapEntry* entry;
  while((entry = VG_(HT_Next)(rangeTable)) != NULL){
    ConcExpr* sampleConcNode = concExprPosGet(cexpr, entry->position);
    // The node mentioned by this table entry might not exist in the
    // concrete expression we're generalizing with, in which case it
    // won't exist anymore in the symbolic expression either. If
    // that's the case, we'll remove it from the range table so that
    // we don't bother trying to maintain it later.
    if (sampleConcNode == NULL){
      // Unfortunately, we can't remove the node directly from the
      // table since that would violate the iterator invariants, so
      // we'll just add it to a stack to be removed at the end of the
      // update.
      lpush(RangeEntryList)(&expiredEntries, entry);
    } else {
      // If the node DOES exist, we should update the range table entry.
      updateRangeRecord(&(entry->range_rec), sampleConcNode->value);
    }
  }
  // Remove all the expired entries.
  while(length(RangeEntryList)(&expiredEntries) > 0){
    RangeMapEntry* expiredEntry = lpop(RangeEntryList)(&expiredEntries);
    VG_(HT_gen_remove)(rangeTable, expiredEntry, cmp_position);
  }
}

RangeRecord* lookupRangeRecord(VgHashTable* rangeMap, NodePos position){
  RangeMapEntry key = {.position = position,
                       .positionHash = hashPosition(position)};
  RangeMapEntry* entry =
    VG_(HT_gen_lookup)(rangeMap, &key, cmp_position);
  if (entry == NULL) return NULL;
  return &(entry->range_rec);
}

void recursivelyPopulateRanges(RangeRecord* totalRanges, RangeRecord* problematicRanges,
                               SymbExpr* curExpr, int* nextVarIdx, NodePos curPos,
                               OSet* seenNodes, VgHashTable* rangeTable, int max_depth);
void getRanges(RangeRecord** totalRangesOut, RangeRecord** problematicRangesOut,
               SymbExpr* expr, int num_vars){
  tl_assert(expr->type == Node_Branch);
  OSet* seenNodes = VG_(OSetWord_Create)(VG_(malloc), "Seen Nodes",
                                         VG_(free));

  *totalRangesOut = VG_(malloc)("expr ranges", sizeof(RangeRecord) * num_vars);
  *problematicRangesOut = VG_(malloc)("problematic ranges",
                                      sizeof(RangeRecord) * num_vars);
  int nextVarIdx = 0;

  GroupList groups = expr->branch.groups;
  (void)groups;
  (void)nextVarIdx;
  for(int i = 0; i < groups->size; ++i){
    Group curGroup = groups->data[i];

    // This chunk of code gets the first position in the group which
    // still exists in the tree. If every position in the group
    // doesn't exist anymore, then we skip this iteration of the for
    // loop.
    NodePos samplePos;
    {
      Group curNode = curGroup;
      for(curNode = curGroup;
          curNode != NULL && symbExprPosGet(expr, curNode->item) == NULL;
          curNode = curNode->next);
      if (curNode == NULL){
        continue;
      }
      samplePos = curNode->item;
    }

    SymbExpr* sampleParent = symbExprPosGet(expr, rtail(samplePos));

    int childIndex = samplePos->data[samplePos->len - 1];
    (*totalRangesOut)[nextVarIdx] =
      sampleParent->branch.op->agg.inputs.range_records[childIndex];
    VgHashTable* rangeTable = expr->branch.varProblematicRanges;
    RangeRecord* entry = lookupRangeRecord(rangeTable, samplePos);
    (*problematicRangesOut)[nextVarIdx] = *entry;
    nextVarIdx++;

    for(Group curNode = curGroup; curNode != NULL; curNode = curNode->next){
      VG_(OSetWord_Insert)(seenNodes, (UWord)(uintptr_t)curNode->item);
    }
  }

  recursivelyPopulateRanges(*totalRangesOut, *problematicRangesOut,
                            expr, &nextVarIdx, null_pos,
                            seenNodes, expr->branch.varProblematicRanges,
                            MAX_FOLD_DEPTH);
  VG_(OSetWord_Destroy)(seenNodes);
}

void recursivelyPopulateRanges(RangeRecord* totalRanges, RangeRecord* problematicRanges,
                               SymbExpr* curExpr, int* nextVarIdx, NodePos curPos,
                               OSet* seenNodes, VgHashTable* rangeTable, int max_depth){
  tl_assert(curExpr->type == Node_Branch);
  for(int i = 0; i < curExpr->branch.nargs; ++i){
    SymbExpr* childExpr = curExpr->branch.args[i];
    NodePos childPos = rconsPos(curPos, i);
    if (childExpr->type == Node_Leaf || max_depth == 1){
      if (!childExpr->isConst &&
          !(VG_(OSetWord_Contains)(seenNodes, (UWord)(uintptr_t)childPos))){
        totalRanges[*nextVarIdx] = curExpr->branch.op->agg.inputs.range_records[i];
        problematicRanges[*nextVarIdx] =
          *(lookupRangeRecord(rangeTable, rconsPos(curPos, i)));
        (*nextVarIdx)++;
      }
    } else {
      recursivelyPopulateRanges(totalRanges, problematicRanges,
                                childExpr, nextVarIdx, rconsPos(curPos, i),
                                seenNodes, rangeTable, max_depth - 1);
    }
  }
}

void ppRangeTable(VgHashTable* rangeTable){
  VG_(HT_ResetIter)(rangeTable);
  RangeMapEntry* entry;
  while((entry = VG_(HT_Next)(rangeTable)) != NULL){
    ppNodePos(entry->position);
    VG_(printf)(" -> [%f, %f]\n",
                entry->range_rec.neg_range.min,
                entry->range_rec.pos_range.max);
  }
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
      ppConcExpr(expr->branch.args[i]);
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
  char* stringRep = symbExprToString(expr, NULL);
  tl_assert(stringRep != NULL);
  VG_(printf)("%s", stringRep);
  VG_(free)(stringRep);
}
#define MAX_EXPR_LEN 2000000000
typedef enum {
  COLOR_RED,
  COLOR_BLUE,
  COLOR_GREEN,
  COLOR_LAST,
} Color;
void printColorCode(BBuf* buffer, Color color);
void printColorCode(BBuf* buffer, Color color){
  switch(color){
  case COLOR_BLUE:
    printBBuf(buffer, "\033[1;34m");
    break;
  case COLOR_RED:
    printBBuf(buffer, "\033[1;31m");
    break;
  case COLOR_GREEN:
    printBBuf(buffer, "\033[1;32m");
  default:
    break;
  }
}
void recursivelyToString(SymbExpr* expr, BBuf* buf, VarMap* varMap,
                         const char* parent_func, Color curColor,
                         NodePos curPos, int max_depth);
char* symbExprToString(SymbExpr* expr, int* numVarsOut){
  if (no_exprs){
    char* buf = VG_(malloc)("buffer data", 2);
    buf[0] = '\0';
    if (numVarsOut != NULL){
      *numVarsOut = 0;
    }
    return buf;
  }
  if (expr->type == Node_Leaf){
    if (expr->isConst){
      char* _buf = VG_(malloc)("buffer data", MAX_EXPR_LEN);
      BBuf* bbuf = mkBBuf(MAX_EXPR_LEN, _buf);
      printBBuf(bbuf, "%f", expr->constVal);
      VG_(realloc_shrink)(_buf, MAX_EXPR_LEN - bbuf->bound + 10);
      VG_(free)(bbuf);
      return _buf;
    } else {
      int len = VG_(strlen)(varnames[0]);
      char* buf = VG_(malloc)("expr string", len + 1);
      VG_(memcpy)(buf, varnames[0], len + 1);
      if (numVarsOut != NULL){
        *numVarsOut = 1;
      }
      return buf;
    }
  } else {
    VarMap* varMap =
      mkVarMap(groupsWithoutNonVars(expr, expr->branch.groups));
    const char* toplevel_func;
    if (!VG_(get_fnname)(expr->branch.op->op_addr, &toplevel_func)){
      toplevel_func = "none";
    }
    char* _buf = VG_(malloc)("buffer data", MAX_EXPR_LEN);
    BBuf* bbuf = mkBBuf(MAX_EXPR_LEN, _buf);
    if (expr_colors){
      printColorCode(bbuf, COLOR_BLUE);
    }
    recursivelyToString(expr, bbuf, varMap,
                        toplevel_func, COLOR_BLUE,
                        null_pos,
                        MAX_FOLD_DEPTH);
    if (expr_colors){
      printBBuf(bbuf, "\033[0m");
    }
    VG_(realloc_shrink)(_buf,
                        MAX_EXPR_LEN - bbuf->bound + 10);
    VG_(free)(bbuf);
    int numVars = countVars(varMap);
    if (numVarsOut != NULL){
      *numVarsOut = numVars;
    }
    freeVarMap(varMap);
    return _buf;
  }
}
void recursivelyToString(SymbExpr* expr, BBuf* buf, VarMap* varMap,
                         const char* parent_func, Color curColor,
                         NodePos curPos, int max_depth){
  if (max_depth == 0){
    printBBuf(buf, " _");
  } else if (expr->type == Node_Leaf){
    if (expr->isConst){
      printBBuf(buf, " ");
      printBBufFloat(buf, expr->constVal);
    } else {
      printBBuf(buf, " %s", getVar(lookupVar(varMap, curPos)));
    }
  } else {
    if (sound_simplify){
      switch((int)expr->branch.op->op_code){
      case Iop_Mul32F0x4:
      case Iop_Mul64F0x2:
      case Iop_Mul32Fx8:
      case Iop_Mul64Fx4:
      case Iop_Mul32Fx4:
      case Iop_Mul64Fx2:
      case Iop_MulF64:
      case Iop_MulF128:
      case Iop_MulF32:
      case Iop_MulF64r32:
        {
          SymbExpr* arg0 = expr->branch.args[0];
          SymbExpr* arg1 = expr->branch.args[1];
          if (arg0->isConst && arg0->constVal == 1.0){
            recursivelyToString(arg1, buf, varMap,
                                parent_func, curColor,
                                rconsPos(curPos, 1),
                                max_depth - 1);
            return;
          }
          if (arg1->isConst && arg1->constVal == 1.0){
            recursivelyToString(arg0, buf, varMap,
                                parent_func, curColor,
                                rconsPos(curPos, 0),
                                max_depth - 1);
            return;
          }
          if ((arg0->isConst && arg0->constVal == 0.0) ||
              (arg1->isConst && arg1->constVal == 0.0)){
            printBBuf(buf, "%f", 0.0);
            return;
          }
          break;
        }
      case Iop_Add32Fx2:
      case Iop_Add32F0x4:
      case Iop_Add64F0x2:
      case Iop_Add32Fx8:
      case Iop_Add64Fx4:
      case Iop_Add32Fx4:
      case Iop_Add64Fx2:
      case Iop_AddF128:
      case Iop_AddF64:
      case Iop_AddF32:
      case Iop_AddF64r32:
        {
          SymbExpr* arg0 = expr->branch.args[0];
          SymbExpr* arg1 = expr->branch.args[1];
          if (arg0->isConst && arg0->constVal == 0.0){
            recursivelyToString(arg1, buf, varMap,
                                parent_func, curColor,
                                rconsPos(curPos, 1),
                                max_depth - 1);
            return;
          }
          if (arg1->isConst && arg1->constVal == 0.0){
            recursivelyToString(arg0, buf, varMap,
                                parent_func, curColor,
                                rconsPos(curPos, 0),
                                max_depth - 1);
            return;
          }
          break;
        }
      case Iop_Sub32Fx2:
      case Iop_Sub32F0x4:
      case Iop_Sub64F0x2:
      case Iop_Sub32Fx8:
      case Iop_Sub64Fx4:
      case Iop_Sub32Fx4:
      case Iop_Sub64Fx2:
      case Iop_SubF128:
      case Iop_SubF64:
      case Iop_SubF32:
      case Iop_SubF64r32:
        {
          SymbExpr* arg0 = expr->branch.args[0];
          SymbExpr* arg1 = expr->branch.args[1];
          if (arg0->isConst && arg0->constVal == 0.0){
            const char* fnname;
            if (!(VG_(get_fnname)(expr->branch.op->op_addr, &fnname))){
              fnname = "none";
            }
            if (VG_(strcmp)(fnname, parent_func)){
              curColor = (curColor + 1) % COLOR_LAST;
              if (expr_colors){
                printColorCode(buf, curColor);
              }
              if (print_subexpr_locations){
                char* addrString =
                  getAddrString(expr->branch.op->op_addr);
                printBBuf(buf, "{%s}", addrString);
                VG_(free)(addrString);
              }
            }
            printBBuf(buf, " (-");
            recursivelyToString(arg1, buf, varMap,
                                parent_func, curColor,
                                rconsPos(curPos, 1),
                                max_depth - 1);
            printBBuf(buf, ")");
            return;
          }
          if (arg1->isConst && arg1->constVal == 0.0){
            recursivelyToString(arg0, buf, varMap,
                                parent_func, curColor,
                                rconsPos(curPos, 0),
                                max_depth - 1);
            return;
          }
        }
      default:
        break;
      }
    }
    printBBuf(buf, " ");

    const char* fnname;
    if (!(VG_(get_fnname)(expr->branch.op->op_addr, &fnname))){
      fnname = "none";
    }
    if (VG_(strcmp)(fnname, parent_func)){
      curColor = (curColor + 1) % COLOR_LAST;
      if (expr_colors){
        printColorCode(buf, curColor);
      }
      if (print_subexpr_locations){
        char* addrString = getAddrString(expr->branch.op->op_addr);
        printBBuf(buf, "{%s}", addrString);
        VG_(free)(addrString);
      }
    } else if (shortmark_all_exprs){
      printBBuf(buf, "{%lX}", expr->branch.op->op_addr);
    }

    printBBuf(buf, "(%s", opSym(expr->branch.op));

    for(int i = 0; i < expr->branch.nargs; i++){
      recursivelyToString(expr->branch.args[i], buf, varMap,
                          fnname, curColor,
                          rconsPos(curPos, i),
                          max_depth - 1);
      if (expr_colors){
        printColorCode(buf, curColor);
      }
    }
    printBBuf(buf, ")");
  }
}
FoldBlock_H(int);
FoldBlock_Impl(int);
FoldBlock_H_Named(BBuf*, BBuf);
FoldBlock_Impl_Named(BBuf*, BBuf);
int countVars(VarMap* map){
  OSet* vars = VG_(OSetWord_Create)(VG_(malloc), "varset",
                                    VG_(free));
  int result = 0;
  VG_(HT_ResetIter)(map->existingEntries);
  VarMapEntry* entry;
  while((entry = VG_(HT_Next)(map->existingEntries)) != NULL){
    if (!(VG_(OSetWord_Contains)(vars, entry->varIdx))){
      result++;
      VG_(OSetWord_Insert)(vars, entry->varIdx);
    }
  }
  VG_(OSetWord_Destroy)(vars);
  return result;
}
int numRepeatedVars(SymbExpr* expr, GroupList trimmedGroups){
  int acc = 0;
  for(int i = 0; i < trimmedGroups->size; ++i){
    for(Group curNode = trimmedGroups->data[i];
        curNode != NULL; curNode = curNode->next){
      SymbExpr* exprNode = symbExprPosGet(expr, curNode->item);
      if (exprNode->type == Node_Leaf &&
          !exprNode->isConst && curNode->item->len < MAX_FOLD_DEPTH){
        acc += 1;
      }
    }
    acc -= 1;
  }
  return acc;
}
char* symbExprVarString(int numVars){
  char* buf = NULL;
  switch(numVars){
  case 0:{
    char noVars[] = "()";
    buf = VG_(malloc)("var string", sizeof(noVars));
    VG_(strcpy)(buf, noVars);
  }
    break;
  case 1:{
    int buflen = VG_(strlen)(varnames[0] + 3);
    buf = VG_(malloc)("var string", sizeof(char) * buflen);
    VG_(snprintf)(buf, buflen, "(%s)", varnames[0]);
  }
    break;
  default:{
    int buflen = 3;
    for(int i = 0; i < numVars; ++i){
      buflen += VG_(strlen)(getVar(i)) + 1;
    }
    buf = VG_(malloc)("var string", buflen * sizeof(char));
    tl_assert(VG_(strlen)(varnames[0]) == 1);
    int count = 0;
    count += VG_(snprintf)(buf + count, buflen - count,
                           "(%s ", varnames[0]);
    for(int i = 1; i < numVars - 1; ++i){
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

void ppEquivGroups(GroupList groups){
  for(int i = 0; i < groups->size; ++i){
    Group g = groups->data[i];
    ppEquivGroup(g);
    VG_(printf)("\n");
  }
}
void ppEquivGroup(Group group){
  VG_(printf)("{ ");
  for(Group curNode = group; curNode != NULL; curNode = curNode->next){
    ppNodePos(curNode->item);
    VG_(printf)(" ");
  }
  VG_(printf)("}");
}
