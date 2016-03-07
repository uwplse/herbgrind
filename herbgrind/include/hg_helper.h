
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            hg_helper.h ---*/
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

#ifndef _HG_HELPER
#define _HG_HELPER

#include "pub_tool_basics.h"

const char* IRExprTagString(IRExprTag tag);
const char* IRExprTagString(IRExprTag tag){
  switch(tag){
  case Iex_Binder:
    return "Iex_Binder";
  case Iex_Get:
    return "Iex_Get";
  case Iex_GetI:
    return "Iex_GetI";
  case Iex_RdTmp:
    return "Iex_RdTmp";
  case Iex_Qop:
    return "Iex_Qop";
  case Iex_Triop:
    return "Iex_Triop";
  case Iex_Binop:
    return "Iex_Binop";
  case Iex_Unop:
    return "Iex_Unop";
  case Iex_Load:
    return "Iex_Load";
  case Iex_Const:
    return "Iex_Const";
  case Iex_ITE:
    return "Iex_ITE";
  case Iex_CCall:
    return "Iex_CCall";
  case Iex_VECRET:
    return "Iex_VECRET";
  case Iex_BBPTR:
    return "Iex_BBPTR";
  }
  return "****";
}

#endif
