/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie          symbolic-op.c ---*/
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

#include "symbolic-op.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_xarray.h"
#include "../../helper/runtime-util.h"

#define GENERALIZE_DEPTH 2

inline
ConcExpr* concGraftChild(ConcGraft graft);

inline
SymbExpr* symbGraftChild(SymbGraft graft);

inline
ConcExpr* concGraftChild(ConcGraft graft){
  return graft.parent->branch.args[graft.childIndex];
}
inline
SymbExpr* symbGraftChild(SymbGraft graft){
  return graft.parent->branch.args[graft.childIndex];
}

void execSymbolicOp(ShadowOpInfo* opinfo, ConcExpr** result,
                    double computedResult, ShadowValue** args,
                    Bool problematic){
  if (no_exprs){
    return;
  }
  ConcExpr* exprArgs[MAX_BRANCH_ARGS];
  int nargs = numArgs(opinfo->op_code);
  for(int i = 0; i < nargs; ++i){
    exprArgs[i] = args[i]->expr;
  }
  *result = mkBranchConcExpr(computedResult, opinfo,
                             nargs, exprArgs);
  generalizeSymbolicExpr(&(opinfo->expr), *result);
  if (problematic){
    updateProblematicRanges(opinfo->expr, *result);
  }
}

void generalizeSymbolicExpr(SymbExpr** symbexpr, ConcExpr* cexpr){
  if (*symbexpr == NULL){
    *symbexpr = concreteToSymbolic(cexpr);
    if (print_expr_updates){
      VG_(printf)("Created expression %p ", *symbexpr);
      ppSymbExpr(*symbexpr);
      VG_(printf)("\n");
    }
  } else {
    if (print_expr_updates){
      VG_(printf)("Merging %p (op %p) ", *symbexpr, (*symbexpr)->branch.op);
      ppSymbExpr(*symbexpr);
      VG_(printf)(" with %p (op %p) ", cexpr, cexpr->branch.op);
      ppConcExpr(cexpr);
      VG_(printf)("\n");
    }
    if ((*symbexpr)->type == Node_Branch &&
        (cexpr->type == Node_Leaf ||
         cexpr->branch.op != (*symbexpr)->branch.op)){
      (*symbexpr) =
        mkFreshSymbolicLeaf((*symbexpr)->isConst &&
                            (*symbexpr)->constVal == cexpr->value,
                            (*symbexpr)->constVal);
    } else {
      if ((*symbexpr)->isConst){
        if ((*symbexpr)->constVal != (*symbexpr)->constVal){
          (*symbexpr)->constVal = cexpr->value;
        } else if (!NaNSafeEquals((*symbexpr)->constVal, cexpr->value) &&
                   cexpr->value == cexpr->value){
          (*symbexpr)->isConst = False;
        }
      }
      generalizeStructure(*symbexpr, cexpr, GENERALIZE_DEPTH);
      if ((*symbexpr)->type == Node_Branch){
        intersectEqualities(*symbexpr, cexpr);
      }
    }
    if (print_expr_updates){
      VG_(printf)("Updated expression %p to ", *symbexpr);
      ppSymbExpr(*symbexpr);
      VG_(printf)("\n");
    }
  }
}

void addValEntry(VgHashTable* valmap, double val, int groupIdx){
  ValMapEntry* entry = VG_(malloc)("val map entry", sizeof(ValMapEntry));
  entry->valHash = hashValue(val);
  entry->val = val;
  entry->groupIdx = groupIdx;
  VG_(HT_add_node)(valmap, entry);
}
int lookupVal(VgHashTable* valmap, double val){
  ValMapEntry key = {.val = val,
                     .valHash = hashValue(val)};
  ValMapEntry* entry = VG_(HT_gen_lookup)(valmap, &key, cmp_value);
  if (entry == NULL){
    return -1;
  } else {
    return entry->groupIdx;
  }
}
UWord hashValue(double val){
  return *(UWord*)&val;
}
Word cmp_value(const void* node1, const void* node2){
  const ValMapEntry* entry1 = (const ValMapEntry*)node1;
  const ValMapEntry* entry2 = (const ValMapEntry*)node2;
  if (entry1->val == entry2->val){
    return 0;
  } else {
    return 1;
  }
}

