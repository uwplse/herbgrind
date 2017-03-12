/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                 list.h ---*/
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
#ifndef _LIST_H
#define _LIST_H

#include "pub_tool_libcassert.h"
#include "pub_tool_mallocfree.h"

#define List_H(Type, name)              \
  typedef struct _##name                \
  {                                     \
    struct _##name* next;               \
    Type item;                          \
  }* name;                              \
  void lpush_##name(name* list_loc, Type list_item);    \
  Type lpop_##name(name* list_loc);                     \
  int  length_##name(name* list_loc);                   \
  Type* getIdxLoc_##name(name* list_loc, int idx);      \
  extern name unused_nodes_##name;
#define List_Impl(Type, name)           \
  name unused_nodes_##name;             \
                                        \
  void lpush_##name(name* list_loc, Type list_item){                    \
    name newnode;                                                       \
    if (unused_nodes_##name == NULL){                                   \
      newnode = VG_(malloc)(#name"node", sizeof(struct _##name));       \
    } else {                                                            \
      newnode = unused_nodes_##name;                                    \
      unused_nodes_##name = unused_nodes_##name->next;                  \
    }                                                                   \
    newnode->next = *list_loc;                                          \
    newnode->item = list_item;                                          \
    *list_loc = newnode;                                                \
  }                                                                     \
  Type lpop_##name(name* list_loc){                                     \
    tl_assert2(*list_loc, "Tried to pop from an empty stack!\n");       \
    Type item = (*list_loc)->item;                                      \
    name newhead = (*list_loc)->next;                                   \
    (*list_loc)->next = unused_nodes_##name;                            \
    unused_nodes_##name = (*list_loc);                                  \
    (*list_loc) = newhead;                                              \
    return item;                                                        \
  }                                                                     \
  int length_##name(name* list_loc){                                   \
    int result = 0;                                                     \
    for(name curNode = *list_loc; curNode != NULL;                      \
        curNode = curNode->next){                                       \
      result++;                                                         \
    }                                                                   \
    return result;                                                      \
  }                                                                     \
  Type* getIdxLoc_##name(name* list_loc, int idx){                      \
    name curNode = *list_loc;                                           \
    for(int i = 0; i < idx; i++){                                       \
      curNode = curNode->next;                                          \
      if (curNode == NULL){                                             \
        return NULL;                                                    \
      }                                                                 \
    }                                                                   \
    return &(curNode->item);                                            \
  }

// This way you can almost pretend it's real polymorphism
#define lpop(type) lpop_##type
#define lpush(type) lpush_##type
#define length(type) length_##type
#define getIdxLoc(type) getIdxLoc_##type

#endif
