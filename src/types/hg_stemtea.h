#ifndef _HG_STEMTEA
#define _HG_STEMTEA

#include "hg_opinfo.hh"
#include "hg_shadowvals.hh"

#include "pub_tool_hashtable.h"
#include "pub_tool_xarray.h"


/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            hg_opinfo.c ---*/
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

typedef struct _NodeMapEntry {
  struct _NodeMapEntry* next;
  NodePos position;
  int varidx;
} NodeMapEntry;

typedef struct _SplitMapEntry {
  struct _SplitNodeMapEntry* next;
  int stemGroup;
  int newGroup;
} SplitMapEntry;

typedef struct _ValMapEntry {
  struct _ValMapEntry* next;
  UWord key;
  int varidx;
} ValMapEntry;

typedef enum {
  Stem_Leaf,
  Stem_Branch
} NodeType;

typedef struct _StemNode {
  NodeType type;
  double value;
  struct branch {
    OpInfo* op;
    SizeT nargs;
    _StemNode** args;
  };
} StemNode;

typedef enum {
  Const,
  NonConst
} ConstType;

typedef struct _TeaNode {
  NodeType type;
  ConstType hasConst;
  double constValue;
  struct branch {
    OpInfo* op;
    SizeT nargs;
    _TeaNode** args;
    VgHashTable* node_map;     
  }; 
} TeaNode;

typedef int* NodePos;

TeaNode* stemToTea(StemNode* stem);
void addStem(TeaNode* tea, StemNode* stem);
void generalizeStructure(TeaNode* tea, StemNode* stem);
void mergeBranchNodeMap(TeaNode* tea, StemNode* stem);
void pruneMapToStructure(TeaNode* tea);
XArray* getGroups(VgHashTable* node_map);
int positionValid(TeaNode tea, NodePos node);
VgHashTable* getStemEquivs(StemNode* stem);
void updateEquivMap(VgHashTable* node_map,
                    VgHashTable* val_map,
                    int* next_idx,
                    StemNode* stem,
                    NodePos curPos);
#endif