void generalizeStructure(SymbExpr* symbExpr, ConcExpr* concExpr,
                         int depth){
  if (depth == 0){
    return;
  }
  for(int i = 0; i < symbExpr->ngrafts; ++i){
    SymbGraft curSymbGraft = symbExpr->grafts[i];
    ConcGraft curConcGraft = concExpr->grafts[i];
    SymbExpr* symbMatch = symbGraftChild(curSymbGraft);
    ConcExpr* concMatch = concGraftChild(curConcGraft);

    // Check grafts for cutting.
    if (symbMatch->type == Node_Branch){
      if (concMatch->type == Node_Leaf ||
          concMatch->branch.op != symbMatch->branch.op){
        curSymbGraft.parent->branch.args[curSymbGraft.childIndex] =
          mkFreshSymbolicLeaf(symbMatch->isConst,
                              symbMatch->constVal);
        symbMatch = symbGraftChild(curSymbGraft);
        if (!generalize_to_constant){
          symbMatch->isConst = False;
        }
      }
    }
    // Check nodes for variance
    if (symbMatch->isConst){
      if (symbMatch->constVal != symbMatch->constVal){
        symbMatch->constVal = concMatch->value;
      } else if (!NaNSafeEquals(symbMatch->constVal, concMatch->value) &&
                 concMatch->value == concMatch->value){
        symbMatch->isConst = False;
      }
    }

    // Recurse
    if (concMatch->type == Node_Branch &&
        symbMatch->type == Node_Branch){
      generalizeStructure(symbMatch, concMatch, depth - 1);
    }
  }
}

void intersectEqualities(SymbExpr* symbExpr, ConcExpr* concExpr){
  tl_assert(concExpr->type == Node_Branch);
  tl_assert(symbExpr->type == Node_Branch);
  GroupList groups = symbExpr->branch.groups;
  GroupList newGroups = mkXA(GroupList)();
  for(int i = 0; i < groups->size; i++){
    Group curGroup = groups->data[i];
    NodePos canonicalPos = curGroup->item;
    Group newCurGroup = NULL;

    double canonicalValue = 0.0;
    VgHashTable* splitMap = NULL;

    while(curGroup != NULL){
      NodePos groupMemberPos = lpop(Group)(&curGroup);
      if (symbExprPosGet(symbExpr, groupMemberPos) == NULL){
        if (newCurGroup == NULL){
          if (curGroup == NULL){
            break;
          }
          RangeRecord* existingRange =
            lookupRangeRecord(symbExpr->branch.varProblematicRanges,
                              canonicalPos);
          addRangeEntryCopy(symbExpr->branch.varProblematicRanges,
                            curGroup->item, existingRange);
          addExampleEntryCopy(symbExpr->branch.exampleProblematicArgs,
                              curGroup->item,
                              lookupExampleInput(symbExpr->branch.exampleProblematicArgs,
                                                 canonicalPos));
          canonicalPos = curGroup->item;
        }
        continue;
      }
      double nodeValue = concExprPosGet(concExpr, groupMemberPos)->value;
      if (newCurGroup == NULL){
        canonicalValue = nodeValue;
        lpush(Group)(&(newCurGroup), groupMemberPos);
      } else {
        if (nodeValue != canonicalValue){
          if (splitMap == NULL){
            splitMap = VG_(HT_construct)("split map.\n");
          }
          int splitGroup = lookupVal(splitMap, nodeValue);
          if (splitGroup == -1){
            Group newSplitGroup = NULL;
            lpush(Group)(&newSplitGroup, groupMemberPos);
            addValEntry(splitMap, nodeValue, newGroups->size);
            XApush(GroupList)(newGroups, newSplitGroup);
            RangeRecord* existingRange =
              lookupRangeRecord(symbExpr->branch.varProblematicRanges,
                                canonicalPos);
            tl_assert(existingRange != NULL);
            addRangeEntryCopy(symbExpr->branch.varProblematicRanges,
                              groupMemberPos,
                              existingRange);
          addExampleEntryCopy(symbExpr->branch.exampleProblematicArgs,
                              groupMemberPos,
                              lookupExampleInput(symbExpr->branch.exampleProblematicArgs,
                                                 canonicalPos));
          } else {
            lpush(Group)(&(newGroups->data[splitGroup]), groupMemberPos);
          }
        } else {
          lpush(Group)(&(newCurGroup), groupMemberPos);
        }
      }
    }
    if (splitMap != NULL){
      VG_(HT_destruct)(splitMap, VG_(free));
    }
    XApush(GroupList)(newGroups, newCurGroup);
  }
  freeXA(GroupList)(symbExpr->branch.groups);
  symbExpr->branch.groups = pruneSingletonGroups(newGroups);
}

