/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie             shadowop.c ---*/
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

#include "shadowop-info.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_libcprint.h"
#include "../../helper/ir-info.h"

ShadowOpInfo* mkShadowOpInfo(IROp op_code, Addr op_addr, Addr block_addr,
                             int nargs){
  ShadowOpInfo* result = VG_(perm_malloc)(sizeof(ShadowOpInfo), vg_alignof(ShadowOpInfo));
  result->op_code = op_code;
  result->op_addr = op_addr;
  result->block_addr = block_addr;
  result->eagg.max_total_error = -1;
  result->eagg.total_total_error = 0;
  result->eagg.max_local_error = -1;
  result->eagg.total_local_error = 0;
  result->expr = NULL;
  result->exinfo.numSIMDOperands =
    op_code == 0x0 ? 1 : numSIMDOperands(op_code);
  result->exinfo.numChannels =
    op_code == 0x0 ? 1 : numChannelsOut(op_code);
  result->exinfo.nargs = nargs;
  result->exinfo.argPrecision =
    op_code == 0x0 ? Ft_Double : argPrecision(op_code);
  return result;
}

void printOpInfo(ShadowOpInfo* opinfo){
  if (opinfo->op_code == 0){
    VG_(printf)("%s", opinfo->name);
  } else {
    ppIROp(opinfo->op_code);
  }
  const HChar* src_filename;
  const HChar* fnname;
  UInt src_line;
  if (VG_(get_filename_linenum)(opinfo->op_addr, &src_filename,
                                NULL, &src_line)){
    VG_(get_fnname)(opinfo->op_addr, &fnname);
    VG_(printf)(" at %s:%u in %s (addr %lX)",
                src_filename, src_line, fnname, opinfo->op_addr);
  } else {
    VG_(printf)(" at addr %lX)",
                opinfo->op_addr);
  }
}
