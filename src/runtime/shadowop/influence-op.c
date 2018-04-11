/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie         influence-op.c ---*/
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

#include "influence-op.h"
#include "pub_tool_libcprint.h"
#include "../value-shadowstate/value-shadowstate.h"
#include "../value-shadowstate/exprs.h"

void execInfluencesOp(ShadowOpInfo* info,
                      InfluenceList* res, ShadowValue** args,
                      Bool flagged){
  if (flagged && print_flagged){
    VG_(printf)("Hit local error! ");
    printOpInfo(info);
    VG_(printf)("\n");
  }
  if (no_influences){
    return;
  }
  if (numFloatArgs(info) == 1){
    *res = mergeInfluences(args[0]->influences, NULL,
                           flagged ? info : NULL);
  } else if (numFloatArgs(info) == 2){
    *res = mergeInfluences(args[0]->influences, args[1]->influences,
                           flagged ? info : NULL);
  } else {
    tl_assert(numFloatArgs(info) == 3);
    InfluenceList intermediary = mergeInfluences(args[0]->influences,
                                                 args[1]->influences,
                                                 flagged ? info : NULL);

    *res = mergeInfluences(intermediary, args[2]->influences, NULL);
    freeInfluenceList(intermediary);
  }
}

void inPlaceMergeInfluences(InfluenceList* dest, InfluenceList arg){
  InfluenceList lst = mergeInfluences(*dest, arg, NULL);
  if (*dest != NULL){
    freeInfluenceList(*dest);
  }
  *dest = lst;
}
void trackOpAsInfluence(ShadowOpInfo* info, ShadowValue* value){
  if (no_influences){
    return;
  }
  InfluenceList lst = mergeInfluences(value->influences, NULL, info);
  if (value->influences != NULL){
    freeInfluenceList(value->influences);
  }
  value->influences = lst;
}
InfluenceList cloneInfluences(InfluenceList influences){
  return mergeInfluences(influences, NULL, NULL);
}

void forceTrack(Addr varAddr){
  ShadowValue* val = getMemShadow(varAddr);
  ShadowOpInfo* info = val->expr->branch.op;
  VG_(printf)("Tracking %p\n", val);
  trackOpAsInfluence(info, val);
}
void forceTrackF(Addr varAddr){
}
