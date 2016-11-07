
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie           hg_stemtea.c ---*/
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
#include "hg_shadowvals.h"
#include "hg_opinfo.h"
#include "../include/hg_macros.h"
#include "../include/hg_options.h"
#include "../types/hg_queue.h"

#include "pub_tool_debuginfo.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_mallocfree.h"

#include "../runtime/performance_analysis.h"

#include <stdarg.h>
#include <math.h>

long long unsigned int num_stems;
long long unsigned int num_stem_bytes;
long long unsigned int num_teas;
long long unsigned int num_tea_bytes;

// This file was designed through several levels of code refinement
// and an executable spec, found in psuedo/aggregateTraces-*.rkt.

void updateTea(Op_Info* op, StemNode* stem){
  if (op->tea == NULL){
    op->tea = stemToTea(stem, 0);
    op->tea->branch.node_map = getStemEquivs(stem);
    if (print_expr_updates){
      char* teaString = teaToString(op->tea, NULL);
      VG_(printf)("Creating new tea %s\n", teaString);
      VG_(free)(teaString);
    }
  } else {
    char *origTeaString, *newTeaString;
    if (print_expr_updates){
      origTeaString = teaToString(op->tea, NULL);
    }
    addStem(op->tea, stem);
    if (print_expr_updates){
      newTeaString = teaToString(op->tea, NULL);
      if (VG_(strcmp)(origTeaString, newTeaString)){
        VG_(printf)("Updating tea %p from %s to %s\n",
                    op->tea, origTeaString, newTeaString);
      } else {
        VG_(printf)(".");
      }
      VG_(free)(origTeaString);
      VG_(free)(newTeaString);
    }
  }
}

