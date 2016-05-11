
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie      hg_instrumentOp.c ---*/
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

#include "hg_instrument.h"
#include "include/hg_macros.h"

// Add instrumenting expressions to sb for an operation, storing the
// result in the temporary at offset.
void instrumentOp(IRSB* sb, Int offset, IRExpr* expr, Addr opAddr){
  IRDirty* executeShadowOp;
  SizeT arg_size, result_size;
  (void)executeShadowOp;
  (void)arg_size;
  (void)result_size;

  // So, I recently learned that valgrind doesn't like passing more
  // than three arguments to a c function called by client code. I
  // couldn't find any documentation on why this might be, or even any
  // concrete documentation that this was the case, but it looks like
  // an assertion fails when trying to make the call in
  // unsafeIRDirty_0_N if you try to use a function with more than
  // three arguments. For this reason, instead of passing each of the
  // arguments to the operations as separate arguments to the client
  // called function, I'm going to try passing them as a malloc'd
  // array.
  switch (expr->tag){
  /* case Iex_Unop: */
  /*   { */
  /*     Op_Info* opInfo; */
  /*     // Determine the argument sizes of each operation we might */
  /*     // encounter so we can allocate the right amount of space in the */
  /*     // argument structure to the runtime shadow execution code. */
  /*     switch(expr->Iex.Unop.op){ */
  /*     case Iop_V128to64: */
  /*     case Iop_V128HIto64: */
  /*     case Iop_F128HItoF64: */
  /*     case Iop_F128LOtoF64: */
  /*       arg_size = sizeof(double) * 2; */
  /*       result_size = sizeof(double); */
  /*       break; */
  /*     case Iop_64UtoV128: */
  /*     case Iop_SetV128lo64: */
  /*       arg_size = sizeof(double); */
  /*       result_size = sizeof(double)*2; */
  /*     case Iop_V128to32: */
  /*       arg_size = sizeof(float) * 4; */
  /*       result_size = sizeof(float); */
  /*     case Iop_F32toF64: */
  /*       arg_size = sizeof(float); */
  /*       result_size = sizeof(double); */
  /*       break; */
  /*     case Iop_TruncF64asF32: */
  /*       arg_size = sizeof(double); */
  /*       result_size = sizeof(float); */
  /*       break; */
  /*     case Iop_NegF32: */
  /*     case Iop_AbsF32: */
  /*       arg_size = sizeof(float); */
  /*       result_size = sizeof(float); */
  /*       break; */
  /*     case Iop_RSqrtEst32Fx2: */
  /*     case Iop_RecipEst32Fx2: */
  /*     case Iop_RoundF64toF32: */
  /*     case Iop_RSqrtEst5GoodF64: */
  /*     case Iop_NegF64: */
  /*     case Iop_AbsF64: */
  /*     case Iop_RoundF64toF64_NEAREST: */
  /*     case Iop_RoundF64toF64_NegINF: */
  /*     case Iop_RoundF64toF64_PosINF: */
  /*     case Iop_RoundF64toF64_ZERO: */
  /*       arg_size = sizeof(double); */
  /*       result_size = sizeof(double); */
  /*       break; */
  /*     case Iop_RecipEst32Fx4: */
  /*     case Iop_RSqrtEst32Fx4: */
  /*     case Iop_RecipEst32F0x4: */
  /*     case Iop_RSqrtEst32F0x4: */
  /*     case Iop_Sqrt32F0x4: */
  /*     case Iop_Abs32Fx4: */
  /*     case Iop_Neg32Fx4: */
  /*     case Iop_ZeroHI96ofV128: */
  /*       arg_size = sizeof(float)*4; */
  /*       result_size = sizeof(float)*4; */
  /*       break; */
  /*     case Iop_RSqrtEst32Fx8: */
  /*     case Iop_RecipEst32Fx8: */
  /*       arg_size = sizeof(float) * 8; */
  /*       result_size = sizeof(float) * 8; */
  /*       break; */
  /*     case Iop_ZeroHI64ofV128: */
  /*     case Iop_RecipEst64Fx2: */
  /*     case Iop_RSqrtEst64Fx2: */
  /*     case Iop_Abs64Fx2: */
  /*     case Iop_Neg64Fx2: */
  /*     case Iop_Sqrt64F0x2: */
  /*       arg_size = sizeof(double) * 2; */
  /*       result_size = sizeof(double) * 2; */
  /*       break; */
  /*     default: */
  /*       return; */
  /*     } */

  /*     // Only bother instrumenting operations that we care about and */
  /*     // can do something useful with. */
  /*     switch (expr->Iex.Unop.op){ */
  /*       // Add all supported unary ops to this list. */
  /*     case Iop_RecipEst32Fx4: */
  /*     case Iop_RSqrtEst32Fx4: */
  /*     case Iop_Abs32Fx4: */
  /*     case Iop_Neg32Fx4: */
  /*     case Iop_ZeroHI96ofV128: */
  /*     case Iop_ZeroHI64ofV128: */
  /*     case Iop_V128to32: */
  /*     case Iop_V128to64: */
  /*     case Iop_V128HIto64: */
  /*     case Iop_64UtoV128: */
  /*     case Iop_SetV128lo64: */
  /*     case Iop_RecipEst64Fx2: */
  /*     case Iop_RSqrtEst64Fx2: */
  /*     case Iop_Abs64Fx2: */
  /*     case Iop_Neg64Fx2: */
  /*     case Iop_RecipEst32F0x4: */
  /*     case Iop_Sqrt32F0x4: */
  /*     case Iop_RSqrtEst32F0x4: */
  /*     case Iop_RSqrtEst32Fx2: */
  /*     case Iop_RecipEst32Fx2: */
  /*     case Iop_RoundF64toF32: */
  /*     case Iop_TruncF64asF32: */
  /*     case Iop_RSqrtEst5GoodF64: */
  /*     case Iop_RoundF64toF64_NEAREST: */
  /*     case Iop_RoundF64toF64_NegINF: */
  /*     case Iop_RoundF64toF64_PosINF: */
  /*     case Iop_RoundF64toF64_ZERO: */
  /*     case Iop_F128HItoF64: */
  /*     case Iop_F128LOtoF64: */
  /*     case Iop_F32toF64: */
  /*     case Iop_NegF32: */
  /*     case Iop_AbsF32: */
  /*     case Iop_NegF64: */
  /*     case Iop_AbsF64: */
  /*     case Iop_Sqrt64F0x2: */
  /*       // Allocate and partially setup the argument structure */
  /*       opInfo = mkOp_Info(1, expr->Iex.Unop.op, */
  /*                          opAddr, */
  /*                          getPlainOpname(expr->Iex.Unop.op), */
  /*                          getOpSymbol(expr->Iex.Unop.op)); */

  /*       // Populate the argument/result values we know at instrument time now. */
  /*       opInfo->args.uargs.arg_tmp = getArgTmp(expr->Iex.Unop.arg, sb); */
  /*       opInfo->dest_tmp = offset; */

  /*       // Allocate the space for the values we won't know until */
  /*       // runtime, but know their size now. */
  /*       ALLOC(opInfo->args.uargs.arg_value, */
  /*             "hg.arg_alloc", 1, arg_size); */
  /*       ALLOC(opInfo->dest_value, */
  /*             "hg.arg_alloc", 1, result_size); */

  /*       // Add statements to populate the values we don't know until */
  /*       // runtime. */
  /*       addStore(sb, expr->Iex.Unop.arg, */
  /*                opInfo->args.uargs.arg_value); */
  /*       addStore(sb, IRExpr_RdTmp(offset), */
  /*                opInfo->dest_value); */

  /*       // Finally, add the statement to call the shadow op procedure. */
  /*       executeShadowOp = */
  /*         unsafeIRDirty_0_N(1, */
  /*                           "executeUnaryShadowOp", */
  /*                           VG_(fnptr_to_fnentry)(&executeUnaryShadowOp), */
  /*                           mkIRExprVec_1(mkU64((uintptr_t)opInfo))); */
  /*       addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp)); */
  /*       break; */
  /*     default: */
  /*       break; */
  /*     } */
  /*   } */
  /*   break; */
  case Iex_Binop:
    {
      Op_Info* opInfo;
      // Determine the argument sizes of each operation we might
      // encounter so we can allocate the right amount of space in the
      // argument structure to the runtime shadow execution code.
      switch (expr->Iex.Binop.op){
      case Iop_64HLtoV128:
      case Iop_F64HLtoF128:
        arg_size = sizeof(double);
        result_size = sizeof(double)*2;
        break;
      case Iop_F64toF32:
        arg_size = sizeof(double);
        result_size = sizeof(float);
        break;
      case Iop_RecpExpF64:
      case Iop_RoundF64toInt:
      case Iop_SinF64:
      case Iop_CosF64:
      case Iop_TanF64:
      case Iop_2xm1F64:
      case Iop_SqrtF64:
        arg_size = sizeof(double);
        result_size = sizeof(double);
        break;
      case Iop_QAdd32S:
      case Iop_QSub32S:
      case Iop_RecpExpF32:
      case Iop_RoundF32toInt:
      case Iop_SqrtF32:
        arg_size = sizeof(float);
        result_size = sizeof(float);
        break;
      case Iop_RecipStep32Fx2:
      case Iop_RSqrtStep32Fx2:
      case Iop_Neg32Fx2:
      case Iop_Abs32Fx2:
      case Iop_Add32Fx2:
      case Iop_Sub32Fx2:
        arg_size = sizeof(float) * 2;
        result_size = sizeof(float) * 2;
        break;
      case Iop_RecipStep32Fx4:
      case Iop_RSqrtStep32Fx4:
      case Iop_Sqrt32Fx4:
      case Iop_SetV128lo32:
        arg_size = sizeof(float) * 4;
        result_size = sizeof(float) * 4;
        break;
      case Iop_Sqrt32Fx8:
        arg_size = sizeof(float) * 8;
        result_size = sizeof(float) * 8;
        break;
      case Iop_RecipStep64Fx2:
      case Iop_RSqrtStep64Fx2:
      case Iop_Sqrt64Fx2:
      case Iop_Add64F0x2:
      case Iop_Sub64F0x2:
      case Iop_Mul64F0x2:
      case Iop_Div64F0x2:
      case Iop_SetV128lo64:
        arg_size = sizeof(double) * 2;
        result_size = sizeof(double) * 2;
        break;
      case Iop_Sqrt64Fx4:
      case Iop_XorV128:
        arg_size = sizeof(double) * 4;
        result_size = sizeof(double) * 4;
        break;
      default:
        return;
      }

      // Only bother instrumenting operations that we care about and
      // can do something useful with.
      switch (expr->Iex.Binop.op){
        // Add all supported binary ops to this list
      case Iop_RecipStep32Fx4:
      case Iop_RSqrtStep32Fx4:
      case Iop_Add32Fx2:
      case Iop_Sub32Fx2:
      case Iop_Sqrt64Fx2:
      case Iop_Add32F0x4:
      case Iop_Sub32F0x4:
      case Iop_Mul32F0x4:
      case Iop_Div32F0x4:
      case Iop_RecipStep32Fx2:
      case Iop_RSqrtStep32Fx2:
      case Iop_RecipStep64Fx2:
      case Iop_RSqrtStep64Fx2:
      case Iop_Neg32Fx2:
      case Iop_Abs32Fx2:
      case Iop_RecpExpF64:
      case Iop_RecpExpF32:
      case Iop_RoundF64toInt:
      case Iop_RoundF32toInt:
      case Iop_64HLtoV128:
      case Iop_F64HLtoF128:
      case Iop_F64toF32:
      case Iop_SinF64:
      case Iop_CosF64:
      case Iop_TanF64:
      case Iop_2xm1F64:
      case Iop_SqrtF64:
      case Iop_SqrtF32:
      case Iop_Add64F0x2:
      case Iop_Sub64F0x2:
      case Iop_Mul64F0x2:
      case Iop_Div64F0x2:
      case Iop_SetV128lo32:
      case Iop_SetV128lo64:
      case Iop_XorV128:
        // Allocate and partially setup the argument structure
        opInfo = mkOp_Info(2, expr->Iex.Binop.op,
                           opAddr,
                           getPlainOpname(expr->Iex.Binop.op),
                           getOpSymbol(expr->Iex.Binop.op));

        // Populate the argument/result values we know at instrument time now.
        opInfo->args.bargs.arg1_tmp =
          getArgTmp(expr->Iex.Binop.arg1, sb);
        opInfo->args.bargs.arg2_tmp =
          getArgTmp(expr->Iex.Binop.arg2, sb);
        opInfo->dest_tmp = offset;

        // Allocate the space for the values we won't know until
        // runtime, but know their size now.
        ALLOC(opInfo->args.bargs.arg1_value,
              "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->args.bargs.arg2_value,
              "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->dest_value,
              "hg.arg_alloc", 1, result_size);

        // Add statements to populate the values we don't know until
        // runtime.
        addStore(sb, expr->Iex.Binop.arg1,
                 opInfo->args.bargs.arg1_value);
        addStore(sb, expr->Iex.Binop.arg2,
                 opInfo->args.bargs.arg2_value);
        addStore(sb, IRExpr_RdTmp(offset),
                 opInfo->dest_value);

        // Finally, add the statement to call the shadow op procedure.
        executeShadowOp =
          unsafeIRDirty_0_N(1,
                            "executeBinaryShadowOp",
                            VG_(fnptr_to_fnentry)(&executeBinaryShadowOp),
                            mkIRExprVec_1(mkU64((uintptr_t)opInfo)));
        addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp));
        break;
      default:
        break;
      }
    }
    break;
  /* case Iex_Triop: */
  /*   { */
  /*     Op_Info* opInfo; */

  /*     // Determine the argument sizes of each operation we might */
  /*     // encounter so we can allocate the right amount of space in the */
  /*     // argument structure to the runtime shadow execution code. */
  /*     switch (expr->Iex.Triop.details->op){ */
  /*     case Iop_AtanF64: */
  /*     case Iop_Yl2xF64: */
  /*     case Iop_Yl2xp1F64: */
  /*     case Iop_ScaleF64: */
  /*     case Iop_AddF64: */
  /*     case Iop_SubF64: */
  /*     case Iop_MulF64: */
  /*     case Iop_DivF64: */
  /*     case Iop_AddF64r32: */
  /*     case Iop_SubF64r32: */
  /*     case Iop_MulF64r32: */
  /*     case Iop_DivF64r32: */
  /*       arg_size = sizeof(double); */
  /*       result_size = sizeof(double); */
  /*       break; */
  /*     case Iop_AddF32: */
  /*     case Iop_SubF32: */
  /*     case Iop_MulF32: */
  /*     case Iop_DivF32: */
  /*       arg_size = sizeof(float); */
  /*       result_size = sizeof(float); */
  /*       break; */
  /*     case Iop_Add32Fx4: */
  /*     case Iop_Sub32Fx4: */
  /*     case Iop_Mul32Fx4: */
  /*     case Iop_Div32Fx4: */
  /*       arg_size = sizeof(float) * 4; */
  /*       result_size = sizeof(float) * 4; */
  /*       break; */
  /*     case Iop_Add64Fx2: */
  /*     case Iop_Sub64Fx2: */
  /*     case Iop_Mul64Fx2: */
  /*     case Iop_Div64Fx2: */
  /*       arg_size = sizeof(double) * 2; */
  /*       result_size = sizeof(double) * 2; */
  /*       break; */
  /*     case Iop_Add32Fx8: */
  /*     case Iop_Sub32Fx8: */
  /*     case Iop_Mul32Fx8: */
  /*     case Iop_Div32Fx8: */
  /*       arg_size = sizeof(float) * 8; */
  /*       result_size = sizeof(float) * 8; */
  /*     case Iop_Add64Fx4: */
  /*     case Iop_Sub64Fx4: */
  /*     case Iop_Mul64Fx4: */
  /*     case Iop_Div64Fx4: */
  /*       arg_size = sizeof(double) * 4; */
  /*       result_size = sizeof(double) * 4; */
  /*       break; */
  /*     default: */
  /*       return; */
  /*     } */

  /*     // Only bother instrumenting operations that we care about and */
  /*     // can do something useful with. */
  /*     switch (expr->Iex.Triop.details->op){ */
  /*       // Add all supported ternary ops to this list */
  /*     case Iop_Add32Fx8: */
  /*     case Iop_Sub32Fx8: */
  /*     case Iop_Mul32Fx8: */
  /*     case Iop_Div32Fx8: */
  /*     case Iop_Add64Fx4: */
  /*     case Iop_Sub64Fx4: */
  /*     case Iop_Mul64Fx4: */
  /*     case Iop_Div64Fx4: */
  /*     case Iop_Add32Fx4: */
  /*     case Iop_Sub32Fx4: */
  /*     case Iop_Mul32Fx4: */
  /*     case Iop_Div32Fx4: */
  /*     case Iop_Add64Fx2: */
  /*     case Iop_Sub64Fx2: */
  /*     case Iop_Mul64Fx2: */
  /*     case Iop_Div64Fx2: */
  /*     case Iop_AtanF64: */
  /*     case Iop_Yl2xF64: */
  /*     case Iop_Yl2xp1F64: */
  /*     case Iop_ScaleF64: */
  /*     case Iop_AddF128: */
  /*     case Iop_SubF128: */
  /*     case Iop_MulF128: */
  /*     case Iop_DivF128: */
  /*     case Iop_AddF64: */
  /*     case Iop_SubF64: */
  /*     case Iop_MulF64: */
  /*     case Iop_DivF64: */
  /*     case Iop_AddF32: */
  /*     case Iop_SubF32: */
  /*     case Iop_MulF32: */
  /*     case Iop_DivF32: */
  /*     case Iop_AddF64r32: */
  /*     case Iop_SubF64r32: */
  /*     case Iop_MulF64r32: */
  /*     case Iop_DivF64r32: */
  /*       // Allocate and partially setup the argument structure */
  /*       opInfo = mkOp_Info(3, expr->Iex.Triop.details->op, opAddr, */
  /*                          getPlainOpname(expr->Iex.Triop.details->op), */
  /*                          getOpSymbol(expr->Iex.Triop.details->op)); */

  /*       // Populate the values we know at instrument time now. */
  /*       opInfo->args.targs.arg1_tmp = */
  /*         getArgTmp(expr->Iex.Triop.details->arg1, sb); */
  /*       opInfo->args.targs.arg2_tmp = */
  /*         getArgTmp(expr->Iex.Triop.details->arg2, sb); */
  /*       opInfo->args.targs.arg3_tmp = */
  /*         getArgTmp(expr->Iex.Triop.details->arg3, sb); */
  /*       opInfo->dest_tmp = offset; */

  /*       // Allocate the space for the values we won't know until */
  /*       // runtime, but know their size now. */
  /*       ALLOC(opInfo->args.targs.arg1_value, */
  /*             "hg.arg_alloc", 1, arg_size); */
  /*       ALLOC(opInfo->args.targs.arg2_value, */
  /*             "hg.arg_alloc", 1, arg_size); */
  /*       ALLOC(opInfo->args.targs.arg3_value, */
  /*             "hg.arg_alloc", 1, arg_size); */
  /*       ALLOC(opInfo->dest_value, */
  /*             "hg.arg_alloc", 1, result_size); */

  /*       // Add statements to populate the values we don't know until */
  /*       // runtime. */
  /*       addStore(sb, expr->Iex.Triop.details->arg1, */
  /*                opInfo->args.targs.arg1_value); */
  /*       addStore(sb, expr->Iex.Triop.details->arg2, */
  /*                opInfo->args.targs.arg2_value); */
  /*       addStore(sb, expr->Iex.Triop.details->arg3, */
  /*                opInfo->args.targs.arg3_value); */
  /*       addStore(sb, IRExpr_RdTmp(offset), */
  /*                opInfo->dest_value); */

  /*       // Finally, add the statement to call the shadow op procedure. */
  /*       executeShadowOp = */
  /*         unsafeIRDirty_0_N(1, */
  /*                           "executeTernaryShadowOp", */
  /*                           VG_(fnptr_to_fnentry)(&executeTernaryShadowOp), */
  /*                           mkIRExprVec_1(mkU64((uintptr_t)opInfo))); */
  /*       addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp)); */
  /*       break; */
  /*     default: */
  /*       break; */
  /*     } */
  /*   } */
  /*   break; */
  /* case Iex_Qop: */
  /*   { */
  /*     Op_Info* opInfo; */

  /*     // Determine the argument sizes of each operation we might */
  /*     // encounter so we can allocate the right amount of space in the */
  /*     // argument structure to the runtime shadow execution code. */
  /*     switch (expr->Iex.Qop.details->op){ */
  /*     case Iop_MAddF32: */
  /*     case Iop_MSubF32: */
  /*       arg_size = sizeof(float); */
  /*       result_size = sizeof(float); */
  /*       break; */
  /*     case Iop_MAddF64: */
  /*     case Iop_MSubF64: */
  /*     case Iop_MAddF64r32: */
  /*     case Iop_MSubF64r32: */
  /*       arg_size = sizeof(double); */
  /*       result_size = sizeof(double); */
  /*       break; */
  /*     default: */
  /*       return; */
  /*     } */

  /*     // Only bother instrumenting operations that we care about and */
  /*     // can do something useful with. */
  /*     switch (expr->Iex.Qop.details->op){ */
  /*       // Add all supported quadnary ops to this list */
  /*     case Iop_MAddF32: */
  /*     case Iop_MSubF32: */
  /*     case Iop_MAddF64: */
  /*     case Iop_MSubF64: */
  /*     case Iop_MAddF64r32: */
  /*     case Iop_MSubF64r32: */
  /*       // Allocate and partially setup the argument structure */
  /*       opInfo = mkOp_Info(4, expr->Iex.Qop.details->op, opAddr, */
  /*                          getPlainOpname(expr->Iex.Qop.details->op), */
  /*                          getOpSymbol(expr->Iex.Qop.details->op)); */

  /*       // Populate the values we know at instrument time now. */
  /*       opInfo->op = expr->Iex.Qop.details->op; */
  /*       opInfo->args.qargs.arg1_tmp = */
  /*         getArgTmp(expr->Iex.Qop.details->arg1, sb); */
  /*       opInfo->args.qargs.arg2_tmp = */
  /*         getArgTmp(expr->Iex.Qop.details->arg2, sb); */
  /*       opInfo->args.qargs.arg3_tmp = */
  /*         getArgTmp(expr->Iex.Qop.details->arg3, sb); */
  /*       opInfo->args.qargs.arg4_tmp = */
  /*         getArgTmp(expr->Iex.Qop.details->arg4, sb); */
  /*       opInfo->dest_tmp = offset; */

  /*       // Allocate the space for the values we won't know until */
  /*       // runtime, but know their size now. */
  /*       ALLOC(opInfo->args.qargs.arg1_value, */
  /*             "hg.arg_alloc", 1, arg_size); */
  /*       ALLOC(opInfo->args.qargs.arg2_value, */
  /*             "hg.arg_alloc", 1, arg_size); */
  /*       ALLOC(opInfo->args.qargs.arg3_value, */
  /*             "hg.arg_alloc", 1, arg_size); */
  /*       ALLOC(opInfo->args.qargs.arg4_value, */
  /*             "hg.arg_alloc", 1, arg_size); */
  /*       ALLOC(opInfo->dest_value, */
  /*             "hg.arg_alloc", 1, result_size); */

  /*       // Add statements to populate the values we don't know until */
  /*       // runtime. */
  /*       addStore(sb, expr->Iex.Qop.details->arg1, */
  /*                opInfo->args.qargs.arg1_value); */
  /*       addStore(sb, expr->Iex.Qop.details->arg2, */
  /*                opInfo->args.qargs.arg2_value); */
  /*       addStore(sb, expr->Iex.Qop.details->arg3, */
  /*                opInfo->args.qargs.arg3_value); */
  /*       addStore(sb, expr->Iex.Qop.details->arg3, */
  /*                opInfo->args.qargs.arg4_value); */
  /*       addStore(sb, IRExpr_RdTmp(offset), */
  /*                opInfo->dest_value); */

  /*       // Finally, add the statement to call the shadow op procedure. */
  /*       executeShadowOp = */
  /*         unsafeIRDirty_0_N(1, */
  /*                           "executeQuadnaryShadowOp", */
  /*                           VG_(fnptr_to_fnentry)(&executeQuadnaryShadowOp), */
  /*                           mkIRExprVec_1(mkU64((uintptr_t)opInfo))); */
  /*       addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp)); */
  /*       break; */
  /*     default: */
  /*       break; */
  /*     } */
  /*     break; */
  /*   } */
  default:
    /* VG_(dmsg)("BAD THINGS!!!!\n"); */
    break;
  }
}

