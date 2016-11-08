/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            herbgrind.h ---*/
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

#ifndef _HG_OUTPUT_H
#define _HG_OUTPUT_H

#include "pub_tool_basics.h"
#include "../types/hg_shadowvals.h"

typedef struct _OutputMark {
  Op_Info* op;
  Addr addr;
  UInt src_line;
  const HChar* src_filename;
  const HChar* fnname;
} OutputMark;

OutputMark* mkMark(Op_Info* op, Addr curAddr);

void dedupAdd(XArray* array, void* item);

void markValueImportant(ShadowValue* shadowVal);
void propagateInfluences(ShadowValue* dest, int nargs, ...);
void trackValueExpr(ShadowValue* val);

void clearInfluence(Op_Info* opinfo, XArray* influences);
void recursivelyClearChildren(TeaNode* _node, XArray* influences);
Word cmp_debuginfo(const Op_Info** a, const Op_Info** b);
void writeReport(const char* filename);

#endif
