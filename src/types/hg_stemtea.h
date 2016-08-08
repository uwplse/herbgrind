/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            hg_stemtea.h ---*/
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

#ifndef _HG_STEMTEA
#define _HG_STEMTEA

#include "hg_stemtea.hh"
#include "hg_opinfo.hh"
#include "hg_shadowvals.hh"

#include "pub_tool_hashtable.h"
#include "pub_tool_xarray.h"

typedef struct {
  UInt* data;
  SizeT len;
} NodePos;

typedef struct _NodeMapEntry {
  struct _NodeMapEntry* next;
  UWord positionHash;
  NodePos position;
  UWord groupIdx;
} NodeMapEntry;

typedef struct _SplitMapEntry {
  struct _SplitNodeMapEntry* next;
  UWord stemGroup;
  UWord newGroup;
} SplitMapEntry;

typedef struct _VarMapEntry {
  struct _VarMapEntry* next;
  UWord groupIdx;
  int varIdx;
} VarMapEntry;

typedef struct _ValMapEntry {
  struct _ValMapEntry* next;
  UWord key;
  UWord groupIdx;
} ValMapEntry;

typedef enum {
  Node_Leaf,
  Node_Branch
} NodeType;

typedef struct _StemNode StemNode;
struct _StemNode {
  ShadowValue* ref;
  NodeType type;
  double value;
  struct {
    Op_Info* op;
    SizeT nargs;
    StemNode** args;
  } branch;
};

typedef struct _TeaNode TeaNode;
struct _TeaNode {
  NodeType type;
  Bool hasConst;
  double constValue;
  struct {
    Op_Info* op;
    SizeT nargs;
    TeaNode** args;
    VgHashTable* node_map;     
  } branch;
};

#define NULL_POS (NodePos){.len = 0, .data = NULL}

// Initialize a new stem node. Pass zero for nargs if this is a leaf
// node.
void initBranchStemNode(ShadowValue* val, Op_Info* opinfo,
                        SizeT nargs, ShadowValue* args, ...);
void initLeafStemNode(ShadowValue* val);
// Free up a stem.
void cleanupStemNode(StemNode* stem);
// Deep copy a stem.
void copyStemNode(StemNode* src, StemNode** dest);

// Update the tea with a new stem.
void updateTea(Op_Info* op, StemNode* stem);
// Brew a single-stem tea.
TeaNode* stemToTea(StemNode* stem);
// Add a new stem to a tea brew.
void addStem(TeaNode* tea, StemNode* stem);
// Perform the proper brewing on the structure of a tea so that it
// matches a given stem, besides for the node map.
void generalizeStructure(TeaNode* tea, StemNode* stem);
// Merge the node map of a stem into a tea, so that nodes are only in
// the same equivalence class in the resulting tea if they were in the
// same equivalence class in both the original tea and the stem.
void mergeBranchNodeMap(TeaNode* tea, StemNode* stem);
// Go through every entry in the node map, and remove those whose
// positions are invalid in the current structure.
void pruneMapToStructure(TeaNode* tea);
// Turn a map from positions to variable indices to a list of groups,
// where the group at each index cooresponds to the positions that map
// to that variable index.
XArray* getGroups(VgHashTable* node_map);
// Check if a given position is valid in a particular tea structure.
Bool positionValid(TeaNode* tea, NodePos node);
// Get a mapping from positions in the given stem to equivalence
// class/variable indices.
VgHashTable* getStemEquivs(StemNode* stem);

UWord hashPosition(NodePos node);
Word cmp_position(const void* node1, const void* node2);

void updateEquivMap(VgHashTable* node_map,
                    VgHashTable* val_map,
                    int* next_idx,
                    StemNode* stem,
                    NodePos curPos);
void freeNodeMapEntry(void* entry);

char* teaToString(TeaNode* tea, SizeT* numVars_out);
char* teaToStringWithMaps(TeaNode* tea, NodePos curpos,
                          VgHashTable* node_map,
                          VgHashTable* var_map,
                          int* nextvar);
char* teaToBenchString(TeaNode* tea);

void printNodeMap(VgHashTable* node_map);
void printPosition(NodePos pos);

#define lookupPosition(resultName, mapName, positionExpr) \
  { \
    NodePos nodePosition = positionExpr; \
    NodeMapEntry key = {.position = nodePosition, \
                        .positionHash = hashPosition(nodePosition)}; \
    resultName = VG_(HT_gen_lookup)(mapName, &key, cmp_position); \
  }
#endif
