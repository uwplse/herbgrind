/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie      intercept-block.c ---*/
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

#include "intercept-block.h"

#include "pub_tool_debuginfo.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_machine.h"

#include "../runtime/wrap/printf-intercept.h"
#include "../helper/instrument-util.h"

#include <stdint.h>

const char * printfNames[] = {
  "camlPrintf__printf",
  "camlPrintf__fprintf",
};

Bool isPrefix(const char* prefix, const char* str){
  while(*prefix != '\0'){
    if (*str != *prefix){
      return False;
    }
    str++;
    prefix++;
  }
  return True;
}

void maybeInterceptBlock(IRSB* sbOut, void* blockAddr, void* srcAddr){
  const char * fnname;
  Bool isStart =
    VG_(get_fnname_if_entry)((uintptr_t)blockAddr, &fnname);
  if (isStart){
    const int numPrintfNames = sizeof(printfNames) / sizeof(const char*);
    for(int i = 0; i < numPrintfNames; ++i){
      if (isPrefix(printfNames[i], fnname)){
        for(int j = 0; j < MAX_THREADSTATE_FLOAT_ARGS; ++j){
          addStoreC(sbOut, runGet64C(sbOut, 224 + 32 * j), &(doubleArgs[j]));
        }
        addStmtToIRSB(sbOut, IRStmt_Dirty(unsafeIRDirty_0_N(3, "interceptPrintf", VG_(fnptr_to_fnentry)(interceptPrintf), mkIRExprVec_3(mkU64((uintptr_t)srcAddr), runGet64C(sbOut, 48), runGet64C(sbOut, 40)))));
        break;
      }
    }
  }
}

/* ------------------------------------------

   The thread state location TS(72), as far as I can tell, contains a
   pointer to the format string going in to the function.

   If you give printf 8 double arguments, they will be stored in
   TS(224), TS(256), TS(288), TS(320),
   TS(352), TS(384), TS(416), and TS(448).
   Other, non-floating point arguments don't seem to change that placement.

   I don't know where floating point args go afte that, probably the
   stack. But for now, let's not worry about it.

   ------*/
