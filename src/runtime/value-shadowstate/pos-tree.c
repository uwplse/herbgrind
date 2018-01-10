/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie             pos-tree.c ---*/
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

#include "pos-tree.h"

#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

Xarray_Impl(NodePos, PosList);

NodePos null_pos;

void initializePositionTree(void){
  null_pos = VG_(perm_malloc)(sizeof(struct _posTreeNode),
                              vg_alignof(struct _posTreeNode));
  null_pos->children = mkXA(PosList)();
  null_pos->data= NULL;
  null_pos->len = 0;
}

NodePos rconsPos(NodePos parent, unsigned char childIndex){
  while (parent->children->size <= childIndex){
    NodePos newNode =
      VG_(perm_malloc)(sizeof(struct _posTreeNode),
                       vg_alignof(struct _posTreeNode));
    newNode->children = mkXA(PosList)();
    newNode->data =
      VG_(perm_malloc)(sizeof(unsigned char) * (parent->len + 1),
                       vg_alignof(NodePos));
    newNode->len = parent->len + 1;
    VG_(memcpy)(newNode->data, parent->data, parent->len);
    newNode->data[parent->len] = parent->children->size;
    newNode->parent = parent;
    XApush(PosList)(parent->children, newNode);
  }
  NodePos result = parent->children->data[childIndex];
  return result;
}
NodePos rtail(NodePos child){
  return child->parent;
}
unsigned char rhead(NodePos pos){
  return pos->data[pos->len - 1];
}
NodePos appendPos(NodePos prefix, NodePos suffix){
  NodePos curPos = prefix;
  for(int i = 0; i < suffix->len; ++i){
    curPos = rconsPos(curPos, suffix->data[i]);
  }
  return curPos;
}

void ppNodePos(NodePos pos){
  VG_(printf)("[");
  for(int i = 0; i < pos->len; ++i){
    VG_(printf)(" %d", (int)pos->data[i]);
  }
  VG_(printf)(" ]");
}
UWord hashPosition(NodePos node){
  return (UWord)(void*)node;
}
