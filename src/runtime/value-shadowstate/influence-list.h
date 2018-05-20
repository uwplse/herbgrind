/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie       influence-list.h ---*/
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

#ifndef _INFLUENCE_LIST_H
#define _INFLUENCE_LIST_H

#include "../op-shadowstate/shadowop-info.h"

typedef struct _influenceList{
  struct _influenceList* next;
  int length;
  ShadowOpInfo** data;
} *InfluenceList;

InfluenceList mkInfluenceList(void);
void freeInfluenceList(InfluenceList il);
InfluenceList mergeInfluences(InfluenceList il1, InfluenceList il2,
                              ShadowOpInfo* extra);
void ppInfluences(InfluenceList influences);
void assertNoDropInfluences(InfluenceList influences1,
                            InfluenceList influences2,
                            InfluenceList merged);
Bool hasInfluence(InfluenceList list, ShadowOpInfo* influence);

#endif
