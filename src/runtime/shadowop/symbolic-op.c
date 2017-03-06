/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          symbolic-op.c ---*/
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

#include "symbolic-op.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcbase.h"

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

void generalizeSymbolicExpr(SymbExpr** symbexpr, ConcExpr* cexpr){
  if (*symbexpr == NULL){
    *symbexpr = concreteToSymbolic(cexpr);
  } else {
    generalizeStructure(*symbexpr, cexpr);
    intersectEqualities(*symbexpr, cexpr);
  }
}

int lookupVal(VgHashTable* valmap, double val){
  ValMapEntry key = {.val = val,
                      .valHash = hashValue(val)};
  ValMapEntry* entry = VG_(HT_gen_lookup)(valmap, &key, cmp_position);
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
  return entry1->val == entry2->val;
}

void generalizeStructure(SymbExpr* symbexpr, ConcExpr* concExpr){
  // TODO
}

void intersectEqualities(SymbExpr* symbexpr, ConcExpr* concExpr){
  // TODO
}

GroupList getConcExprEquivGroups(ConcExpr* cexpr){
  // TODO
  return NULL;
}
VarMap* mkVarMap(GroupList groups){
  // TODO
  return NULL;
}

int lookupVar(VarMap* map, NodePos pos){
  // TODO
  return 0;
}

void freeVarMapEntry(void* entry);
void freeVarMapEntry(void* entry){
  freePos(((VarMapEntry*)entry)->position);
}

void freeVarMap(VarMap* map){
  VG_(HT_destruct)(map->existingEntries, freeVarMapEntry);
  VG_(free)(map);
}

UWord hashPosition(NodePos node){
  UWord hash = 0;
  for(int i = 0; i < node.len; ++i){
    hash = 31 * hash + node.data[i] + 1;
  }
  return hash;
}
Word cmp_position(const void* node1, const void* node2){
  const VarMapEntry* entry1 = (const VarMapEntry*)node1;
  const VarMapEntry* entry2 = (const VarMapEntry*)node2;
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