TeaNode* stemToTea(StemNode* stem, SizeT curDepth){
  if (curDepth >= max_tea_track_depth){
    return NULL;
  }
  switch(stem->type){
  case Node_Leaf:
    {
      TeaNode* tea;
      ALLOC(tea, "hg.tea", 1, sizeof(TeaNode));
      num_teas += 1;
      num_tea_bytes += sizeof(TeaNode);

      tea->type = Node_Leaf;
      tea->hasConst = True;
      tea->constValue = stem->value;
      return tea;
    }
    break;
  case Node_Branch:
    {
      TeaNode* tea = stem->branch.op->tea;
      if (tea == NULL){
        ALLOC(tea, "hg.tea", 1, sizeof(TeaNode));
        num_teas += 1;
        num_tea_bytes += sizeof(TeaNode);

        tea->type = Node_Branch;
        tea->hasConst = True;
        tea->constValue = stem->value;
        tea->branch.op = stem->branch.op;
        tea->branch.nargs = stem->branch.nargs;
        ALLOC(tea->branch.args, "hg.tea args",
              tea->branch.nargs, sizeof(TeaNode*));
        num_tea_bytes += tea->branch.nargs * sizeof(TeaNode*);
        tea->branch.op->tea = tea;
        for(int i = 0; i < tea->branch.nargs; ++i){
          tea->branch.args[i] =
            stemToTea(stem->branch.args[i],
                      curDepth + 1);
        }
      } else {
        generalizeStructure(&(tea), stem, curDepth);
      }
      return tea;
    }
    break;
  }
  tl_assert(0);
  return NULL;
}
void addStem(TeaNode* tea, StemNode* stem){
  generalizeStructure(&(tea), stem, 0);
  if (tea->type == Node_Branch && stem->type == Node_Branch){
    pruneMapToStructure(tea);
    mergeBranchNodeMap(tea, stem);
  }
}
typedef struct _gEntry {
  TeaNode** tea;
  StemNode* stem;
  SizeT depth;
} gEntry;
gEntry* mkGEntry(TeaNode** tea, StemNode* stem, SizeT depth);
gEntry* mkGEntry(TeaNode** tea, StemNode* stem, SizeT depth){
  gEntry* entry;
  ALLOC(entry, "generalizeStructure argument entry",
        1, sizeof(gEntry));
  entry->tea = tea;
  entry->stem = stem;
  entry->depth = depth;
  return entry;
}
void generalizeStructure(TeaNode** _tea, StemNode* _stem, SizeT depth){
  // We can't use recursive calls to traverse stems, because they can
  // get really large and the call stack isn't meant to handle that
  // much data. Instead, we use an explicit queue data structure.

  // In case you're wondering why we use a queue and not a stack, it
  // really doesn't matter much, but I expect highly unbalanced trees
  // that are basically just a linked list with a few nodes hanging
  // off the side to be a lot more common than highly balanced trees,
  // since the former could result from a loop or repeated mutation of
  // a single accumulator, while the latter requires a complex
  // expression built of many parts. And a Queue is going to use a bit
  // less memory in this case, which might matter as stem tree sizes
  // get very large. Of course, when we start really supporting loops
  // it might not matter...
  Queue* generalizeQueue = mkQueue();

  queue_push(generalizeQueue, mkGEntry(_tea, _stem, depth));

  while (! queue_empty(generalizeQueue)){
    gEntry* entry = queue_pop(generalizeQueue);
    TeaNode** tea = entry->tea;
    StemNode* stem = entry->stem;

    // If the tea continues down, but the stem stops, or the stem stops
    // matching, then we want to replace the tea with a branch. But we
    // don't want to do it in place, because then other places that
    // reference that operation will get the truncated tree even though
    // the stem never passed through that op. So instead, allocate a new
    // leaf and stick it in the reference location we got.
    if ((*tea)->type == Node_Branch &&
        (stem->type == Node_Leaf ||
         ((*tea)->type == Node_Branch &&
          stem->type == Node_Branch &&
         VG_(strcmp)((*tea)->branch.op->debuginfo.symbol,
                     stem->branch.op->debuginfo.symbol)) ||
         (*tea)->branch.nargs != stem->branch.nargs)){
      TeaNode* oldTea = (*tea);
      ALLOC(*tea, "tea leaf", 1, sizeof(StemNode));
      (*tea)->type = Node_Leaf;
      (*tea)->hasConst = (oldTea->hasConst &&
                          (oldTea->constValue == stem->value ||
                           (oldTea->constValue != oldTea->constValue &&
                            stem->value != stem->value))) ? True : False;
      (*tea)->constValue = oldTea->constValue;
      num_teas += 1;
      num_tea_bytes += sizeof(TeaNode);
    }
    // If the value this node was initially assigned doesn't match that
    // of the new stem, then it isn't constant across all stems, so mark
    // it as such.
    if ((*tea)->constValue != stem->value &&
        (*tea)->constValue == (*tea)->constValue &&
        stem->value == stem->value){
      (*tea)->hasConst = False;
    }
    // If the result is still a branch, generalize the children
    if ((*tea)->type == Node_Branch && entry->depth + 1 < max_tea_track_depth){
      for (int i = 0; i < (*tea)->branch.nargs; ++i){
        if ((*tea)->branch.args[i] == NULL){
          (*tea)->branch.args[i] =
            stemToTea(stem->branch.args[i], entry->depth + 1);
        } else {
          queue_push(generalizeQueue, mkGEntry(&((*tea)->branch.args[i]),
                                               stem->branch.args[i],
                                               entry->depth + 1));
        }
      }
    }
    VG_(free)(entry);
  }
  freeQueue(generalizeQueue);
}
void mergeBranchNodeMap(TeaNode* tea, StemNode* stem){
  // Take the current node map, from the tea, which maps positions of
  // nodes to indices of equivalence classes, and turn it into a list
  // of lists, where the rows are the equivlance classes of positions.
  XArray* teaMapGroups = getGroups(tea->branch.node_map);
  // Now, get the map from node positions to equivalence class indices
  // from the stem, to merge into the tea one.
  VgHashTable* stemMap = getStemEquivs(stem);
  // We're going to be splitting nodes off the tea equivalence groups
  // if they aren't equivalent in the stem map, so we'll need to
  // create new groups. Get the index of the next group we'll create.
  int nextGroupIdx = VG_(sizeXA)(teaMapGroups);
  // Go through each of the current tea equivalence classes.
  for (int groupIdx = 0;
       groupIdx < VG_(sizeXA)(teaMapGroups);
       ++groupIdx){
    XArray* positions = *(XArray**)VG_(indexXA)(teaMapGroups, groupIdx);
    if (VG_(sizeXA)(positions) != 0){
      // The split map is a map from equivalence group indices IN THE
      // STEM MAP to new equivalence group indices in the tea map.
      VgHashTable* splitMap = VG_(HT_construct)("split map");
      // Figure out what the first position in this group was mapped
      // to in the stem map, and map it to the current group, so that
      // any position in this group currently that is also in the same
      // group in the stem map stays in this group.
      NodeMapEntry* initialGroupEntry;
      lookupPosition(initialGroupEntry, stemMap,
                     *(NodePos*)VG_(indexXA)(positions, 0));
      SplitMapEntry* initialSplitMapEntry;
      ALLOC(initialSplitMapEntry, "initial split map entry",
            1, sizeof(SplitMapEntry));
      initialSplitMapEntry->stemGroup = initialGroupEntry->groupIdx;
      initialSplitMapEntry->newGroup = groupIdx;
      VG_(HT_add_node)(splitMap, initialSplitMapEntry);
      // Now, go through the rest of the entries in the group.
      for (int posIdx = 1;
           posIdx < VG_(sizeXA)(positions);
           ++posIdx){
        // Figure out which group this node belongs to in the stem
        // map.
        NodePos position = *(NodePos*)VG_(indexXA)(positions, posIdx);
        NodeMapEntry* stemGroupEntry;
        lookupPosition(stemGroupEntry, stemMap,
                       position);
        // Check to see if this node already has an entry in the split
        // map, and what it is if so. If this node matches previous
        // nodes in the group, it should already have an entry,
        // otherewise it was mapped to a group in the stem map that no
        // previous node was mapped to.
        SplitMapEntry* splitMapEntry =
          VG_(HT_lookup)(splitMap, stemGroupEntry->groupIdx);
        if (splitMapEntry != NULL){
          // If the stem group this node maps to already has an entry,
          // move it in the tea map if that entry isn't the group it's
          // already in.
          if (splitMapEntry->newGroup != groupIdx){
            // Remove the old entry
            NodeMapEntry oldEntryPrototype;
            oldEntryPrototype.position = position;
            oldEntryPrototype.positionHash = hashPosition(position);
            VG_(free)(VG_(HT_gen_remove)(tea->branch.node_map, &oldEntryPrototype, cmp_position));

            // Add the new entry.
            NodeMapEntry* newTeaGroupEntry;
            ALLOC(newTeaGroupEntry, "tea node map entry",
                  1, sizeof(NodeMapEntry));
            newTeaGroupEntry->position = position;
            newTeaGroupEntry->positionHash = hashPosition(position);
            newTeaGroupEntry->groupIdx = splitMapEntry->newGroup;
            VG_(HT_add_node)(tea->branch.node_map, newTeaGroupEntry);
          }
        } else {
          // Otherwise, create a new group for it, and direct future
          // nodes in this existing tea group that match the stem
          // group to this new tea group.
          int newIdx = nextGroupIdx++;
          ALLOC(splitMapEntry, "split map entry",
                1, sizeof(SplitMapEntry));
          splitMapEntry->stemGroup = stemGroupEntry->groupIdx;
          splitMapEntry->newGroup = newIdx;
          VG_(HT_add_node)(splitMap, splitMapEntry);

          // Remove the old tea map entry.
          NodeMapEntry oldEntryPrototype;
          oldEntryPrototype.position = position;
          oldEntryPrototype.positionHash = hashPosition(position);
          VG_(free)(VG_(HT_gen_remove)(tea->branch.node_map, &oldEntryPrototype, cmp_position));

          // Add the new tea map entry.
          NodeMapEntry* newTeaGroupEntry;
          ALLOC(newTeaGroupEntry, "tea group entry",
                1, sizeof(NodeMapEntry));
          newTeaGroupEntry->position = position;
          newTeaGroupEntry->positionHash = hashPosition(position);
          newTeaGroupEntry->groupIdx = newIdx;
          VG_(HT_add_node)(tea->branch.node_map, newTeaGroupEntry);
        }
      }
      VG_(HT_destruct)(splitMap, VG_(free));
    }
    // Clean up after ourselves, deleting each group as we're done
    // with it.
    VG_(deleteXA)(positions);
  }
  // Finally, delete the group of groups, and the equivalence map we
  // got from the stem.
  VG_(deleteXA)(teaMapGroups);
  VG_(HT_destruct)(stemMap, freeNodeMapEntry);
}
// Go through every entry in the node map, and remove those whose
// positions are invalid in the current structure.
void pruneMapToStructure(TeaNode* tea){
  UInt numNodes;
  NodeMapEntry** entries =
    (NodeMapEntry**)VG_(HT_to_array)(tea->branch.node_map,
                                     &numNodes);
  for(UInt i = 0; i < numNodes; ++i){
    NodePos position = entries[i]->position;
    if (!positionValid(tea, position)){
      NodeMapEntry key = {.position = position,
                          .positionHash = hashPosition(position)};
      NodeMapEntry* entry = VG_(HT_gen_remove)(tea->branch.node_map, &key, cmp_position);
      VG_(free)(entry);
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
    while (entry->groupIdx >= VG_(sizeXA)(groups)){
      XArray* newGroup = VG_(newXA)(VG_(malloc), "group",
                                    VG_(free), sizeof(NodePos));
      VG_(addToXA)(groups, &newGroup);
    }
    VG_(addToXA)(*(XArray**)VG_(indexXA)(groups, entry->groupIdx), &(entry->position));
  }
  return groups;
}
void printGroups(XArray* groups){
  for (SizeT i = 0; i < VG_(sizeXA)(groups); i++){
    XArray* group = *(XArray**)VG_(indexXA)(groups, i);
    for (SizeT j = 0; j < VG_(sizeXA)(group); j++){
      printPosition(*(NodePos*)VG_(indexXA)(group, j));
      VG_(printf)(", ");
    }
    VG_(printf)("\n");
  }
  VG_(printf)("\n");
}

// Check if a given position is valid in a particular tea structure.
Bool positionValid(TeaNode* tea, NodePos pos){
  if (pos.len >= max_tea_track_depth){
    return False;
  }
  TeaNode* curTea = tea;
  for (int i = pos.len - 1; i >= 0; --i){
    tl_assert2(curTea != NULL, "Tea arg is null when checking for validity of position!\n");
    if (curTea->type == Node_Leaf){
      return False;
    } else if (curTea->branch.nargs <= pos.data[i]){
      return False;
    }
    curTea = curTea->branch.args[pos.data[i]];
  }
  return True;
}
// Get a mapping from positions in the given stem to equivalence
// class/variable indices.
VgHashTable* getStemEquivs(StemNode* stem){
  // NodePos -> GroupIdx
  VgHashTable* node_map = VG_(HT_construct)("node map");
  // Float -> GroupIdx
  VgHashTable* val_map = VG_(HT_construct)("val map");
  int next_idx = 0;
  updateEquivMap(node_map, val_map, &next_idx, stem, NULL_POS);
  VG_(HT_destruct)(val_map, VG_(free));
  return node_map;
}

typedef struct _uemEntry {
  StemNode* stem;
  NodePos curPos;
} uemEntry;
uemEntry* mkUemEntry(StemNode* stem, NodePos curPos);
uemEntry* mkUemEntry(StemNode* stem, NodePos curPos) {
  uemEntry* entry;
  ALLOC(entry, "update equivalence map entry",
        1, sizeof(uemEntry));
  entry->stem = stem;
  entry->curPos = curPos;
  return entry;
}

void updateEquivMap(VgHashTable* node_map,
                    VgHashTable* val_map,
                    int* next_idx,
                    StemNode* _stem,
                    NodePos _curPos){
  // We can't use recursive calls to traverse stems, because they can
  // get really large and the call stack isn't meant to handle that
  // much data. Instead, we use an explicit queue data structure.

  Queue* updateEquivMapQueue = mkQueue();

  queue_push(updateEquivMapQueue, mkUemEntry(_stem, _curPos));

  while (! queue_empty(updateEquivMapQueue)){
    uemEntry* entry = queue_pop(updateEquivMapQueue);
    StemNode* stem = entry->stem;
    NodePos curPos = entry->curPos;

    // Allocate a new node map entry for the current node.
    NodeMapEntry* newNodeEntry;
    ALLOC(newNodeEntry, "node map entry", 1, sizeof(NodeMapEntry));
    newNodeEntry->position = curPos;
    newNodeEntry->positionHash = hashPosition(curPos);

    // Convert the value of the stem to a UWord key by literally
    // reinterpreting the bytes. Not clear that this will do the right
    // thing on 32-bit platforms, as it should pull the first 32-bits
    // out of the double for matching. Might overmatch as a result.
    UWord keyval = 0;
    VG_(memcpy)(&keyval, &stem->value, sizeof(UWord));
    ValMapEntry* existing_entry = VG_(HT_lookup)(val_map, keyval);

    // If we already have an entry for this value, map the current
    // position to that.
    if (existing_entry != NULL){
      newNodeEntry->groupIdx = existing_entry->groupIdx;
    } else {
      // Otherwise, create a fresh index, map the current value to that
      // (so that later nodes that share this value will also get
      // mapped), and then map the current position to that.
      int new_idx = (*next_idx)++;

      ValMapEntry* newValEntry;
      ALLOC(newValEntry, "val map entry", 1, sizeof(ValMapEntry));
      newValEntry->key = keyval;
      newValEntry->groupIdx = new_idx;
      VG_(HT_add_node)(val_map, newValEntry);

      newNodeEntry->groupIdx = new_idx;
    }
    VG_(HT_add_node)(node_map, newNodeEntry);

    // Finally, if this is a branch, run on the children.
    if (stem->type == Node_Branch && curPos.len + 1 < max_tea_track_depth){
      // To do that, we need to create a new position for each child
      // based off the current position.
      for (UInt argIdx = 0; argIdx < stem->branch.nargs; ++argIdx){
        StemNode* argStem = stem->branch.args[argIdx];
        NodePos newPos;
        newPos.len = curPos.len + 1;
        ALLOC(newPos.data, "pos data", newPos.len, sizeof(UInt));
        VG_(memcpy)(newPos.data + 1, curPos.data, curPos.len * sizeof(UInt));
        newPos.data[0] = argIdx;
        queue_push(updateEquivMapQueue, mkUemEntry(argStem, newPos));
      }
    }
    VG_(free)(entry);
  }
  freeQueue(updateEquivMapQueue);
}
void freeNodeMapEntry(void* entry){
  VG_(free)(((NodeMapEntry*)entry)->position.data);
  VG_(free)(entry);
}
// Initialize a new stem node.
void initBranchStemNode(ShadowValue* val, Op_Info* opinfo,
                        SizeT nargs, ...){
  if (!report_exprs) return;
  ALLOC(val->stem, "hg.stem_branch", 1, sizeof(StemNode));
  num_stems += 1;
  num_stem_bytes += sizeof(StemNode);

  val->stem->value = mpfr_get_d(val->value, MPFR_RNDN);
  // Normalize NaN's
  if (val->stem->value != val->stem->value){
    val->stem->value = NAN;
  }
  if (print_moves){
    VG_(printf)("Making stem node %p\n", val->stem);
  }
  val->stem->ref_count = 1;
  val->stem->type = Node_Branch;
  val->stem->branch.op = opinfo;
  val->stem->branch.nargs = nargs;

  ALLOC(val->stem->branch.args, "stem args",
        nargs, sizeof(StemNode*));
  num_stem_bytes += nargs * sizeof(StemNode*);

  va_list args;
  va_start(args, nargs);
  for(SizeT i = 0; i < nargs; ++i){
    ShadowValue* argVal = va_arg(args, ShadowValue*);
    val->stem->branch.args[i] = argVal->stem;
    (argVal->stem->ref_count)++;
    if (print_counts){
      VG_(printf)("Increasing count of stem %p to %lu\n",
                  argVal->stem,
                  argVal->stem->ref_count);
    }
  }
  va_end(args);
}
void initLeafStemNode(ShadowValue* val){
  if (!report_exprs) return;
  ALLOC(val->stem, "hg.stem_leaf", 1, sizeof(StemNode));
  num_stems += 1;
  num_stem_bytes += sizeof(StemNode);

  val->stem->value = mpfr_get_d(val->value, MPFR_RNDN);
  // Normalize NaN's
  if (val->stem->value != val->stem->value){
    val->stem->value = NAN;
  }
  if (print_moves){
    VG_(printf)("Making stem node %p\n", val->stem);
  }
  val->stem->ref_count = 1;
  val->stem->type = Node_Leaf;
}
// copy a stem.
void copyStemNode(StemNode* src, StemNode** dest){
  (*dest) = src;
  (src->ref_count)++;
  if (print_counts){
    VG_(printf)("Increasing count of stem %p to %lu\n",
                src, src->ref_count);
  }
}
void disownStemNode(StemNode* stem){
  // We used to use potentiall recursive calls to cascade frees for
  // chained stem nodes, but unfortunately the stem nodes can get
  // quite big, big enough that cascading frees of children using the
  // call stack can overflow it. So instead we're using this explicit
  // queue system to keep track of cascading frees, and we're going to
  // reach into the stem data that should be encapsulated to disown
  // references to children.
  tl_assert(stem != NULL);

  Queue* disownQueue = mkQueue();
  queue_push(disownQueue, stem);

  while (!queue_empty(disownQueue)){
    StemNode* curStem = queue_pop(disownQueue);
    (curStem->ref_count) --;
    if (print_counts){
      VG_(printf)("Decreasing count of stem %p to %lu\n",
                  curStem, curStem->ref_count);
    }
    if (curStem->ref_count < 1){
      if (print_moves){
        VG_(printf)("Cleaning up stem node %p\n", curStem);
      }
      if (curStem->type == Node_Branch){
        for (int i = 0; i < curStem->branch.nargs; ++i){
          queue_push(disownQueue,
                     curStem->branch.args[i]);
        }
        VG_(free)(curStem->branch.args);
        num_stem_bytes -= curStem->branch.nargs * sizeof(StemNode*);
      }
      VG_(free)(curStem);
      num_stems -= 1;
      num_stem_bytes -= sizeof(StemNode);
    }
  }
  freeQueue(disownQueue);
}
static const char varNames[] =
  {'x', 'y', 'z', 'w', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};

char* teaToString(TeaNode* tea, SizeT* numVars_out){
  char* result;
  VgHashTable* var_map = VG_(HT_construct)("var map");
  int nextvar = 0;
  SizeT max_depth =
    max_tea_track_depth > max_print_depth ?
    max_print_depth : max_tea_track_depth;
  if (tea->type == Node_Branch){
    result = teaToStringWithMaps(tea, NULL_POS,
                                 tea->branch.node_map, var_map,
                                 &nextvar,
                                 max_depth,
                                 NULL, 0);
  } else {
    result = teaToStringWithMaps(tea, NULL_POS, NULL, var_map,
                                 &nextvar,
                                 max_depth,
                                 NULL, 0);
  }
  if (numVars_out != NULL){
    *numVars_out = VG_(HT_count_nodes)(var_map);
  }
  VG_(HT_destruct)(var_map, VG_(free));
  return result;
}

// Instead of replacing recursive calls with explicit queue's in the
// printing methods, we're just going to limit their depth, since they
// won't handle very large expressions anyway since they allocate a
// fixed string buffer.

char* teaToStructureString(TeaNode* tea, SizeT max_depth){
  char* buf;
  SizeT bufpos = 0;
  ALLOC(buf, "ast string", max_expr_string_size, sizeof(char));

  if (max_depth == 0){
    VG_(snprintf)(buf, 4, "...");
  } else if (tea->type == Node_Leaf){
    if (tea->hasConst){
      VG_(snprintf)(buf, max_expr_string_size, "%f", tea->constValue);
    } else {
      VG_(snprintf)(buf, 2, "%c", varNames[0]);
    }
  } else {
    bufpos += VG_(snprintf)(buf, max_expr_string_size, "(%s",
                            tea->branch.op->debuginfo.symbol);
    for (SizeT argIdx = 0; argIdx < tea->branch.nargs; ++argIdx){
      char* subexpr = teaToStructureString(tea->branch.args[argIdx], max_depth - 1);
      bufpos += VG_(snprintf)(buf + bufpos, max_expr_string_size - bufpos,
                              " %s", subexpr);
      VG_(free)(subexpr);
    }
    VG_(snprintf)(buf + bufpos, max_expr_string_size - bufpos, ")");
  }
  return buf;
}

char* teaToStringWithMaps(TeaNode* tea, NodePos curpos,
                          VgHashTable* node_map,
                          VgHashTable* var_map,
                          int* nextvar,
                          SizeT max_depth,
                          const char* parent_filename,
                          int parent_linenum){
  char* buf;
  SizeT bufpos = 0;
  ALLOC(buf, "ast string", max_expr_string_size, sizeof(char));
  if (tea == NULL){
    VG_(snprintf)(buf, max_expr_string_size, "...");
  } else if (tea->type == Node_Leaf || max_depth <= 1){
    if (tea->hasConst){
      VG_(snprintf)(buf, max_expr_string_size, "%f", tea->constValue);
    } else {
      if (node_map == NULL){
        VG_(snprintf)(buf, 2, "%c", varNames[0]);
      } else {
        NodeMapEntry* group_entry;
        lookupPosition(group_entry, node_map, curpos);
        if (group_entry == NULL){
          VG_(printf)("Couldn't find a group entry for node %p at position ",
                      tea);
          printPosition(curpos);
          VG_(printf)("\n");
        }
        tl_assert2(group_entry != NULL,
                   "Couldn't find a group entry for node with max_depth %lu!\n",
                   max_depth);
        VarMapEntry* var_entry =
          VG_(HT_lookup)(var_map, group_entry->groupIdx);
        if (var_entry == NULL){
          ALLOC(var_entry, "var entry", 1, sizeof(VarMapEntry));
          var_entry->groupIdx = group_entry->groupIdx;
          var_entry->varIdx = (*nextvar)++;
          VG_(HT_add_node)(var_map, var_entry);
        }
        VG_(snprintf)(buf, 2, "%c", varNames[var_entry->varIdx]);
      }
    }
  } else {
    bufpos += VG_(snprintf)(buf, max_expr_string_size, "(%s",
                            tea->branch.op->debuginfo.symbol);
    for (SizeT argIdx = 0; argIdx < tea->branch.nargs; ++argIdx){
      NodePos newPos;
      newPos.len = curpos.len + 1;
      ALLOC(newPos.data, "pos data", newPos.len, sizeof(UInt));
      VG_(memcpy)(newPos.data + 1, curpos.data, curpos.len * sizeof(UInt));
      newPos.data[0] = argIdx;
      tl_assert2(tea->branch.args[argIdx] != NULL, "Argument is null with max_depth %lu\n", max_depth);

      char* subexpr = teaToStringWithMaps(tea->branch.args[argIdx],
                                          newPos, node_map, var_map,
                                          nextvar,
                                          max_depth - 1,
                                          tea->branch.op->debuginfo.src_filename,
                                          tea->branch.op->debuginfo.src_line);
      VG_(free)(newPos.data);
      bufpos += VG_(snprintf)(buf + bufpos, max_expr_string_size - bufpos,
                              " %s", subexpr);
      VG_(free)(subexpr);
    }
    bufpos += VG_(snprintf)(buf + bufpos,
                            max_expr_string_size - bufpos,
                            ")");
    if (verbose_linenums &&
        tea->branch.op->debuginfo.src_filename != NULL &&
        parent_filename != NULL){
      if (VG_(strcmp)(tea->branch.op->debuginfo.src_filename, parent_filename)){
        bufpos += VG_(snprintf)(buf + bufpos,
                                max_expr_string_size - bufpos,
                                ":[%s:%u]",
                                tea->branch.op->debuginfo.src_filename,
                                tea->branch.op->debuginfo.src_line);
      } else if (tea->branch.op->debuginfo.src_line != parent_linenum){
        bufpos += VG_(snprintf)(buf + bufpos,
                                max_expr_string_size - bufpos,
                                ":[:%u]",
                                tea->branch.op->debuginfo.src_line);
      }
    }
  }
  return buf;
}
char* teaToBenchString(TeaNode* tea, Bool haveNewlines){
  SizeT nvars;
  char* exprString = teaToString(tea, &nvars);
  SizeT exprStringSize = VG_(strlen)(exprString);
  char* binderString;
  // We're assuming here that each variable is only one character long
  // to size this allocation.
  SizeT binderStringSize = (nvars * 2);
  if (binderStringSize == 0)
    binderStringSize = 1;

  ALLOC(binderString, "binder string", binderStringSize, sizeof(char));
  SizeT cursor = 0;
  for (int i = 0; i < nvars; ++i){
    binderString[cursor++] = varNames[i];
    if (i < nvars - 1){
      binderString[cursor++] = ' ';
    }
  }
  SizeT benchStringSize;
  if (haveNewlines){
    benchStringSize =
      9 /* "(FPCore (" */ + binderStringSize - 1 /* This one includes a null char which we don't need */ +
      31 /* ")\n      :type binary64\n      " */ + exprStringSize +
      2 /* ")\0" */;
  } else {
    benchStringSize =
      9 /* "(FPCore (" */ + binderStringSize - 1 /* This one includes a null char which we don't need */ +
      29 /* ")      :type binary64      " */ + exprStringSize +
      2 /* ")\0" */;
  }
  
  char* benchString;
  ALLOC(benchString, "hg.bench_string", benchStringSize, sizeof(char));
  if (haveNewlines){
    VG_(snprintf)(benchString, benchStringSize,
                  "(FPCore (%s)\n      :type binary64\n      %s)",
                  binderString,
                  exprString);
  } else {
    VG_(snprintf)(benchString, benchStringSize,
                  "(FPCore (%s)      :type binary64      %s)",
                  binderString,
                  exprString);
  }
  VG_(free)(binderString);
  VG_(free)(exprString);
  return benchString;
}
UWord hashPosition(NodePos node){
  UWord hash = 0;
  for (SizeT i = 0; i < node.len; ++i){
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
void printNodeMap(VgHashTable* node_map){
  VG_(printf)("===============================\n");
  VG_(HT_ResetIter)(node_map);
  for(NodeMapEntry* entry = VG_(HT_Next)(node_map); entry != NULL;
      entry = VG_(HT_Next)(node_map)){
    printPosition(entry->position);
    VG_(printf)("(%lu) ==> %lu\n", entry->positionHash, entry->groupIdx);
  }
  VG_(printf)("\n");
}
void printPosition(NodePos pos){
  VG_(printf)("[");
  for(SizeT i = 0; i < pos.len; ++i){
    VG_(printf)("%u ", pos.data[i]);
  }
  VG_(printf)("]");
}

// WARNING: Do not use this function to compare two runtime stems
// unless you think very carefully about it!

// This function uses recursive function calls to traverse the
// structure of the stems it's matching on. Now, normally this would
// be problematic because stems can get big enough that traversing
// them through recursive calls overflows the stack. In this case
// however, it's currently okay because we only compare a runtime stem
// to a predefined one of a fixed, small size. Since this will stop
// after fully traversing the smaller of the trees, this means we're
// safe from overflow.
Bool teaStructureMatches(TeaNode* tea1, TeaNode* tea2){
  if (tea1->type != tea2->type) return False;
  if (tea1->type == Node_Leaf) return True;
  if (VG_(strcmp)(tea1->branch.op->debuginfo.symbol,
                  tea2->branch.op->debuginfo.symbol))
    return False;
  if (tea1->branch.nargs != tea2->branch.nargs)
    return False;
  for (int i = 0; i < tea1->branch.nargs; ++i){
    if (!teaStructureMatches(tea1->branch.args[i], tea2->branch.args[i])){
      return False;
    }
  }
  return True;
}
