/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie           runtime-util ---*/
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

#include "runtime-util.h"
#include "pub_tool_stacktrace.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"
#include <math.h>

#define NCALLFRAMES 5
Addr getCallAddr(void){
  Addr trace[NCALLFRAMES];
  UInt nframes = VG_(get_StackTrace)(VG_(get_running_tid)(),
                                     trace, NCALLFRAMES, // Is this right?
                                     NULL, NULL,
                                     0);
  for(int i = 0; i < nframes; ++i){
    Addr addr = trace[i];
    // Basically, this whole block discards addresses which are part
    // of the redirection process or internal to the replacement
    // function, and are "below" the location of the call in the calls
    // stack. Currently it looks like we really only have to look at
    // the second frame up, but screw it, this probably isn't the
    // performance bottleneck, and it might be nice to have the
    // robustness somewhere down the line.
    const HChar* filename;
    if (!VG_(get_filename)(addr, &filename)) continue;
    if (VG_(strcmp)(filename, "mathwrap.c") == 0) continue;
    if (VG_(strcmp)(filename, "printf-wrap.c") == 0) continue;
    return addr;
  }
  return 0;
}

void printBBufFloat(BBuf* buf, double val){
  int i = 0;
  if (val != val){
    printBBuf(buf, "+nan.0");
  } else if (val == INFINITY){
    printBBuf(buf, "+inf.0");
  } else if (val == -INFINITY){
    printBBuf(buf, "-inf.0");
  } else if (val > 0 && val < 1){
    while (val < 1) {
      val *= 10;
      i++;
    }
    printBBuf(buf, "%fe-%d", val, i);
  } else if (val < 0 && val > -1){
    while (val > -1) {
      val *= 10;
      i++;
    }
    printBBuf(buf, "%fe-%d", val, i);
    // Floating-point error can make this value 10 - eps when it's
    // supposed to be 10, and valgrind will round it to 6 digits when
    // printing, producing 10.
  } else if (val >= 9.9999999){
    while (val >= 9.9999999){
      val /= 10;
      i++;
    }
    printBBuf(buf, "%fe%d", val, i);
  } else if (val <= -9.9999999){
    while (val <= -9.9999999){
      val /= 10;
      i++;
    }
    printBBuf(buf, "%fe%d", val, i);
  } else {
    printBBuf(buf, "%f", val);
  }
}

VG_REGPARM(1) void ppFloat_wrapper(UWord value){
  ppFloat(*(double*)(void*)&value);
}
void ppFloat(double val){
  int i = 0;
  if (val != val){
    VG_(printf)("+nan.0");
  } else if (val == INFINITY){
    VG_(printf)("+inf.0");
  } else if (val == -INFINITY){
    VG_(printf)("-inf.0");
  } else if (val > 0 && val < 1){
    while (val < 1) {
      val *= 10;
      i++;
    }
    VG_(printf)("%fe-%d", val, i);
  } else if (val < 0 && val > -1){
    while (val > -1) {
      val *= 10;
      i++;
    }
    VG_(printf)("%fe-%d", val, i);
  } else if (val >= 9.9999999){
    while (val >= 9.9999999){
      val /= 10;
      i++;
    }
    VG_(printf)("%fe%d", val, i);
  } else if (val <= -9.9999999){
    while (val <= -9.9999999){
      val /= 10;
      i++;
    }
    VG_(printf)("%fe%d", val, i);
  } else {
    VG_(printf)("%f", val);
  }
}
