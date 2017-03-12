/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie                marks.c ---*/
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

#include "marks.h"
#include "../../helper/runtime-util.h"
#include "../value-shadowstate/shadowval.h"
#include "../shadowop/error.h"
#include "pub_tool_libcprint.h"

List_Impl(MarkInfo, MarkList);

VgHashTable* markMap = NULL;

void markImportant(Addr varAddr){
  Addr callAddr = getCallAddr();
  MarkInfo* info = getMarkInfo(callAddr);
  tl_assert(info != NULL);
  ShadowValue* val = getMemShadow(varAddr);
  addInfluencesToMark(info, val->influences);
  if (print_errors || print_errors_long){
    printMarkInfo(info);
  }
  updateError(&(info->eagg), val->real, *(double*)varAddr);
}

MarkInfo* getMarkInfo(Addr callAddr){
  MarkInfo* markInfo = VG_(HT_lookup)(markMap, callAddr);
  if (markInfo == NULL){
    markInfo = VG_(perm_malloc)(sizeof(MarkInfo), vg_alignof(MarkInfo));
    markInfo->addr = callAddr;
    markInfo->influences = NULL;
    markInfo->eagg.max_error = -1;
    markInfo->eagg.total_error = 0;
    markInfo->eagg.num_evals = 0;
    VG_(HT_add_node)(markMap, markInfo);
  }
  return markInfo;
}

void addInfluencesToMark(MarkInfo* info, InfluenceList influences){
  for(InfluenceList curNode = influences; curNode != NULL; curNode = curNode->next){
    addInfluenceToMarkIfNotAlreadyThere(info, curNode->item);
  }
}

void addInfluenceToMarkIfNotAlreadyThere(MarkInfo* info, ShadowOpInfo* influence){
  for(InfluenceList curNode = info->influences; curNode != NULL; curNode = curNode->next){
    if (curNode->item == influence){
      return;
    }
  }
  lpush(InfluenceList)(&(info->influences), influence);
}

void printMarkInfo(MarkInfo* info){
  VG_(printf)("At ");
  ppAddr(info->addr);
}
