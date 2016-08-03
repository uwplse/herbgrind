
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            hg_opinfo.h ---*/
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

#ifndef _HG_OPINFO
#define _HG_OPINFO

#include "hg_opinfo.hh"
#include "hg_shadowvals.hh"
#include "hg_stemtea.hh"

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_xarray.h"

// When I was looking through the FpDebug source as inspiration for
// this project, I kept seeing these structures all over the place
// that looked like they only were really meant to be used and passed
// around in one place, and I kept wondering why you would use them
// when there are easier ways to pass things around, like usually as
// seperate arguments. Now that I've messed around with valgrind
// enough, I finally understand that these weird structures are used
// to get around the three argument restriction that valgrind puts on
// inserting client calls.
struct _LoadG_Info {
  UWord cond;
  Addr src_mem;
  UWord alt_tmp;
  UWord dest_tmp;
  IRType dest_type;
};

struct _CpShadow_Info {
  UWord src_idx;
  UWord dest_idx;
  Addr instr_addr;
  IRType type;
};

struct _OpDebug_Info {
  // The address of the operation in the binary
  Addr op_addr;
  // The source line of the operation
  UInt src_line;
  // The source file of the operation
  const HChar* src_filename;
  // The function the operation resides in.
  const HChar* fnname;
  // The plaintext name of the operation, like "subtraction"
  const HChar* plain_opname;
  // The function symbol of the operation (e.g. '+' or 'sin')
  const HChar* symbol;
};

void getOpDebug_Info(Addr op_addr,
                     const HChar* plain_opname,
                     const HChar* symbol,
                     OpDebug_Info* result);

struct _Eval_Info {
  // The most error the output of this operation has ever had.
  double max_error;
  // All the bits of error this operation has had, added together
  double total_error;
  // The number of times we've evaluated the error of this operation.
  SizeT num_calls;
};

typedef enum {
  Op_Branch,
  Op_Leaf,
} OpInfoType;

struct _Op_Info {
  OpInfoType tag;
  // The arity of the operation. This determines which of the args
  // structures we are allowed to put in and pull out.
  SizeT nargs;
  // The VEX op code of the operation.
  IROp op;
  // Information about where the operation resides, for reporting
  // error to the user.
  OpDebug_Info debuginfo;
  // Information about the evaluated behaviour of the operation
  Eval_Info evalinfo;
  // An tea head representing the most specific expression general
  // enough to capture all seen inputs to this op.
  TeaNode* tea;
  //This is the index into where we're putting the result.
  UWord dest_tmp;
  // This is the actual computed value of the result, for checking
  // accuracy.
  UWord* dest_value;
  // The argument information for the op

  // These are the indicies of the temporaries that hold these
  // values. We'll use these indices to index into the shadow value
  // array too.
  UWord* arg_tmps;

  // If we don't have a shadow value yet for these arguments, we're
  // going to use the existing float values to create one. These float
  // values can be as big as 256 bits, since we account for SIMD
  // locations, so we're going to store them as an array that's
  // malloc'd when we know how big they're going to be.
  UWord** arg_values;
};

struct _Op_Infos_ptr {
  // This member is here to make this structure compatible with the
  // hash table implementation in pub_tool_hashtable. None of our code
  // will actually use it.
  struct _Op_Infos_ptr* next;
  // This part is also here for the hash table structure, but we'll
  // actually be messing with it as we'll set it to the key.
  UWord addr;
  // The actual opinfo we're pointing too.
  XArray* infos;
};

Op_Info* mkOp_Info(SizeT arity, IROp op, Addr opAddr,
                   const HChar* name, const HChar* symbol);

Op_Info* mkLeafOp_Info(ShadowValue* val);

#ifdef VG_LITTLEENDIAN
#define ENDIAN Iend_LE
#else
#define ENDIAN Iend_BE;
#endif

#endif
