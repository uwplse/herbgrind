
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie          hg_shadowop.c ---*/
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

// The declarations for the function we implement.
#include "hg_shadowop.h"

// Some functions and definitions that our implementation depends on.
#include "hg_hiprec_ops.h"
#include "hg_storage_runtime.h"
#include "../include/hg_options.h"
#include "../include/hg_macros.h"
#include "../types/hg_stemtea.h"
#include "hg_runtime.h"

// Execute a shadow operation, storing the result of the high
// precision operation applied to the shadow value at the arg_tmp
// offset, into the shadow value at the dest_tmp offset. Depending on
// the value of op, we'll know which operation to apply. op will be
// mapped to operations by the enumeration at libvex_ir.h:415.
VG_REGPARM(1) void executeUnaryShadowOp(Op_Info* opInfo){
  return;
  ShadowLocation* argLocation;
  ShadowLocation* destLocation;
  ShadowValue* arg;
  ShadowValue* dest;

  if (!running) return;

  switch(opInfo->op){
    // All the math-y operations
  case Iop_RecipEst32Fx8:
  case Iop_RSqrtEst32Fx8:
  case Iop_RecipEst32Fx4:
  case Iop_RSqrtEst32Fx4:
  case Iop_RecipEst64Fx2:
  case Iop_RSqrtEst64Fx2:
  case Iop_RecipEst32Fx2:
  case Iop_RSqrtEst32Fx2:
  case Iop_Abs64Fx2:
  case Iop_Neg64Fx2:
  case Iop_Neg32Fx2:
  case Iop_Abs32Fx2:
  case Iop_Neg32Fx4:
  case Iop_Abs32Fx4:
  case Iop_NegF32:
  case Iop_AbsF32:
  case Iop_NegF64:
  case Iop_AbsF64:
  case Iop_Sqrt64F0x2:
  case Iop_RecipEst32F0x4:
  case Iop_Sqrt32F0x4:
  case Iop_RSqrtEst32F0x4:
  case Iop_RSqrtEst5GoodF64:
    {
      // We keep track of three attributes for each of these
      // instructions: what function it performs, what register
      // size/type does it expect to work with, and how many values of
      // it's input should be processed. This last one isn't
      // inherently tied to the register type because some
      // instructions here will act on a four channel float, but only
      // perform an operation on one channel, and copy the others across.
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_rnd_t);
      LocType argType;
      size_t num_vals;

      // Figure out the underlying math function of the
      // operation. Some are not in the mpfr library directly, but are
      // a combination of mpfr functions with the right arguments, so
      // I added wrappers in hg_hiprec_ops.c to let us do this
      // uniformly.
      switch(opInfo->op){
      case Iop_Sqrt32F0x4:
      case Iop_Sqrt64F0x2:
        mpfr_func = mpfr_sqrt;
        break;
      case Iop_RecipEst32F0x4:
      case Iop_RecipEst32Fx2:
      case Iop_RecipEst32Fx4:
      case Iop_RecipEst32Fx8:
      case Iop_RecipEst64Fx2:
        mpfr_func = hiprec_recip;
        break;
      case Iop_RSqrtEst32F0x4:
      case Iop_RSqrtEst32Fx2:
      case Iop_RSqrtEst32Fx4:
      case Iop_RSqrtEst32Fx8:
      case Iop_RSqrtEst5GoodF64:
      case Iop_RSqrtEst64Fx2:
        mpfr_func = mpfr_rec_sqrt;
        break;
      case Iop_NegF32:
      case Iop_Neg32Fx2:
      case Iop_Neg32Fx4:
      case Iop_NegF64:
      case Iop_Neg64Fx2:
        mpfr_func = mpfr_neg;
        break;
      case Iop_AbsF32:
      case Iop_Abs32Fx2:
      case Iop_Abs32Fx4:
      case Iop_AbsF64:
      case Iop_Abs64Fx2:
        mpfr_func = mpfr_abs;
        break;
      default:
        return;
      }

      // Now we're going to determine the size, and how many values
      // should be operated on (the rest will be copied).
      switch(opInfo->op){
      case Iop_RecipEst32Fx2:
      case Iop_RSqrtEst32Fx2:
      case Iop_Neg32Fx2:
      case Iop_Abs32Fx2:
        argType = Lt_Floatx2;
        num_vals = 2;
        break;
      case Iop_RecipEst64Fx2:
      case Iop_RSqrtEst64Fx2:
      case Iop_Abs64Fx2:
      case Iop_Neg64Fx2:
        argType = Lt_Doublex2;
        num_vals = 2;
        break;
      case Iop_RecipEst32Fx4:
      case Iop_RSqrtEst32Fx4:
      case Iop_Neg32Fx4:
      case Iop_Abs32Fx4:
        argType = Lt_Floatx4;
        num_vals = 4;
        break;
      case Iop_RecipEst32Fx8:
      case Iop_RSqrtEst32Fx8:
        argType = Lt_Floatx8;
        num_vals = 8;
        break;
      case Iop_NegF32:
      case Iop_AbsF32:
        argType = Lt_Float;
        num_vals = 1;
        break;
      case Iop_NegF64:
      case Iop_AbsF64:
      case Iop_RSqrtEst5GoodF64:
        argType = Lt_Double;
        num_vals = 1;
        break;
      case Iop_Sqrt32F0x4:
      case Iop_RecipEst32F0x4:
      case Iop_RSqrtEst32F0x4:
        argType = Lt_Floatx4;
        num_vals = 1;
        break;
      case Iop_Sqrt64F0x2:
        argType = Lt_Doublex2;
        num_vals = 1;
        break;
      default:
        return;
      }
      
      // Pull the shadow values for the argument. If we don't already
      // have shadow values for this argument, we'll generate a fresh
      // one from the runtime float value.
      argLocation = getShadowLocation(opInfo->arg_tmps[0],
                                      argType);
      // Allocate space for the result
      destLocation = mkShadowLocation_bare(argType);
      if (print_inputs){
        VG_(printf)("Shadow arg: ");
        printShadowLoc(argLocation);
        VG_(printf)("\n");
      }
      int i;
      for (i = 0; i < num_vals; ++i){
        arg = getShadowValue(argLocation, i,
                             opInfo->arg_values[i]);
        dest = mkShadowValue();
        destLocation->values[i] = dest;
        if (print_inputs){
          VG_(printf)("Computed arg, part %d: %f\n",
                      i, ((double*)opInfo->arg_values[0])[i]);
        }
        mpfr_func(dest->value, arg->value, MPFR_RNDN);
        // Set up the stem record of this operation.
        initBranchStemNode(destLocation->values[i], opInfo, 1,
                           argLocation->values[i]);
        // Evaluate the computed value against the high precision
        // shadow result.
        evaluateOpError_helper(destLocation->values[i],
                               argType, i,
                               opInfo);
      }
      // Copy across the rest of the values from the argument
      for (;i < capacity(argType); ++i){
        copySV(getShadowValue(argLocation, i, opInfo->arg_values[i]), &(destLocation->values[i]));
      }
    }
    break;
    // Operations for copying across part of or all of a number.
  case Iop_F32toF64:
    // These next two we'll treat as no-ops, since we're shadowing
    // either type of value with the same number of bits.
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF32:
  case Iop_ZeroHI64ofV128:
  case Iop_ZeroHI96ofV128:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_V128to32:
  case Iop_64UtoV128:
  case Iop_SetV128lo64:
    {
      LocType resultType;
      // Get the input and output location types for allocating
      // storage for each.
      switch(opInfo->op){
      case Iop_F32toF64:
        resultType = Lt_Double;
        break;
      case Iop_TruncF64asF32:
      case Iop_RoundF64toF32:
        resultType = Lt_Float;
        break;
      case Iop_ZeroHI64ofV128:
        resultType = Lt_Doublex2;
        break;
      case Iop_ZeroHI96ofV128:
        resultType = Lt_Floatx4;
        break;
      case Iop_F128HItoF64:
      case Iop_F128LOtoF64:
      case Iop_V128to64:
      case Iop_V128HIto64:
        resultType = Lt_Double;
        break;
      case Iop_V128to32:
        resultType = Lt_Float;
        break;
      case Iop_64UtoV128:
      case Iop_SetV128lo64:
        resultType = Lt_Doublex2;
        break;
      default:
        return;
      }
      // Get the input location. If it don't exist, skip the whole
      // thing, and overwrite the destination with NULL.
      argLocation = getTemp(opInfo->arg_tmps[0]);
      // Initialize the output location.
      if (argLocation == NULL){
        destLocation = NULL;
        break;
      }
      destLocation = mkShadowLocation_bare(resultType);

      // Depending on the op, either copy across the first half of the
      // argument, or the second half. Some of these ops copy across
      // the whole thing, but since the types are set right, that's
      // like copying across the first element.
      switch(opInfo->op){
      case Iop_V128HIto64:
      case Iop_F128HItoF64:
        copySV(argLocation->values[1], &(destLocation->values[0]));
        break;
      case Iop_F32toF64:
      case Iop_TruncF64asF32:
      case Iop_RoundF64toF32:
      case Iop_ZeroHI64ofV128:
      case Iop_ZeroHI96ofV128:
      case Iop_V128to64:
      case Iop_V128to32:
      case Iop_F128LOtoF64:
      case Iop_64UtoV128:
      case Iop_SetV128lo64:
        copySV(argLocation->values[0], &(destLocation->values[0]));
        break;
      default:
        return;
      }
    }
    break;
    // Operations for rounding floats to integer values, but keeping
    // them as float types.
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
    // Set up space for the argument and result shadow values.
    argLocation = getTemp(opInfo->arg_tmps[0]);
    if (argLocation == NULL){
      destLocation = NULL;
      break;
    }
    destLocation = mkShadowLocation(Lt_Double);
    if (report_exprs){
      copyStemNode(argLocation->values[0]->stem,
                   &(destLocation->values[0]->stem));
    }

    // Perform the mpfr rounding to int that matches the requested
    // rounding type.
    switch(opInfo->op){
    case Iop_RoundF64toF64_NEAREST:
      mpfr_round(destLocation->values[0]->value, argLocation->values[0]->value);
      break;
    case Iop_RoundF64toF64_NegINF:
      mpfr_floor(destLocation->values[0]->value, argLocation->values[0]->value);
      break;
    case Iop_RoundF64toF64_PosINF:
      mpfr_ceil(destLocation->values[0]->value, argLocation->values[0]->value);
      break;
    case Iop_RoundF64toF64_ZERO:
      mpfr_trunc(destLocation->values[0]->value, argLocation->values[0]->value);
      break;
    default:
      return;
    }
    break;
  default:
    return;
  }

  // Put the resulting location in the space for the dest temp.
  if (print_moves && destLocation != NULL){
    VG_(printf)("Putting shadow location ");
    printShadowLoc(destLocation);
    VG_(printf)(" in temp %lu.\n", opInfo->dest_tmp);
  }
  setTemp(opInfo->dest_tmp, destLocation);

}
VG_REGPARM(1) void executeBinaryShadowOp(Op_Info* opInfo){
  // The shadowing locations for the first argument, second argument,
  // and destination respectively.
  ShadowLocation* arg1Location;
  ShadowLocation* arg2Location;
  ShadowLocation* destLocation = NULL;
  ShadowValue* arg1 = NULL;
  ShadowValue* arg2 = NULL;

  if (!running) return;
  switch(opInfo->op){

  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
    break;
    // Pull the shadow locations for the arguments. If we don't
    // already have a shadow location for one argument, but we do for
    // the other, we'll generate a fresh location from the runtime
    // float value.
    if (getTemp(opInfo->arg_tmps[0]) == NULL &&
        getTemp(opInfo->arg_tmps[1]) == NULL){
      destLocation = NULL;
      break;
    }
    arg1Location = getShadowLocation(opInfo->arg_tmps[0],
                                     Lt_Double);
    arg2Location = getShadowLocation(opInfo->arg_tmps[1],
                                     Lt_Double);
    arg1 = getShadowValue(arg1Location, 0,
                          opInfo->arg_values[0]);
    arg2 = getShadowValue(arg2Location, 0,
                          opInfo->arg_values[1]);

    // Now we'll allocate memory for the shadowed result of this
    // operation.
    destLocation = mkShadowLocation_bare(Lt_Doublex2);

    // Finally, take the 64 bits of each argument, and put them in the
    // two halves of the result.
    copySV(arg1, &(destLocation->values[0]));
    copySV(arg2, &(destLocation->values[1]));
    break;

  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
    break;
    {
      LocType argType;
      if (getTemp(opInfo->arg_tmps[1]) == NULL){
        destLocation = NULL;
        break;
      }
      switch(opInfo->op){
      case Iop_RoundF64toInt:
        argType = Lt_Double;
        break;
      case Iop_RoundF32toInt:
        argType = Lt_Float;
        break;
      default:
        return;
      }
      arg2Location = getShadowLocation(opInfo->arg_tmps[1],
                                       argType);
      arg2 = getShadowValue(arg2Location, 0,
                            opInfo->arg_values[1]);
      destLocation = mkShadowLocation_bare(argType);
      destLocation->values[0] = mkShadowValue();
      if (report_exprs){
        copyStemNode(arg2->stem, &(destLocation->values[0]->stem));
      }
      mpfr_round(destLocation->values[0]->value, arg2->value);
    }
    break;
  case Iop_F64toF32:
    break;
    // For semantic conversions between floating point types we can
    // just copy across the values, if they're there.
    if (getTemp(opInfo->arg_tmps[1]) == NULL){
      destLocation = NULL;
      break;
    }
    arg2Location = getShadowLocation(opInfo->arg_tmps[1],
                                     Lt_Double);
    arg2 = getShadowValue(arg2Location, 0,
                          opInfo->arg_values[1]);
    destLocation = mkShadowLocation_bare(Lt_Float);
    copySV(arg2, &(destLocation->values[0]));
    break;

    // Ops that have a rounding mode and a single floating point argument
  case Iop_Sqrt32Fx8:
  case Iop_Sqrt32Fx4:
  case Iop_Sqrt64Fx4:
  case Iop_Sqrt64Fx2:
  case Iop_RecpExpF64:
  case Iop_RecpExpF32:
  case Iop_SinF64:
  case Iop_CosF64:
  case Iop_TanF64:
  case Iop_2xm1F64:
  case Iop_SqrtF64:
  case Iop_SqrtF32:
    break;
    {
      // We keep track of three attributes for each of these
      // instructions: what function it performs, what register
      // size/type does it expect to work with, and how many values of
      // it's input should be processed. This last one isn't
      // inherently tied to the register type because some
      // instructions here will act on a four channel float, but only
      // perform an operation on one channel, and copy the others across.
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_rnd_t);
      LocType argType;
      size_t num_values;


      // Figure out the underlying math function of the
      // operation. Some are not in the mpfr library directly, but are
      // a combination of mpfr functions with the right arguments, so
      // I added wrappers in hg_hiprec_ops.c to let us do this
      // uniformly.
      switch(opInfo->op){
      case Iop_RecpExpF64:
      case Iop_RecpExpF32:
        mpfr_func = hiprec_recpexp;
        break;
      case Iop_SinF64:
        mpfr_func = mpfr_sin;
        break;
      case Iop_CosF64:
        mpfr_func = mpfr_cos;
        break;
      case Iop_TanF64:
        mpfr_func = mpfr_tan;
        break;
      case Iop_2xm1F64:
        mpfr_func = hiprec_2xm1;
        break;
      case Iop_Sqrt32Fx8:
      case Iop_Sqrt32Fx4:
      case Iop_Sqrt64Fx2:
      case Iop_Sqrt64Fx4:
      case Iop_SqrtF32:
      case Iop_SqrtF64:
        mpfr_func = mpfr_sqrt;
        break;
      default:
        return;
      }

      // Now we're going to determine the size, and how many values
      // should be operated on.
      switch(opInfo->op){
      case Iop_RecpExpF32:
      case Iop_SqrtF32:
        argType = Lt_Float;
        num_values = 1;
        break;
      case Iop_RecpExpF64:
      case Iop_SinF64:
      case Iop_CosF64:
      case Iop_TanF64:
      case Iop_2xm1F64:
      case Iop_SqrtF64:
        argType = Lt_Double;
        num_values = 1;
        break;
      case Iop_Sqrt32Fx8:
        argType = Lt_Floatx8;
        num_values = 8;
        break;
      case Iop_Sqrt32Fx4:
        argType = Lt_Floatx4;
        num_values = 4;
        break;
      case Iop_Sqrt64Fx4:
        argType = Lt_Doublex4;
        num_values = 4;
        break;
      case Iop_Sqrt64Fx2:
        argType = Lt_Doublex2;
        num_values = 2;
        break;
      default:
        return;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg2Location =
        getShadowLocation(opInfo->arg_tmps[1],
                          argType);
      for (int i = 0; i < num_values; ++i)
        getShadowValue(arg2Location, i,
                       opInfo->arg_values[1]);
      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation_bare(argType);
      if (print_inputs){
        VG_(printf)("Shadow arg: ");
        printShadowLoc(arg2Location);
        VG_(printf)("\n");
      }
      for (int i = 0; i < num_values; ++i){

        if (print_inputs){
          VG_(printf)("Computed arg, part %d: %f\n",
                      i, ((double*)opInfo->arg_values[0])[i]);
        }
        // Initialize a shadow value in the given slot in
        // destLocation.
        destLocation->values[i] = mkShadowValue();
        // Set the low order bits to the result of the operation, but in
        // higher precision.
        mpfr_func(destLocation->values[i]->value,
                  arg2Location->values[i]->value,
                  MPFR_RNDN);
        // Set the stem record of this operation.
        initBranchStemNode(destLocation->values[i], opInfo, 1,
                           arg2Location->values[i]);

        // Now, we'll evaluate the shadow values against each
        // channel of the computed result.
        evaluateOpError_helper(destLocation->values[i],
                               argType, i,
                               opInfo);
      }
    }
    break;
    // Ops that have two floating point arguments
  case Iop_Add32F0x4:
  case Iop_Sub32F0x4:
  case Iop_Mul32F0x4:
  case Iop_Div32F0x4:
  case Iop_Add64F0x2:
  case Iop_Sub64F0x2:
  case Iop_Div64F0x2:
  case Iop_Add32Fx2:
  case Iop_Sub32Fx2:
  case Iop_Mul32Fx2:
    // case Iop_Div32Fx2: // This op doesn't actually exist, not clear why.
  case Iop_RecipStep32Fx4:
  case Iop_RSqrtStep32Fx4:
  case Iop_RSqrtStep32Fx2:
  case Iop_RecipStep32Fx2:
  case Iop_RecipStep64Fx2:
  case Iop_RSqrtStep64Fx2:
    break;
  case Iop_Mul64F0x2:
    {
      // We keep track of three attributes for each of these
      // instructions: what function it performs, what register
      // size/type does it expect to work with, and how many values of
      // it's input should be processed. This last one isn't
      // inherently tied to the register type because some
      // instructions here will act on a four channel float, but only
      // perform an operation on one channel, and copy the others across.
      int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
      LocType argType;
      size_t num_values;

      // Figure out the underlying math function of the
      // operation. Some are not in the mpfr library directly, but are
      // a combination of mpfr functions with the right arguments, so
      // I added wrappers in hg_hiprec_ops.c to let us do this
      // uniformly.
      switch(opInfo->op){
      case Iop_RSqrtStep32Fx4:
      case Iop_RSqrtStep32Fx2:
      case Iop_RSqrtStep64Fx2:
        mpfr_func = hiprec_rsqrtstep;
        break;
      case Iop_RecipStep32Fx4:
      case Iop_RecipStep32Fx2:
      case Iop_RecipStep64Fx2:
        mpfr_func = hiprec_recipstep;
        break;
      case Iop_Add64F0x2:
      case Iop_Add32F0x4:
      case Iop_Add32Fx2:
        mpfr_func = mpfr_add;
        break;
      case Iop_Sub64F0x2:
      case Iop_Sub32F0x4:
      case Iop_Sub32Fx2:
        mpfr_func = mpfr_sub;
        break;
      case Iop_Mul64F0x2:
      case Iop_Mul32F0x4:
      case Iop_Mul32Fx2:
        mpfr_func = mpfr_mul;
        break;
      case Iop_Div64F0x2:
      case Iop_Div32F0x4:
        mpfr_func = mpfr_div;
        break;
      default:
        return;
      }

      // Now we're going to determine the size, and how many values
      // should be operated on (the rest will be copied).
      switch(opInfo->op){
      case Iop_Add32Fx2:
      case Iop_Sub32Fx2:
      case Iop_Mul32Fx2:
      case Iop_RSqrtStep32Fx2:
      case Iop_RecipStep32Fx2:
        argType = Lt_Floatx2;
        num_values = 2;
        break;
      case Iop_RecipStep64Fx2:
      case Iop_RSqrtStep64Fx2:
        argType = Lt_Doublex2;
        num_values = 2;
        break;
      case Iop_RecipStep32Fx4:
      case Iop_RSqrtStep32Fx4:
        argType = Lt_Floatx4;
        num_values = 4;
        break;
      case Iop_Add32F0x4:
      case Iop_Sub32F0x4:
      case Iop_Mul32F0x4:
      case Iop_Div32F0x4:
        argType = Lt_Floatx4;
        num_values = 1;
        break;
      case Iop_Add64F0x2:
      case Iop_Sub64F0x2:
      case Iop_Mul64F0x2:
      case Iop_Div64F0x2:
        argType = Lt_Doublex2;
        num_values = 1;
        break;
      default:
        return;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg1Location =
        getShadowLocation(opInfo->arg_tmps[0],
                          argType);
      arg2Location =
        getShadowLocation(opInfo->arg_tmps[1],
                          argType);

      for(int i = 0; i < capacity(argType); ++i){
        getShadowValue(arg1Location, i,
                       opInfo->arg_values[0]);
        getShadowValue(arg2Location, i,
                       opInfo->arg_values[1]);
      }
  
      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation_bare(argType);

      if (print_inputs){
        VG_(printf)("Shadow first arg: ");
        printShadowLoc(arg1Location);
        VG_(printf)("\nShadow second arg: ");
        printShadowLoc(arg2Location);
        VG_(printf)("\n");
      }
      int i;
      for (i = 0; i < num_values; ++i){
        if (print_inputs){
          VG_(printf)("Computed first arg, part %d: %f\n"
                      "Computed second arg, part %d: %f\n",
                      i, ((double*)opInfo->arg_values[0])[i],
                      i, ((double*)opInfo->arg_values[1])[i]);
        }
        destLocation->values[i] = mkShadowValue();
        // Set the destination shadow values to the result of a
        // high-precision shadowing operation, for each channel in which
        // it occurs.
        continue;
        mpfr_func(destLocation->values[i]->value, arg1Location->values[i]->value,
                  arg2Location->values[i]->value, MPFR_RNDN);
        // Set up the stem record of this operation.
        initBranchStemNode(destLocation->values[i], opInfo, 2,
                           arg1Location->values[i],
                           arg2Location->values[i]);
        continue;
        // Now, we'll evaluate the shadow value against the result
        // value, for each of it's channels.
        evaluateOpError_helper(destLocation->values[i],
                               argType, i,
                               opInfo);
      }
      break;
      // Copy across the rest of the values from the first argument
      for (;i < capacity(argType); ++i){
        copySV(arg1Location->values[i], &(destLocation->values[i]));
      }
    }
    break;
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
    break;
    {
      LocType type;
      // For operations like this, since it's not a math-y operation,
      // we're not going to track it unless it's already being
      // tracked.
      if (getTemp(opInfo->arg_tmps[0]) == NULL &&
          getTemp(opInfo->arg_tmps[1]) == NULL){
        destLocation = NULL;
        break;
      }
      switch(opInfo->op){
      case Iop_SetV128lo32:
        type = Lt_Floatx4;
        break;
      case Iop_SetV128lo64:
        type = Lt_Doublex2;
        break;
      default:
        return;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg1Location =
        getShadowLocation(opInfo->arg_tmps[0],
                          type);
      arg2Location =
        getShadowLocation(opInfo->arg_tmps[1],
                          type);
      // Now we'll allocate memory for the shadowed result of this
      // operation, which is a 128-bit SIMD value. The high order
      // 64-bits are taken from the first argument, while the low order
      // 64-bits are taken from the second argument.
      destLocation = mkShadowLocation_bare(type);

      // Copy the low order bits shadow value from the second argument.
      copySV(arg2Location->values[0], &(destLocation->values[0]));

      // Copy across the higher order bits shadow value from the first
      // argument.
      for (int i = 1; i < capacity(type); ++i){
        copySV(getShadowValue(arg1Location, i,
                              opInfo->arg_values[0]),
               &(destLocation->values[i]));
      }

      // This isn't really a "real" op in the math-y sense, so let's not
      // evaluate its error.
    }
    break;
  case Iop_XorV128:
    break;
    // Probably a negation
    {
      LocType argType = Lt_Float;
      if (*(opInfo->arg_values[0]) == 0x8000000000000000 ||
          *(opInfo->arg_values[1]) == 0x8000000000000000 ||
          *(opInfo->arg_values[0]) == 0x2424242424242424 ||
          *(opInfo->arg_values[1]) == 0x2424242424242424){
        // Pull the shadow values for the arguments. If we don't already
        // have shadow values for these arguments, we'll generate fresh
        // ones from the runtime float values.
        if(*(opInfo->arg_values[0]) == 0x8000000000000000){
          argType = opInfo->expected_type;
          arg1Location =
            getShadowLocation(opInfo->arg_tmps[1],
                              argType);
          arg1 = getShadowValue(arg1Location, 0,
                                opInfo->arg_values[1]);
        } else if (*(opInfo->arg_values[1]) == 0x8000000000000000){
          argType = opInfo->expected_type;
          arg1Location =
            getShadowLocation(opInfo->arg_tmps[0],
                              argType);
          arg1 = getShadowValue(arg1Location, 0,
                                opInfo->arg_values[0]);
        } else if (*(opInfo->arg_values[0]) == 0x2424242424242424){
          argType = opInfo->expected_type;
          arg1Location =
            getShadowLocation(opInfo->arg_tmps[1],
                              argType);
          arg1 = getShadowValue(arg1Location, 0,
                                opInfo->arg_values[1]);
        } else if (*(opInfo->arg_values[1]) == 0x2424242424242424){
          argType = opInfo->expected_type;
          arg1Location =
            getShadowLocation(opInfo->arg_tmps[0],
                              argType);
          arg1 = getShadowValue(arg1Location, 0,
                                opInfo->arg_values[0]);
        }

        // Now we'll allocate memory for the shadowed result of this
        // operation.
        destLocation = mkShadowLocation_bare(argType);
        destLocation->values[0] = mkShadowValue();

        // Set the destination shadow values to the result of a
        // high-precision shadowing operation.
        mpfr_neg(destLocation->values[0]->value, arg1->value,
                 MPFR_RNDN);
        // Set up the stem record of this operation.
        initBranchStemNode(destLocation->values[0], opInfo, 1,
                           arg1);
        // Now, we'll evaluate the shadow value against the result
        // value.
        evaluateOpError_helper(destLocation->values[0],
                               argType, 0,
                               opInfo);
      } else
        destLocation = NULL;
    }
    break;
  default:
    return;
  }
  // Put the resulting location in the space for the dest temp.
  if (print_moves && destLocation != NULL){
    VG_(printf)("Putting shadow location ");
    printShadowLoc(destLocation);
    VG_(printf)(" in temp %lu.\n", opInfo->dest_tmp);
  }
  setTemp(opInfo->dest_tmp, destLocation);
}
VG_REGPARM(1) void executeTernaryShadowOp(Op_Info* opInfo){
  return;
  // The shadowing locations for the arguments and the
  // destination. The rounding mode (first argument) needs no shadow
  // location, since it's an int.
  ShadowLocation *arg2Location, *arg3Location, *destLocation;
  // We're going to use a lot of switch statements here because there
  // are a lot of similar cases, and we don't want to repeat too much
  // code. Hopefully, it will be moderately well organized. In the
  // case of ternary ops, the structure is the same for all of the ops
  // we support, so we don't need the outer switch statement that some
  // of the other aritys of shadow ops have.

  // We keep track of three attributes for each of these
  // instructions: what function it performs, what register
  // size/type does it expect to work with, and how many values of
  // it's input should be processed. This last one isn't
  // inherently tied to the register type because some
  // instructions here will act on a four channel float, but only
  // perform an operation on one channel, and copy the others across.
  int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
  LocType type;
  int num_vals;
  if (!running) return;

  // Figure out the underlying math function of the
  // operation. Some are not in the mpfr library directly, but are
  // a combination of mpfr functions with the right arguments, so
  // I added wrappers in hg_hiprec_ops.c to let us do this
  // uniformly.
  switch(opInfo->op){
  case Iop_AddF32:
  case Iop_Add32Fx4:
  case Iop_Add32Fx8:
  case Iop_AddF64r32:
  case Iop_AddF64:
  case Iop_Add64Fx2:
  case Iop_Add64Fx4:
    mpfr_func = mpfr_add;
    break;
  case Iop_SubF32:
  case Iop_Sub32Fx4:
  case Iop_Sub32Fx8:
  case Iop_SubF64r32:
  case Iop_SubF64:
  case Iop_Sub64Fx2:
  case Iop_Sub64Fx4:
    mpfr_func = mpfr_sub;
    break;
  case Iop_MulF32:
  case Iop_Mul32Fx4:
  case Iop_Mul32Fx8:
  case Iop_MulF64r32:
  case Iop_MulF64:
  case Iop_Mul64Fx2:
  case Iop_Mul64Fx4:
    mpfr_func = mpfr_mul;
    break;
  case Iop_DivF32:
  case Iop_Div32Fx4:
  case Iop_Div32Fx8:
  case Iop_DivF64r32:
  case Iop_DivF64:
  case Iop_Div64Fx2:
  case Iop_Div64Fx4:
    mpfr_func = mpfr_div;
    break;
  case Iop_AtanF64:
    mpfr_func = mpfr_atan2;
    break;
  case Iop_Yl2xF64:
    mpfr_func = hiprec_yl2x;
    break;
  case Iop_Yl2xp1F64:
    mpfr_func = hiprec_yl2xp1;
    break;
  case Iop_ScaleF64:
    mpfr_func = hiprec_scale;
    break;
  default:
    return;
  }

  // Now we're going to determine the size, and how many values
  // should be operated on (the rest will be copied).
  switch(opInfo->op){
  case Iop_Add64Fx2:
  case Iop_Sub64Fx2:
  case Iop_Mul64Fx2:
  case Iop_Div64Fx2:
    type = Lt_Doublex2;
    num_vals = 2;
    break;
  case Iop_Add64Fx4:
  case Iop_Sub64Fx4:
  case Iop_Mul64Fx4:
  case Iop_Div64Fx4:
    type = Lt_Doublex4;
    num_vals = 4;
    break;
  case Iop_Add32Fx4:
  case Iop_Sub32Fx4:
  case Iop_Mul32Fx4:
  case Iop_Div32Fx4:
    type = Lt_Floatx4;
    num_vals = 4;
    break;
  case Iop_Add32Fx8:
  case Iop_Sub32Fx8:
  case Iop_Mul32Fx8:
  case Iop_Div32Fx8:
    type = Lt_Floatx8;
    num_vals = 8;
    break;
  case Iop_AtanF64:
  case Iop_Yl2xF64:
  case Iop_Yl2xp1F64:
  case Iop_ScaleF64:
  case Iop_AddF64:
  case Iop_SubF64:
  case Iop_MulF64:
  case Iop_DivF64:
  case Iop_AddF64r32:
  case Iop_SubF64r32:
  case Iop_MulF64r32:
  case Iop_DivF64r32:
    type = Lt_Double;
    num_vals = 1;
    break;
  case Iop_AddF32:
  case Iop_SubF32:
  case Iop_MulF32:
  case Iop_DivF32:
    type = Lt_Float;
    num_vals = 1;
  default:
    return;
  }

  // Pull the shadow values for the arguments. If we don't already
  // have shadow values for these arguments, we'll generate fresh
  // ones from the runtime float values.
  arg2Location =
    getShadowLocation(opInfo->arg_tmps[1],
                      type);
  arg3Location =
    getShadowLocation(opInfo->arg_tmps[2],
                      type);

  for(int i = 0; i < num_vals; ++i){
    getShadowValue(arg2Location, i,
                   opInfo->arg_values[1]);
    getShadowValue(arg3Location, i,
                   opInfo->arg_values[2]);
  }

  // Now we'll allocate memory for the shadowed result of this
  // operation.
  destLocation = mkShadowLocation(type);
  if (print_inputs){
    VG_(printf)("Shadow first arg: ");
    printShadowLoc(arg2Location);
    VG_(printf)("\nShadow second arg: ");
    printShadowLoc(arg3Location);
    VG_(printf)("\n");
  }

  for (int i = 0; i < num_vals; ++i){
    if (print_inputs){
      VG_(printf)("Computed first arg, part %d: %f\n"
                  "Computed second arg, part %d: %f\n",
                  i, ((double*)opInfo->arg_values[1])[i],
                  i, ((double*)opInfo->arg_values[2])[i]);
    }
    // Set the destination shadow value to the result of a
    // high-precision shadowing operation.
    mpfr_func(destLocation->values[i]->value, arg2Location->values[i]->value,
              arg3Location->values[i]->value,
              roundmodeIRtoMPFR(*((IRRoundingMode*)opInfo->arg_values[0])));
    // Set up the stem record of this operation.
    initBranchStemNode(destLocation->values[i], opInfo, 2,
                       arg2Location->values[i],
                       arg3Location->values[i]);
    // Now let's compare the computed value to the high precision result.
    evaluateOpError_helper(destLocation->values[i],
                           type, i,
                           opInfo);
  }

  // Put the resulting location in the space for the dest temp.
  if (print_moves && destLocation != NULL){
    VG_(printf)("Putting shadow location ");
    printShadowLoc(destLocation);
    VG_(printf)(" in temp %lu.\n", opInfo->dest_tmp);
  }
  setTemp(opInfo->dest_tmp, destLocation);
}
VG_REGPARM(1) void executeQuadnaryShadowOp(Op_Info* opInfo){
  return;
  // The shadowing locations for the arguments and the
  // destination. The rounding mode (first argument) needs no shadow
  // location, since it's an int.
  ShadowLocation *arg2Location, *arg3Location, *arg4Location, *destLocation;
  ShadowValue *arg2, *arg3, *arg4;
  // We're going to use a lot of switch statements here because there
  // are a lot of similar cases, and we don't want to repeat too much
  // code. Hopefully, it will be moderately well organized.

  int (*mpfr_func)(mpfr_t, mpfr_srcptr, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
  LocType argType;
  if (!running) return;

  // Determine the mpfr shadow function. In these cases, we don't
  // actually have a mpfr function for fused multiply add/sub, but we
  // can combine mpfr functions to get that functionality. These
  // combined functions are in hg_hiprec_ops.c.
  switch(opInfo->op){
  case Iop_MAddF32:
  case Iop_MAddF64:
  case Iop_MAddF64r32:
    mpfr_func = hiprec_fma;
    break;
  case Iop_MSubF32:
  case Iop_MSubF64:
  case Iop_MSubF64r32:
    mpfr_func = hiprec_fms;
    break;
  default:
    return;
  }

  // Determine the type/size of the arguments
  switch(opInfo->op){
  case Iop_MAddF32:
  case Iop_MSubF32:
    argType = Lt_Float;
    break;
  case Iop_MAddF64:
  case Iop_MSubF64:
  case Iop_MAddF64r32:
  case Iop_MSubF64r32:
    argType = Lt_Double;
    break;
  default:
    return;
  }

  // Pull the shadow values for the arguments. If we don't already
  // have shadow values for these arguments, we'll generate fresh
  // ones from the runtime float values.
  arg2Location =
    getShadowLocation(opInfo->arg_tmps[1],
                      argType);
  arg2 = getShadowValue(arg2Location, 0,
                        opInfo->arg_values[1]);
  
  arg3Location =
    getShadowLocation(opInfo->arg_tmps[2],
                      argType);
  arg3 = getShadowValue(arg3Location, 0,
                        opInfo->arg_values[2]);
  arg4Location =
    getShadowLocation(opInfo->arg_tmps[3],
                      argType);
  arg4 = getShadowValue(arg4Location, 0,
                        opInfo->arg_values[3]);

  // Now we'll allocate memory for the shadowed result of this
  // operation.
  destLocation = mkShadowLocation(argType);
  

  // Set the destination shadow value to the result of a
  // high-precision shadowing operation.
  mpfr_func(destLocation->values[0]->value, arg2->value,
            arg3->value, arg4->value,
            roundmodeIRtoMPFR(((IRRoundingMode*)opInfo->arg_values[0])[0]));
  // Set up the stem record of this operation.
  initBranchStemNode(destLocation->values[0], opInfo, 3,
                     arg2, arg3, arg4);

  if (print_inputs){
    VG_(printf)("Shadow first arg: ");
    printShadowLoc(arg2Location);
    VG_(printf)("\nShadow second arg: ");
    printShadowLoc(arg3Location);
    VG_(printf)("\nShadow third arg: ");
    printShadowLoc(arg4Location);
    VG_(printf)("\n");
    VG_(printf)("Computed first arg: %f\n"
                "Computed second arg: %f\n"
                "Computed third arg: %f\n",
                ((double*)opInfo->arg_values[1])[0],
                ((double*)opInfo->arg_values[2])[0],
                ((double*)opInfo->arg_values[3])[0]);
  }
  // Now, we'll evaluate the shadow value against the result value.
  evaluateOpError_helper(destLocation->values[0],
                         argType, 0,
                         opInfo);

  // Put the resulting location in the space for the dest temp.
  if (print_moves && destLocation != NULL){
    VG_(printf)("Putting shadow location ");
    printShadowLoc(destLocation);
    VG_(printf)(" in temp %lu.\n", opInfo->dest_tmp);
  }
  setTemp(opInfo->dest_tmp, destLocation);
}

mpfr_rnd_t roundmodeIRtoMPFR(IRRoundingMode round){
  switch (round){
  case Irrm_PREPARE_SHORTER:
  case Irrm_NEAREST:
  case Irrm_NEAREST_TIE_AWAY_0:
  case Irrm_NEAREST_TIE_TOWARD_0:
    return MPFR_RNDN;
  case Irrm_NegINF:
    return MPFR_RNDD;
  case Irrm_PosINF:
    return MPFR_RNDU;
  case Irrm_ZERO:
    return MPFR_RNDZ;
  case Irrm_AWAY_FROM_ZERO:
    return MPFR_RNDA;
  default:
    return MPFR_RNDN;
  }
}

// Get's the current shadow location in the given temporary, or makes
// a new bare one with the given type if it doesn't exist.
ShadowLocation* getShadowLocation(UWord tmp_num, LocType type){
  // If we already have a shadow location here, just return it.
  ShadowLocation* location = getTemp(tmp_num);
  if (location != NULL && location->type == type) return location;

  if (print_moves)
    VG_(printf)("Creating new shadow location in temp %lu\n", tmp_num);

  // Otherwise we need to create a new one. How we do this will depend
  // on the expected type of the location, passed as "type".
  location = mkShadowLocation_bare(type);

  // Put the new shadow location in the temporary where we expected to
  // find it.
  setTemp(tmp_num, location);
  return location;
}

// Get's the current shadow value at the given index in the given
// location, or creates a new one initialized to the appropriate value
// in loc_bytes with src_loc as it's leaf node op src.
ShadowValue* getShadowValue(ShadowLocation* loc, UWord index,
                            UWord* loc_bytes){
  // if (loc->values[index] != NULL) return loc->values[index];
  // Create a new shadow value, and give it a leaf node stem.
  loc->values[index] = mkShadowValue();
  // Add a reference since by adding this to a shadow location, we're
  // putting it in a temp.
  addRef(loc->values[index]);

  // Initialize it's MPFR value with the current value of its float
  // bytes.
  tl_assert2(index < capacity(loc->type),
             "Invalid index %lu for location of type %d!\n",
             index, loc->type);
  switch(loc->type){
  case Lt_Doublex4:
  case Lt_Doublex2:
  case Lt_Double:
    mpfr_set_d(loc->values[index]->value, ((double*)loc_bytes)[index],
               MPFR_RNDN);
    break;
  case Lt_Floatx8:
  case Lt_Floatx4:
  case Lt_Floatx2:
  case Lt_Float:
    mpfr_set_d(loc->values[index]->value, ((float*)loc_bytes)[index],
               MPFR_RNDN);
    break;
  default:
    VG_(printf)("Don't handle that type of shadow location (%d)!\n", loc->type);
    return NULL;
  }
  // Initialize the stem node for it.
  initLeafStemNode(loc->values[index]);
  return loc->values[index];
}