// WARNING: This potentially mutates sbOut, don't reorder. Or do, it
// might be fine.
IRTemp getArgTmp(IRExpr* arg, IRSB* sbOut){
  // If the arg is already a temp, just return the temp number
  if (arg->tag == Iex_RdTmp) return arg->Iex.RdTmp.tmp;
  // Otherwise, put it in a temp and then return the temp number of that.
  IRTemp argTemp = newIRTemp(sbOut->tyenv, typeOfIRExpr(sbOut->tyenv, arg));
  addStmtToIRSB(sbOut, IRStmt_WrTmp(argTemp, arg));
  return argTemp;
}

const HChar* getPlainOpname(IROp op){
  switch (op){
  case Iop_RecipEst32Fx4:
  case Iop_RecipEst64Fx2:
  case Iop_RecipEst32F0x4:
  case Iop_RecipEst32Fx2:
    return "reciprical estimate";
  case Iop_RecipStep32Fx4:
  case Iop_RecipStep32Fx2:
  case Iop_RecipStep64Fx2:
    return "reciprical step";
  case Iop_RSqrtEst32Fx4:
  case Iop_RSqrtEst64Fx2:
  case Iop_RSqrtEst32F0x4:
  case Iop_RSqrtEst32Fx2:
  case Iop_RSqrtEst5GoodF64:
    return "reciprical square root estimate";
  case Iop_RSqrtStep32Fx4:
  case Iop_RSqrtStep32Fx2:
  case Iop_RSqrtStep64Fx2:
    return "reciprical square root step";
  case Iop_Abs32Fx4:
  case Iop_Abs64Fx2:
  case Iop_AbsF32:
  case Iop_AbsF64:
  case Iop_Abs32Fx2:
    return "absolute value";
  case Iop_Neg32Fx4:
  case Iop_Neg64Fx2:
  case Iop_NegF32:
  case Iop_NegF64:
  case Iop_Neg32Fx2:
  case Iop_XorV128:
    return "negation";
  case Iop_Sqrt32F0x4:
  case Iop_Sqrt64F0x2:
  case Iop_Sqrt64Fx2:
  case Iop_SqrtF64:
  case Iop_SqrtF32:
    return "square root";
  case Iop_Add32Fx2:
  case Iop_Add32F0x4:
  case Iop_Add64F0x2:
  case Iop_Add32Fx8:
  case Iop_Add64Fx4:
  case Iop_Add32Fx4:
  case Iop_Add64Fx2:
  case Iop_AddF128:
  case Iop_AddF64:
  case Iop_AddF32:
  case Iop_AddF64r32:
    return "addition";
  case Iop_Sub32Fx2:
  case Iop_Sub32F0x4:
  case Iop_Sub64F0x2:
  case Iop_Sub32Fx8:
  case Iop_Sub64Fx4:
  case Iop_Sub32Fx4:
  case Iop_Sub64Fx2:
  case Iop_SubF128:
  case Iop_SubF64:
  case Iop_SubF32:
  case Iop_SubF64r32:
    return "subtraction";
  case Iop_Mul32F0x4:
  case Iop_Mul64F0x2:
  case Iop_Mul32Fx8:
  case Iop_Mul64Fx4:
  case Iop_Mul32Fx4:
  case Iop_Mul64Fx2:
  case Iop_MulF128:
  case Iop_MulF64:
  case Iop_MulF32:
  case Iop_MulF64r32:
    return "multiplication";
  case Iop_Div32F0x4:
  case Iop_Div64F0x2:
  case Iop_Div32Fx8:
  case Iop_Div64Fx4:
  case Iop_Div32Fx4:
  case Iop_Div64Fx2:
  case Iop_DivF128:
  case Iop_DivF64:
  case Iop_DivF32:
  case Iop_DivF64r32:
    return "division";
  case Iop_SinF64:
    return "sine";
  case Iop_CosF64:
    return "cosine";
  case Iop_TanF64:
    return "tangent";
  case Iop_AtanF64:
    return "arctangent";
  case Iop_MAddF32:
  case Iop_MAddF64:
  case Iop_MAddF64r32:
    return "fused multiply-add";
  case Iop_MSubF32:
  case Iop_MSubF64:
  case Iop_MSubF64r32:
    return "fused multiply-subtract";
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_F32toF64:
  case Iop_RecpExpF64:
  case Iop_RecpExpF32:
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
  case Iop_2xm1F64:
  case Iop_Yl2xF64:
  case Iop_Yl2xp1F64:
  case Iop_ScaleF64:
  default:
    return "";

  }
}
const HChar* getOpSymbol(IROp op){
  switch (op){
  case Iop_Abs32Fx4:
  case Iop_Abs64Fx2:
  case Iop_AbsF32:
  case Iop_AbsF64:
  case Iop_Abs32Fx2:
    return "abs";
  case Iop_Neg32Fx4:
  case Iop_Neg64Fx2:
  case Iop_NegF32:
  case Iop_NegF64:
  case Iop_Neg32Fx2:
  case Iop_XorV128:
    return "-";
  case Iop_Sqrt32F0x4:
  case Iop_Sqrt64F0x2:
  case Iop_Sqrt64Fx2:
  case Iop_SqrtF64:
  case Iop_SqrtF32:
    return "sqrt";
  case Iop_Add32Fx2:
  case Iop_Add32F0x4:
  case Iop_Add64F0x2:
  case Iop_Add32Fx8:
  case Iop_Add64Fx4:
  case Iop_Add32Fx4:
  case Iop_Add64Fx2:
  case Iop_AddF128:
  case Iop_AddF64:
  case Iop_AddF32:
  case Iop_AddF64r32:
    return "+";
  case Iop_Sub32Fx2:
  case Iop_Sub32F0x4:
  case Iop_Sub64F0x2:
  case Iop_Sub32Fx8:
  case Iop_Sub64Fx4:
  case Iop_Sub32Fx4:
  case Iop_Sub64Fx2:
  case Iop_SubF128:
  case Iop_SubF64:
  case Iop_SubF32:
  case Iop_SubF64r32:
    return "-";
  case Iop_Mul32F0x4:
  case Iop_Mul64F0x2:
  case Iop_Mul32Fx8:
  case Iop_Mul64Fx4:
  case Iop_Mul32Fx4:
  case Iop_Mul64Fx2:
  case Iop_MulF128:
  case Iop_MulF64:
  case Iop_MulF32:
  case Iop_MulF64r32:
    return "*";
  case Iop_Div32F0x4:
  case Iop_Div64F0x2:
  case Iop_Div32Fx8:
  case Iop_Div64Fx4:
  case Iop_Div32Fx4:
  case Iop_Div64Fx2:
  case Iop_DivF128:
  case Iop_DivF64:
  case Iop_DivF32:
  case Iop_DivF64r32:
    return "/";
  case Iop_SinF64:
    return "sin";
  case Iop_CosF64:
    return "cos";
  case Iop_TanF64:
    return "tan";
  case Iop_AtanF64:
    return "atan";
  case Iop_MAddF32:
  case Iop_MAddF64:
  case Iop_MAddF64r32:
    return "flfma";
  case Iop_MSubF32:
  case Iop_MSubF64:
  case Iop_MSubF64r32:
    return "flfms";
  case Iop_ZeroHI96ofV128:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to32:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_F32toF64:
  case Iop_RecpExpF64:
  case Iop_RecpExpF32:
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F64toF32:
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
  case Iop_2xm1F64:
  case Iop_Yl2xF64:
  case Iop_Yl2xp1F64:
  case Iop_ScaleF64:
  case Iop_RecipEst32Fx4:
  case Iop_RecipEst64Fx2:
  case Iop_RecipEst32F0x4:
  case Iop_RecipEst32Fx2:
  case Iop_RecipStep32Fx4:
  case Iop_RecipStep32Fx2:
  case Iop_RecipStep64Fx2:
  case Iop_RSqrtEst32Fx4:
  case Iop_RSqrtEst64Fx2:
  case Iop_RSqrtEst32F0x4:
  case Iop_RSqrtEst32Fx2:
  case Iop_RSqrtEst5GoodF64:
  case Iop_RSqrtStep32Fx4:
  case Iop_RSqrtStep32Fx2:
  case Iop_RSqrtStep64Fx2:
  default:
    return "";
  }
}
