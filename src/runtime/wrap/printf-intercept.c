/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie     printf-intercept.c ---*/
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

#include "printf-intercept.h"
#include "../../helper/runtime-util.h"
#include "../value-shadowstate/shadowval.h"
#include "../value-shadowstate/value-shadowstate.h"
#include "../op-shadowstate/marks.h"

#include "pub_tool_libcprint.h"
#include "pub_tool_libcbase.h"

#include <stdint.h>

double doubleArgs[MAX_THREADSTATE_FLOAT_ARGS];

VG_REGPARM(2)
void interceptPrintf(Addr address, void* stackFrame,
                     ocamlFString* formatStringObject){
  char* formatString = formatStringObject->string;
  void* nextArg = (char*)stackFrame + 8;
  int numFloatArgs = 0;
  /* VG_(printf)("Intercepting printf at %#lX with format %s\n", address, formatString); */
  for(char* p = formatString; *p != '\0'; ++p){
    if (*p == '%'){
      switch(*(p + 1)){
      case 'e':
      case 'g':
      case 'f':
        numFloatArgs += 1;
        break;
      default: {
        /* int* argLoc = nextArg; */
        nextArg = (char*)nextArg + 8;
        /* VG_(printf)("argLoc is %p\n", argLoc); */
        /* int arg = (*argLoc) >> 1; */
        /* VG_(printf)("Found integer arg %d with format specifier %%d\n", arg); */
      }
        break;
      }
    }
  }
  int curArg = 1;
  for(char* p = formatString + VG_(strlen)(formatString); p >= formatString; --p){
    if (*p == '%'){
      switch(*(p + 1)){
      case 'e':
      case 'g':
      case 'f':{
        double* argLoc = nextArg;
        nextArg = (char*)nextArg + 8;
        /* VG_(printf)("argLoc is %p\n", argLoc); */
        maybeMarkImportantAtAddr(getMemShadow((uintptr_t)(void*)argLoc),
                                 *argLoc, numFloatArgs - curArg, numFloatArgs, address);
        /* VG_(printf)("Found double arg "); */
        /* ppFloat(*argLoc); */
        /* VG_(printf)(" for with format specifier %%%c\n", *(p + 1)); */
        curArg += 1;
      }
        break;
      }
    }
  }
}
