
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            hg_opinfo.c ---*/
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

#include "hg_opinfo.h"
#include "hg_stemtea.h"
#include "../include/hg_macros.h"
#include "pub_tool_debuginfo.h"

void getOpDebug_Info(Addr op_addr,
                     const HChar* plain_opname,
                     const HChar* symbol,
                     OpDebug_Info* result){
  result->op_addr = op_addr;
  result->plain_opname = plain_opname;
  result->symbol = symbol;
  if (VG_(get_filename_linenum)(op_addr,
                                &(result->src_filename),
                                NULL,
                                &(result->src_line)) &&
      VG_(get_fnname)(op_addr, &(result->fnname)))
    return;
  result->src_filename = NULL;
  result->src_line = 0;
  result->fnname = NULL;
}
Op_Info* mkOp_Info(SizeT arity, IROp op, Addr opAddr,
                   const HChar* name, const HChar* symbol){
  Op_Info* result;
  ALLOC(result, "hg.op_info.1", 1, sizeof(Op_Info));
  ALLOC(result->arg_tmps, "hg.op_tmps", arity, sizeof(UWord));
  ALLOC(result->arg_values, "hg.op_values", arity, sizeof(UWord*));
  result->tag = Op_Branch;
  result->nargs = arity;
  result->op = op;
  getOpDebug_Info(opAddr, name, symbol, &(result->debuginfo));

  // Set the evalinfo up.

  // We want to give the evalinfo an initial max error of negative
  // one, instead of zero, for the corner case where the user wants to
  // report all operations. In that case, the user should set the
  // error threshold to zero. But ops only start getting tracked when
  // they weren't previously above the error threshold, and their new
  // value is. If the max error starts at zero, there will never come
  // that point, since the op starts out in a state we think it should
  // be tracked it, and we never know when to start tracking it. So
  // instead, set it to -1, so that any update of it to a non-negative
  // number will trigger a tracking.
  result->evalinfo.max_error = -1;

  return result;
}

Op_Info* mkLeafOp_Info(ShadowValue* val){
  Op_Info* result;
  ALLOC(result, "leaf op", 1, sizeof(Op_Info));
  result->tag = Op_Leaf;
  result->tea = NULL;
  return result;
}
