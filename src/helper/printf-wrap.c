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
#include <assert.h>
#include <regex.h>

int isFloatPrintfSpecifier(const char* str);
int isFloatPrintfSpecifier(const char* str){
  regex_t regex;
  int reti;

  reti = regcomp(&regex,
                 "%[-+ #0]*[[:digit:]]*\\*\\{0,1\\}\\(\\.[[:digit:]]\\{1,\\}\\)\\{0,1\\}l\\{0,1\\}[gfeaGFEA].*",
                 0);
  assert(reti == 0);
  reti = regexec(&regex, str, 0, NULL, 0);
  regfree(&regex);
  if (reti == 0){
    return 1;
  } else if (reti == REG_NOMATCH){
    return 0;
  }
  assert(0);
  return 0;
}
int isPointerPrintfSpecifier(const char* str);
int isPointerPrintfSpecifier(const char* str){
  regex_t regex;
  int reti;

  reti = regcomp(&regex,
                 "%[-+ #0]*[[:digit:]]*\\*\\{0,1\\}\\(\\.[[:digit:]]\\{1,\\}\\)\\{0,1\\}[ps].*",
                 0);
  assert(reti == 0);
  reti = regexec(&regex, str, 0, NULL, 0);
  regfree(&regex);
  if (reti == 0){
    return 1;
  } else if (reti == REG_NOMATCH){
    return 0;
  }
  assert(0);
  return 0;
}

int VG_REPLACE_FUNCTION_ZU(VG_Z_LIBC_SONAME, printf)(const char* format, ...);
int VG_REPLACE_FUNCTION_ZU(VG_Z_LIBC_SONAME, printf)(const char* format, ...){
  va_list args;
  va_start(args, format);
  int numFloatArgs = 0;
  for (const char* p = format; *p != '\0'; p++){
    if (*p == '%'){
      if (isFloatPrintfSpecifier(p)){
        numFloatArgs += 1;
      }
    }
  }
  int fArgIdx = 0;
  for (const char* p = format; *p != '\0'; p++){
    if (*p == '%'){
      if (isFloatPrintfSpecifier(p)){
        double arg = va_arg(args, double);
        HERBGRIND_MAYBE_MARK_IMPORTANT_WITH_INDEX(arg, fArgIdx, numFloatArgs);
        fArgIdx += 1;
      } else if (isPointerPrintfSpecifier(p)){
        va_arg(args, void*);
      } else {
        va_arg(args, int);
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
