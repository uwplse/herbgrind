/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie       influence-list.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017Alex Sanchez-Stern

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

#include "influence-list.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"

#include "../../options.h"
#include "../../helper/runtime-util.h"

InfluenceList pool = NULL;

InfluenceList mkInfluenceList(void){
  InfluenceList result;
  if (pool == NULL){
    result =
      VG_(malloc)("influence list", sizeof(struct _influenceList));
    result->data =
      VG_(malloc)("influence list data", sizeof(ShadowOpInfo*) * max_influences);
  } else {
    result = pool;
    pool = pool->next;
  }
  result->next = NULL;
  result->length = 0;
  return result;
}

void freeInfluenceList(InfluenceList il){
  il->next = pool;
  pool = il;
}

inline int score(ShadowOpInfo* info);
inline int score(ShadowOpInfo* info){
  return info->agg.local_error.max_error;
}

InfluenceList mergeInfluences(InfluenceList il1, InfluenceList il2,
                              ShadowOpInfo* extra){
  if (il1 == NULL && il2 == NULL && extra == NULL) return NULL;
  InfluenceList result = mkInfluenceList();
  int i = 0;
  int j = 0;
  while(((il1 != NULL && i < il1->length) ||
         (il2 != NULL && j < il2->length) ||
         extra != NULL) &&
        result->length < max_influences){
    if (il1 != NULL && i < il1->length &&
        (il2 == NULL || j >= il2->length || cmpInfo(il1->data[i], il2->data[j]) >= 0) &&
        (extra == NULL || cmpInfo(il1->data[i], extra) >= 0)){
      if (result->length == 0 || result->data[result->length-1] != il1->data[i]){
        result->data[result->length++] = il1->data[i];
      }
      i++;
    } else if (il2 != NULL && j < il2->length &&
               (il1 == NULL || i >= il1->length ||
                cmpInfo(il2->data[j], il1->data[i]) >= 0) &&
               (extra == NULL || cmpInfo(il2->data[j], extra) >= 0)){
      if (result->length == 0 || result->data[result->length-1] != il2->data[j]){
        result->data[result->length++] = il2->data[j++];
      }
      j++;
    } else if (extra != NULL &&
               (il1 == NULL || i >= il1->length || cmpInfo(extra, il1->data[i]) >= 0) &&
               (il2 == NULL || j >= il2->length || cmpInfo(extra, il2->data[j]) >= 0)){
      if (result->length == 0 || result->data[result->length-1] != extra){
        result->data[result->length++] = extra;
      }
      extra = NULL;
    } else {
      VG_(printf)("cmpInfo(il2->data[j], il1->data[i]) : %d\n",
                  cmpInfo(il2->data[j], il1->data[i]));
      VG_(printf)("il2->data[j]: %p\nil1->data[i]: %p\n",
                  il2->data[j], il1->data[i]);
      VG_(printf)("extra: %p\n", extra);
      VG_(printf)("i: %d, il1: %p", i, il1);
      if (il1 != NULL){
        VG_(printf)(" il1->length : %d", il1->length);
      }
      VG_(printf)("\n");
      VG_(printf)("j: %d, il2: %p", j, il2);
      if (il2 != NULL){
        VG_(printf)(" il2->length : %d", il2->length);
      }
      VG_(printf)("\n");
      tl_assert(0);
    }
  }
  return result;
}

void ppInfluences(InfluenceList influences){
  if (influences == NULL){
    return;
  }
  for(int i = 0; i < influences->length; ++i){
    VG_(printf)("* ");
    printOpInfo(influences->data[i]);
    VG_(printf)("\n");
  }
}
