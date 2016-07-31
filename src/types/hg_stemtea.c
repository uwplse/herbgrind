
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            hg_stemtea.c ---*/
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

#include "hg_stemtea.h"

// This file was designed through several levels of code refinement
// and an executable spec, found in psuedo/aggregateTraces-*.rkt.

TeaNode* stemToTea(StemNode* stem){
  switch(stem->type){
  case Node_Leaf:
    {
      TeaNode* tea;
      ALLOC(tea, "hg.tea", 1, sizeof(TeaNode));
      tea->type = Node_Leaf;
      tea->hasConst = True;
      tea->constValue = stem->value;
      return tea;
    }
    break;
  case Node_Branch:
    {
      TeaNode* tea;
      ALLOC(tea, "hg.tea", 1, sizeof(TeaNode));
      tea->type = Node_Branch;
      tea->hasConst = True;
      tea->constValue = stem->value;
      tea->branch.op = stem->branch.op;
      tea->branch.nargs = stem->branch.nargs;
      ALLOC(tea->branch.args, "hg.tea args",
            tea->branch.nargs, sizeof(TeaNode*));
      for(int i = 0; i < tea->branch.nargs; ++i){
        tea->branch.args[i] = stemToTea(stem->branch.args[i]);
      }
      tea->branch.node_map = getStemEquivs(stem);
      return tea;
    }
    break;
  }
  return NULL;
}
void addStem(TeaNode* tea, StemNode* stem){
  generalizeStructure(tea, stem);
  if (tea->type == Node_Branch && stem->type == Node_Branch){
    pruneMapToStructure(tea);
    mergeBranchNodeMap(tea, stem);
  }
}
void generalizeStructure(TeaNode* tea, StemNode* stem){
  // If the value this node was initially assigned doesn't match that
  // of the new stem, then it isn't constant across all stems, so mark
  // it as such.
  if (tea->constValue != stem->value){
    tea->hasConst = False;
  }
  // Next, we'll need to figure out if this is a node that should
  // continue down, or be stopped right here. If either the current
  // tea, or the new stem stops here, the new tea should stop
  // here. And if they both continue, but their ops don't match, also
  // cut the new one off here.
  if (stem->type == Node_Leaf || tea->type == Node_Leaf ||
      tea->branch.op->op != stem->branch.op->op){
    if (tea->type == Node_Branch){
      VG_(free)(tea->branch.args);
      VG_(HT_destruct)(tea->branch.node_map, freeNodeMapEntry);
    }
    tea->type == Node_Leaf;
  } else {
    // Otherwise, we'll recurse on the descendents.
    for (int i = 0; i < tea->nargs; ++i){
      generalizeStructure(tea->args[i], stem->args[i]);
    }
  }
}
void mergeBranchNodeMap(TeaNode* tea, StemNode* stem){
  // Take the current node map, from the tea, which maps positions of
  // nodes to indices of equivalence classes, and turn it into a list
  // of lists, where the rows are the equivlance classes of positions.
  XArray* teaMapGroups = getGroups(tea->node_map);
  // Now, get the map from node positions to equivalence class indices
  // from the stem, to merge into the tea one.
  VgHashTable* stemMap = getStemEquivs(stem);
  // We're going to be splitting nodes off the tea equivalence groups
  // if they aren't equivalent in the stem map, so we'll need to
  // create new groups. Get the index of the next group we'll create.
  int nextGroupIndex = VG_(sizeXA)(teaMapGroups);
  // Go through each of the current tea equivalence classes.
  for (int groupIdx = 0;
       groupIdx < VG_(sizeXA)(teaMapGroups);
       ++groupIdx){
    XArray* positions = *(XArray**)VG_(indexXA)(teaMapGroups, i);
    if (VG_(sizeXA)(positions) != 0){
      // The split map is a map from equivalence group indices IN THE
      // STEM MAP to new equivalence group indices in the tea map.
      VgHashTable* splitMap = VG_(HT_construct)("split map");
      // Figure out what the first position in this group was mapped
      // to in the stem map, and map it to the current group, so that
      // any position in this group currently that is also in the same
      // group in the stem map stays in this group.
      NodeMapEntry* initialGroupEntry =
        VG_(HT_lookup)(stemMap, *(NodePos*)VG_(indexXA)(positions, 0));
      SplitMapEntry* initialSplitMapEntry;
      ALLOC(initialSplitMapEntry, "initial split map entry",
            1, sizeof(SplitMapEntry));
      initialSplitMapEntry->stemGroup = initialGroupEntry->varidx;
      initialSplitMapEntry->newGroup = groupIdx;
      // Now, go through the rest of the entries in the group.
      for (int posIdx = 0;
           posIdx < VG_(sizeXA)(positions);
           ++posIdx){
        NodePos position = *(NodePos*)VG_(indexXA)(positions, posIdx);
        // Figure out which group this node belongs to in the stem
        // map.
        NodeMapEntry* stemGroupEntry =
          VG_(HT_lookup)(stemMap, position);
        // Check to see if this node already has an entry in the split
        // map, and what it is if so. If this node matches previous
        // nodes in the group, it should already have an entry,
        // otherewise it was mapped to a group in the stem map that no
        // previous node was mapped to.
        SplitMapEntry* splitMapEntry =
          VG_(HT_lookup)(splitMap, stemGroupEntry->varidx);
        if (splitMapEntry != NULL){
          // If the stem group this node maps to already has an entry,
          // move it in the tea map if that entry isn't the group it's
          // already in.
          if (splitMapentry->newGroup != groupIdx){
            NodeMapEntry* newTeaGroupEntry;
            ALLOC(newTeaGroupEntry, "tea node map entry",
                  1, sizeof(NodeMapEntry));
            newTeaGroupEntry->position = position;
            newTeaGroupEntry->varidx = splitMapEntry->newGroup;
            VG_(HT_add_node)(tea->node_map, newTeaGroupEntry);
          }
        } else {
          // Otherwise, create a new group for it, and direct future
          // nodes in this existing tea group that match the stem
          // group to this new tea group.
          int newIdx = nextGroupIdx++;
          ALLOC(splitMapEntry, "split map entry",
                1, sizeof(SplitMapEntry));
          splitMapEntry->stemGroup = stemGroupEntry->varIdx;
          splitMapEntry->newGroup = newIdx;
          VG_(HT_add_node)(splitMap, splitMapEntry);

          NodeMapEntry* newTeaGroupEntry;
          ALLOC(newTeaGroupEntry, "tea group entry",
                1, sizeof(NodeMapEntry));
          newTeaGroupEntry->position = position;
          newTeaGroupEntry->varidx = newIdx;
          VG_(HT_add_node)(tea->node_map, newTeaGroupEntry);
        }
      }
      VG_(HT_destruct)(splitMap);
    }
    // Clean up after ourselves, deleting each group as we're done
    // with it.
    VG_(deleteXA)(positions);
  }
  // Finally, delete the group of groups, and the equivalence map we
  // got from the stem.
  VG_(deleteXA)(teaMapGroups);
  VG_(HT_destruct)(stemMap);
}
// Go through every entry in the node map, and remove those whose
// positions are invalid in the current structure.
void pruneMapToStructure(TeaNode* tea){
  UInt numNodes;
  NodeMapEntry** entries = VG_(HT_to_array)(tea->node_map, &numNodes);
  for(UInt i = 0; i < numNodes; ++i){
    NodePos position = entries[i]->position;
    if (!positionValid(tea, position)){
      VG_(HT_remove)(tea->node_map, position);
    }
  }
  VG_(free)(entries);
}
// Turn a map from positions to variable indices to a list of groups,
// where the group at each index cooresponds to the positions that map
// to that variable index.
XArray* getGroups(VgHashTable* node_map){
  XArray* groups = VG_(newXA)(VG_(malloc), "groups",
                              VG_(free), sizeof(XArray*));
  VG_(HT_ResetIter)(node_map);
  for(NodeMapEntry* entry = VG_(HT_Next)(node_map); entry != NULL;
      entry = VG_(HT_Next)(node_map)){
    while (entry->varidx >= VG_(sizeXA)(groups)){
      VG_(addToXA)(groups, VG_(newXA)(VG_(malloc), "group",
                                      VG_(free), sizeof(NodePos)));
    }
    VG_(addToXA)(VG_(indexXA)(groups, entry->varidx));
  }
  return groups;
}
// Check if a given position is valid in a particular tea structure.
Bool positionValid(TeaNode* tea, NodePos node){
  if (node.len == 0){
    return True;
  } else if (tea->type == Node_Leaf){
    return False;
  } else if (tea->nargs < node.data[0]) {
    return False;
  } else {
    return positionValid(tea->args[node.data[0]],
                         (NodePos) {.data = node.data, .len = node.len - 1});
  }
}
// Get a mapping from positions in the given stem to equivalence
// class/variable indices.
VgHashTable* getStemEquivs(StemNode* stem){
  // NodePos -> VarIdx
  VgHashTable* node_map = VG_(HT_construct)("node map");
  // Float -> VarIdx
  VgHashTable* val_map = VG_(HT_construct)("val map");
  int next_idx = 0;
  updateEquivMap(node_map, val_map, &next_idx, stem,
                 (NodePos) {.data = NULL, .len = 0});
  VG_(HT_destruct)(val_map, VG_(free));
  return node_map;
}
void updateEquivMap(VgHashTable* node_map,
                    VgHashTable* val_map,
                    int* next_idx,
                    StemNode* stem,
                    NodePos curPos){
  // Allocate a new node map entry for the current node.
  NodeMapEntry* newNodeEntry;
  ALLOC(newNodeEntry, "node map entry", 1, sizeof(NodeMapEntry));
  newNodeEntry->position = curPos;

  // Convert the value of the stem to a UWord key by literally
  // reinterpreting the bytes. Not clear that this will do the right
  // thing on 32-bit platforms, as it should pull the first 32-bits
  // out of the double for matching. Might overmatch as a result.
  UWord keyval = 0;
  VG_(memcpy)(&keyval, &stem->value, sizeof(UWord));
  ValMapEntry existing_entry = VG_(HT_lookup)(val_map, keyval);

  // If we already have an entry for this value, map the current
  // position to that.
  if (existing_entry != NULL){
    newNodeEntry->varidx = existing_entry->varidx;
  } else {
    // Otherwise, create a fresh index, map the current value to that
    // (so that later nodes that share this value will also get
    // mapped), and then map the current position to that.
    int new_idx = (*next_idx)++;

    ValMapEntry* newValEntry;
    ALLOC(newValEntry, "val map entry", 1, sizeof(ValMapEntry));
    newValEntry->key = keyval;
    newValEntry->varidx = new_idx;
    VG_(HT_add_node)(val_map, newValEntry);

    newNodeEntry->varidx = new_idx;
  }
  VG_(HT_add_node)(node_map, newNodeEntry);

  // Finally, if this is a branch recurse on the children.
  if (stem->type == Node_Branch){
    // To do that, we need to create a new position for each child
    // based off the current position.
    for (UInt argIdx = 0; argIdx < stem->nargs; ++argIdx){
      StemNode argStem = stem->args[argIdx];
      NodePos newPos;
      newPos.len = curPos.len + 1;
      ALLOC(newPos.data, "pos data", newPos.len, sizeof(UInt));
      VG_(memcpy)(newPos.data, curPos.data + 1, curPos.len);
      newPos[0] = argIdx;
      updateEquivMap(node_map, val_map, next_idx, argStem, newPos);
    }
  }
}
void freeNodeMapEntry(NodeMapEntry* entry){
  VG_(free)(entry->position.data);
  VG_(free)(entry);
}
// Initialize a new stem node. Pass zero for nargs if this is a leaf
// node.
void initStemNode(ShadowValue* val, Op_Info* opinfo,
                  SizeT nargs, ShadowValue* args, ...);
// Free up a stem.
void cleanupStemNode(StemNode* stem);
// Deep copy a stem.
void copyStemNode(StemNode* src, StemNode** dest);