void getGrouped(GroupList groupList, VgHashTable* valMap,
                ConcExpr* concExpr, SymbExpr* symbExpr,
                NodePos curPos, int maxDepth);
void getGrouped(GroupList groupList, VgHashTable* valMap,
                ConcExpr* concExpr, SymbExpr* symbExpr,
                NodePos curPos, int maxDepth){
  tl_assert2(concExpr->ngrafts == symbExpr->ngrafts,
             "concrete expression has %d grafts, "
             "but symbolic expression has %d\n",
             concExpr->ngrafts, symbExpr->ngrafts);
  for(int i = 0; i < concExpr->ngrafts; ++i){
    ConcGraft curGraft = concExpr->grafts[i];
    SymbGraft symbGraft = symbExpr->grafts[i];
    ConcExpr* curNode = curGraft.parent;
    SymbExpr* symbNode = symbGraft.parent;
    NodePos graftPos = rconsPos(curPos, i);
    double value = concGraftChild(curGraft)->value;

    int existingEntry =
      lookupVal(valMap, value);
    int groupIdx;
    if (existingEntry == -1){
      groupIdx = groupList->size;
      addValEntry(valMap, concGraftChild(curGraft)->value, groupIdx);
      Group newGroup = NULL;
      XApush(GroupList)(groupList, newGroup);
    } else {
      groupIdx = existingEntry;
    }
    Group* groupLoc = &(groupList->data[groupIdx]);
    lpush(Group)(groupLoc, graftPos);

    ConcExpr* curChild = concGraftChild(curGraft);
    SymbExpr* symbChild = symbGraftChild(symbGraft);
    if (curNode->type == Node_Branch &&
        symbNode->type == Node_Branch &&
        curChild->type == Node_Branch &&
        symbChild->type == Node_Branch &&
        curChild->branch.op == symbChild->branch.op){
      if (maxDepth > 1){
        getGrouped(groupList, valMap,
                   curNode->branch.args[curGraft.childIndex],
                   symbNode->branch.args[curGraft.childIndex],
                   graftPos, maxDepth - 1);
      } else {
        for(int j = 0; j < symbChild->branch.groups->size; ++j){
          Group oldGroup = symbChild->branch.groups->data[j];
          Group newGroup = NULL;
          for(Group childItem = oldGroup; oldGroup != NULL;
              oldGroup = oldGroup->next){
            if (childItem->item->len <
                max_expr_block_depth){
              lpush(Group)(&newGroup,
                           appendPos(graftPos, childItem->item));
            }
          }
          XApush(GroupList)(groupList, newGroup);
        }
      }
    }
  }
}

GroupList pruneSingletonGroups(GroupList _list){
  if (_list->size == 0) return _list;
  GroupList newGroupList = mkXA(GroupList)();
  GroupList list = dedupGroups(_list);
  for(int i = 0; i < list->size; ++i){
    if (list->data[i] != NULL){
      if (list->data[i]->next != NULL){
        XApush(GroupList)(newGroupList, list->data[i]);
      }
    }
  }
  freeXA(GroupList)(list);
  return newGroupList;
}
GroupList dedupGroups(GroupList list){
  GroupList newGroupList = mkXA(GroupList)();
  for(int i = 0; i < list->size; ++i){
    Group newGroup = NULL;
    while(list->data[i] != NULL){
      NodePos curPos = lpop(Group)(&(list->data[i]));
      for(Group existingNode = newGroup; existingNode != NULL;
          existingNode = existingNode->next){
        if (existingNode->item == curPos){
          goto is_duplicate;
        }
      }
      lpush(Group)(&newGroup, curPos);
    is_duplicate:;
    }
    XApush(GroupList)(newGroupList, newGroup);
  }
  freeXA(GroupList)(list);
  return newGroupList;
}
GroupList groupsWithoutNonVars(SymbExpr* structure, GroupList list,
                               int max_depth){
  GroupList newGroupList = mkXA(GroupList)();
  if (list->size == 0) return newGroupList;
  for(int i = 0; i < list->size; ++i){
    for(Group curNode = list->data[i]; curNode != NULL;
        curNode = curNode->next){
      SymbExpr* target = symbExprPosGet(structure, curNode->item);
      // So the position could possibly be invalid, because of the
      // fact that expressions share structure, and prunings, but
      // don't share their equivalence maps, so those might get out of
      // date if a child expression decides to prune.
      if (target != NULL &&
          (target->type == Node_Leaf || curNode->item->len == max_depth) &&
          !target->isConst){
        XApush(GroupList)(newGroupList, list->data[i]);
        break;
      }
    }
  }
  return newGroupList;
}

