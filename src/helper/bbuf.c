/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie                 bbuf.c ---*/
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

#include "bbuf.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"

BBuf* mkBBuf(int bound, char* buf){
  BBuf* res = VG_(malloc)("bounded buffer", sizeof(BBuf));
  res->bound = bound;
  res->buf = buf;
  return res;
}
void printBBuf(BBuf* bbuf, const char* format, ...){
  va_list arglist;
  va_start(arglist, format);
  int printLength = VG_(vsnprintf)(bbuf->buf, bbuf->bound, format,
                                   arglist);
  if (printLength >= bbuf->bound){
    tl_assert2(printLength < bbuf->bound,
               "trying to print %d character past bound!\n",
               printLength - bbuf->bound);
  }
  va_end(arglist);
  bbuf->bound -= printLength;
  bbuf->buf += printLength;
}
