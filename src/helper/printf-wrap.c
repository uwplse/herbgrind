/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie          printf-wrap.c ---*/
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

#include "pub_tool_redir.h"
#include "../include/herbgrind.h"

#include <stdarg.h>
#include <stdio.h>

int VG_REPLACE_FUNCTION_ZU(VG_Z_LIBC_SONAME, printf)(const char* format, ...);
int VG_REPLACE_FUNCTION_ZU(VG_Z_LIBC_SONAME, printf)(const char* format, ...){
  va_list args;
  va_start(args, format);
  int numFloatArgs = 0;
  for (const char* p = format; *p != '\0'; p++){
    if (*p == '%'){
      switch(*(p + 1)){
      case 'e':
      case 'f':
        numFloatArgs += 1;
        break;
      default:
        break;
      }
    }
  }
  int fArgIdx = 0;
  for (const char* p = format; *p != '\0'; p++){
    if (*p == '%'){
      switch(*(p + 1)){
      case 'e':
      case 'f':{
        double arg = va_arg(args, double);
        HERBGRIND_MAYBE_MARK_IMPORTANT_WITH_INDEX(arg, fArgIdx, numFloatArgs);
        fArgIdx += 1;
      }
        break;
      case 'd':
      case 'x':
      case 'X':
      case 'l':
        va_arg(args, int);
        break;
      case 's':
      case 'p':
        va_arg(args, void*);
      default:
        tl_assert2(0, "We don't support the printf format specifier %%%c!\n", *(p + 1));
        break;
      }
    }
  }
  va_end(args);
  va_start(args, format);
  HERBGRIND_END();
  int result = vprintf(format, args);
  HERBGRIND_BEGIN();
  return result;
}