int numTrackedNodes(GroupList glist);
int numTrackedNodes(GroupList glist){
  int count = 0;
  for(int i = 0; i < glist->size; ++i){
    count += length(Group)(&(glist->data[i]));
  }
  return count;
}

int groupsGetTimes=0;
GroupList getExprsEquivGroups(ConcExpr* concExpr, SymbExpr* symbExpr){
  GroupList groupList = mkXA(GroupList)();
  VgHashTable* valMap = VG_(HT_construct)("val map");
  getGrouped(groupList, valMap, concExpr, symbExpr,
             null_pos, max_expr_block_depth);
  VG_(HT_destruct)(valMap, VG_(free));
  GroupList prunedGroups = pruneSingletonGroups(groupList);
  return prunedGroups;
}
VarMap* mkVarMap(GroupList groups){
  VarMap* map = VG_(malloc)("var map", sizeof(VarMap));
  map->existingEntries = VG_(HT_construct)("var map table");
  map->nextVarIdx = 0;
  for(int i = 0; i < groups->size; ++i){
    Group curGroup = groups->data[i];
    int curVarIdx = map->nextVarIdx;
    map->nextVarIdx++;
    for(Group curNode = curGroup; curNode != NULL;
        curNode = curNode->next){
      VarMapEntry* entry = VG_(malloc)("var map entry",
                                       sizeof(VarMapEntry));
      entry->positionHash = hashPosition(curNode->item);
      entry->position = curNode->item;
      entry->varIdx = curVarIdx;
      VG_(HT_add_node)(map->existingEntries, entry);
    }
  }
  return map;
}

int lookupVar(VarMap* map, NodePos pos){
  VarMapEntry key = {.position = pos,
                     .positionHash = hashPosition(pos)};
  VarMapEntry* entry =
    VG_(HT_gen_lookup)(map->existingEntries, &key, cmp_position);
  if (entry == NULL){
    entry = VG_(malloc)("var map entry", sizeof(VarMapEntry));
    entry->position = pos;
    entry->positionHash = key.positionHash;
    entry->varIdx = map->nextVarIdx;
    VG_(HT_add_node)(map->existingEntries, entry);
    (map->nextVarIdx)++;
  }

  return entry->varIdx;
}

void freeVarMapEntry(void* entry);
void freeVarMapEntry(void* entry){
  VG_(free)(entry);
}

void freeVarMap(VarMap* map){
  VG_(HT_destruct)(map->existingEntries, freeVarMapEntry);
  VG_(free)(map);
}

ConcExpr* concExprPosGet(ConcExpr* expr, NodePos pos){
  ConcExpr* curExpr = expr;
  for(int i = 0; i < pos->len; ++i){
    if (curExpr->type == Node_Leaf){
      return NULL;
    }
    if (curExpr->ngrafts <= pos->data[i]){
      return NULL;
    }
    ConcGraft curGraft = curExpr->grafts[pos->data[i]];
    curExpr = curGraft.parent->branch.args[curGraft.childIndex];
  }
  return curExpr;
}
SymbExpr* symbExprPosGet(SymbExpr* expr, NodePos pos){
  SymbExpr* curExpr = expr;
  for(int i = 0; i < pos->len; ++i){
    if (curExpr->type == Node_Leaf){
      return NULL;
    }
    if (curExpr->branch.nargs <= pos->data[i]){
      return NULL;
    }
    curExpr = curExpr->branch.args[pos->data[i]];
  }
  return curExpr;
}
SymbExpr** symbExprPosGetRef(SymbExpr** expr, NodePos pos){
  SymbExpr** curExpr = expr;
  for(int i = 0; i < pos->len; ++i){
    if ((*curExpr)->type == Node_Leaf){
      return NULL;
    }
    if ((*curExpr)->ngrafts <= pos->data[i]){
      return NULL;
    }
    curExpr = &((*curExpr)->branch.args[pos->data[i]]);
  }
  return curExpr;
}

Word cmp_position(const void* node1, const void* node2){
  const VarMapEntry* entry1 = (const VarMapEntry*)node1;
  const VarMapEntry* entry2 = (const VarMapEntry*)node2;
  if (entry1->position->len != entry2->position->len){
    return 1;
  }
  for(SizeT i = 0; i < entry1->position->len; ++i){
    if (entry1->position->data[i] != entry2->position->data[i]){
      return 1;
    }
  }
  return 0;
}
