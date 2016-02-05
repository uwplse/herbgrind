#include "hg_instrument.h"
#include "hg_macros.h"
#include "pub_tool_debuginfo.h"

// Add instrumenting expressions to sb for an operation, storing the
// result in the temporary at offset.
void instrumentOp(IRSB* sb, Int offset, IRExpr* expr, Addr opAddr){
  IRDirty* executeShadowOp;
  size_t arg_size, result_size;

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
  case Iex_Unop:
    {
      UnaryOp_Info* opInfo;
      // Determine the argument sizes of each operation we might
      // encounter so we can allocate the right amount of space in the
      // argument structure to the runtime shadow execution code.
      switch(expr->Iex.Unop.op){
      case Iop_V128to64:
      case Iop_V128HIto64:
      case Iop_F128HItoF64:
      case Iop_F128LOtoF64:
        arg_size = sizeof(double) * 2;
        result_size = sizeof(double);
        break;
      case Iop_V128to32:
        arg_size = sizeof(float) * 4;
        result_size = sizeof(float);
      case Iop_F32toF64:
        arg_size = sizeof(float);
        result_size = sizeof(double);
        break;
      case Iop_TruncF64asF32:
        arg_size = sizeof(double);
        result_size = sizeof(float);
        break;
      case Iop_NegF32:
      case Iop_AbsF32:
        arg_size = sizeof(float);
        result_size = sizeof(float);
        break;
      case Iop_RSqrtEst32Fx2:
      case Iop_RecipEst32Fx2:
      case Iop_RoundF64toF32:
      case Iop_RSqrtEst5GoodF64:
      case Iop_NegF64:
      case Iop_AbsF64:
      case Iop_RoundF64toF64_NEAREST:
      case Iop_RoundF64toF64_NegINF:
      case Iop_RoundF64toF64_PosINF:
      case Iop_RoundF64toF64_ZERO:
        arg_size = sizeof(double);
        result_size = sizeof(double);
        break;
      case Iop_RecipEst32Fx4:
      case Iop_RSqrtEst32Fx4:
      case Iop_RecipEst32F0x4:
      case Iop_RSqrtEst32F0x4:
      case Iop_Sqrt32F0x4:
      case Iop_Abs32Fx4:
      case Iop_Neg32Fx4:
      case Iop_ZeroHI96ofV128:
        arg_size = sizeof(float)*4;
        result_size = sizeof(float)*4;
        break;
      case Iop_RSqrtEst32Fx8:
      case Iop_RecipEst32Fx8:
        arg_size = sizeof(float) * 8;
        result_size = sizeof(float) * 8;
        break;
      case Iop_ZeroHI64ofV128:
      case Iop_RecipEst64Fx2:
      case Iop_RSqrtEst64Fx2:
      case Iop_Abs64Fx2:
      case Iop_Neg64Fx2:
      case Iop_Sqrt64F0x2:
        arg_size = sizeof(double) * 2;
        result_size = sizeof(double) * 2;
        break;
      default:
        return;
      }

      // Only bother instrumenting operations that we care about and
      // can do something useful with.
      switch (expr->Iex.Unop.op){
        // Add all supported unary ops to this list.
      case Iop_RecipEst32Fx4:
      case Iop_RSqrtEst32Fx4:
      case Iop_Abs32Fx4:
      case Iop_Neg32Fx4:
      case Iop_ZeroHI96ofV128:
      case Iop_ZeroHI64ofV128:
      case Iop_V128to32:
      case Iop_V128to64:
      case Iop_V128HIto64:
      case Iop_RecipEst64Fx2:
      case Iop_RSqrtEst64Fx2:
      case Iop_Abs64Fx2:
      case Iop_Neg64Fx2:
      case Iop_RecipEst32F0x4:
      case Iop_Sqrt32F0x4:
      case Iop_RSqrtEst32F0x4:
      case Iop_RSqrtEst32Fx2:
      case Iop_RecipEst32Fx2:
      case Iop_RoundF64toF32:
      case Iop_TruncF64asF32:
      case Iop_RSqrtEst5GoodF64:
      case Iop_RoundF64toF64_NEAREST:
      case Iop_RoundF64toF64_NegINF:
      case Iop_RoundF64toF64_PosINF:
      case Iop_RoundF64toF64_ZERO:
      case Iop_F128HItoF64:
      case Iop_F128LOtoF64:
      case Iop_F32toF64:
      case Iop_NegF32:
      case Iop_AbsF32:
      case Iop_NegF64:
      case Iop_AbsF64:
      case Iop_Sqrt64F0x2:
        // Allocate the memory for the argument structure
        ALLOC(opInfo, "hg.op_alloc.1", 1, sizeof(UnaryOp_Info));

        // Populate the values we know at instrument time now.
        opInfo->op = expr->Iex.Unop.op;
        opInfo->arg_tmp = expr->Iex.Unop.arg->Iex.RdTmp.tmp;
        opInfo->dest_tmp = offset;

        // Store the operations debugging information for printing the
        // errors later.
        opInfo->op_addr = opAddr;
        VG_(get_filename_linenum)(opAddr, &(opInfo->src_filename), NULL, &(opInfo->src_line));
        VG_(get_fnname)(opAddr, &(opInfo->fnname));

        // Allocate the space for the values we won't know until
        // runtime, but know their size now.
        ALLOC(opInfo->arg_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->dest_value, "hg.arg_alloc", 1, result_size);

        // Add statements to populate the values we don't know until
        // runtime.
        addStore(sb, expr->Iex.Unop.arg, opInfo->arg_value);
        addStore(sb, IRExpr_RdTmp(offset), opInfo->dest_value);

        // Finally, add the statement to call the shadow op procedure.
        executeShadowOp =
          unsafeIRDirty_0_N(1,
                            "executeUnaryShadowOp",
                            VG_(fnptr_to_fnentry)(&executeUnaryShadowOp),
                            mkIRExprVec_1(mkU64((uintptr_t)opInfo)));
        addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp));
        break;
      default:
        break;
      }
    }
    break;
  case Iex_Binop:
    {
      BinaryOp_Info* opInfo;
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
      case Iop_QAdd32S:
      case Iop_QSub32S:
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
        // Allocate the memory for the argument structure
        ALLOC(opInfo, "hg.op_alloc.1", 1, sizeof(BinaryOp_Info));

        // Populate the values we know at instrument time now.
        opInfo->op = expr->Iex.Binop.op;
        if (expr->Iex.Binop.arg1->tag == Iex_Const){
          // If we have a const instead of a temporary going into this
          // function, store it in a temp first.
          IRTemp constTemp = newIRTemp(sb->tyenv, typeOfIRConst(expr->Iex.Binop.arg1->Iex.Const.con));
          IRStmt* storeConst = IRStmt_WrTmp(constTemp, expr->Iex.Binop.arg1);
          addStmtToIRSB(sb, storeConst);
          opInfo->arg1_tmp = constTemp;
        } else {
          // Otherwise, just pass the temp argument
          opInfo->arg1_tmp = expr->Iex.Binop.arg1->Iex.RdTmp.tmp;
        }

        if (expr->Iex.Binop.arg2->tag == Iex_Const){
          // If we have a const instead of a temporary going into this
          // function, store it in a temp first.
          IRTemp constTemp = newIRTemp(sb->tyenv, typeOfIRConst(expr->Iex.Binop.arg2->Iex.Const.con));
          IRStmt* storeConst = IRStmt_WrTmp(constTemp, expr->Iex.Binop.arg2);
          addStmtToIRSB(sb, storeConst);
          opInfo->arg2_tmp = constTemp;
        } else {
          // Otherwise, just pass the temp argument
          opInfo->arg2_tmp = expr->Iex.Binop.arg2->Iex.RdTmp.tmp;
        }

        opInfo->dest_tmp = offset;

        // Store the operations debugging information for printing the
        // errors later.
        opInfo->op_addr = opAddr;
        VG_(get_filename_linenum)(opAddr, &(opInfo->src_filename), NULL, &(opInfo->src_line));
        VG_(get_fnname)(opAddr, &(opInfo->fnname));

        // Allocate the space for the values we won't know until
        // runtime, but know their size now.
        ALLOC(opInfo->arg1_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->arg2_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->dest_value, "hg.arg_alloc", 1, result_size);

        // Add statements to populate the values we don't know until
        // runtime.
        addStore(sb, expr->Iex.Binop.arg1, opInfo->arg1_value);
        addStore(sb, expr->Iex.Binop.arg2, opInfo->arg2_value);
        addStore(sb, IRExpr_RdTmp(offset), opInfo->dest_value);

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
  case Iex_Triop:
    {
      TernaryOp_Info* opInfo;

      // Determine the argument sizes of each operation we might
      // encounter so we can allocate the right amount of space in the
      // argument structure to the runtime shadow execution code.
      switch (expr->Iex.Triop.details->op){
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
        arg_size = sizeof(double);
        result_size = sizeof(double);
        break;
      case Iop_AddF32:
      case Iop_SubF32:
      case Iop_MulF32:
      case Iop_DivF32:
        arg_size = sizeof(float);
        result_size = sizeof(float);
        break;
      case Iop_Add32Fx4:
      case Iop_Sub32Fx4:
      case Iop_Mul32Fx4:
      case Iop_Div32Fx4:
        arg_size = sizeof(float) * 4;
        result_size = sizeof(float) * 4;
        break;
      case Iop_Add64Fx2:
      case Iop_Sub64Fx2:
      case Iop_Mul64Fx2:
      case Iop_Div64Fx2:
        arg_size = sizeof(double) * 2;
        result_size = sizeof(double) * 2;
        break;
      case Iop_Add32Fx8:
      case Iop_Sub32Fx8:
      case Iop_Mul32Fx8:
      case Iop_Div32Fx8:
        arg_size = sizeof(float) * 8;
        result_size = sizeof(float) * 8;
      case Iop_Add64Fx4:
      case Iop_Sub64Fx4:
      case Iop_Mul64Fx4:
      case Iop_Div64Fx4:
        arg_size = sizeof(double) * 4;
        result_size = sizeof(double) * 4;
        break;
      default:
        return;
      }

      // Only bother instrumenting operations that we care about and
      // can do something useful with.
      switch (expr->Iex.Triop.details->op){
        // Add all supported ternary ops to this list
      case Iop_Add32Fx8:
      case Iop_Sub32Fx8:
      case Iop_Mul32Fx8:
      case Iop_Div32Fx8:
      case Iop_Add64Fx4:
      case Iop_Sub64Fx4:
      case Iop_Mul64Fx4:
      case Iop_Div64Fx4:
      case Iop_Add32Fx4:
      case Iop_Sub32Fx4:
      case Iop_Mul32Fx4:
      case Iop_Div32Fx4:
      case Iop_Add64Fx2:
      case Iop_Sub64Fx2:
      case Iop_Mul64Fx2:
      case Iop_Div64Fx2:
      case Iop_AtanF64:
      case Iop_Yl2xF64:
      case Iop_Yl2xp1F64:
      case Iop_ScaleF64:
      case Iop_AddF128:
      case Iop_SubF128:
      case Iop_MulF128:
      case Iop_DivF128:
      case Iop_AddF64:
      case Iop_SubF64:
      case Iop_MulF64:
      case Iop_DivF64:
      case Iop_AddF32:
      case Iop_SubF32:
      case Iop_MulF32:
      case Iop_DivF32:
      case Iop_AddF64r32:
      case Iop_SubF64r32:
      case Iop_MulF64r32:
      case Iop_DivF64r32:
        // Allocate the memory for the argument structure
        ALLOC(opInfo, "hg.op_alloc.1", 1, sizeof(TernaryOp_Info));

        // Populate the values we know at instrument time now.
        opInfo->op = expr->Iex.Triop.details->op;
        opInfo->arg1_tmp = expr->Iex.Triop.details->arg1->Iex.RdTmp.tmp;
        opInfo->arg2_tmp = expr->Iex.Triop.details->arg2->Iex.RdTmp.tmp;
        opInfo->arg3_tmp = expr->Iex.Triop.details->arg3->Iex.RdTmp.tmp;
        opInfo->dest_tmp = offset;

        // Store the operations debugging information for printing the
        // errors later.
        opInfo->op_addr = opAddr;
        VG_(get_filename_linenum)(opAddr, &(opInfo->src_filename), NULL, &(opInfo->src_line));
        VG_(get_fnname)(opAddr, &(opInfo->fnname));

        // Allocate the space for the values we won't know until
        // runtime, but know their size now.
        ALLOC(opInfo->arg1_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->arg2_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->arg3_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->dest_value, "hg.arg_alloc", 1, result_size);

        // Add statements to populate the values we don't know until
        // runtime.
        addStore(sb, expr->Iex.Triop.details->arg1, opInfo->arg1_value);
        addStore(sb, expr->Iex.Triop.details->arg2, opInfo->arg2_value);
        addStore(sb, expr->Iex.Triop.details->arg3, opInfo->arg3_value);
        addStore(sb, IRExpr_RdTmp(offset), opInfo->dest_value);

        // Finally, add the statement to call the shadow op procedure.
        executeShadowOp =
          unsafeIRDirty_0_N(1,
                            "executeTernaryShadowOp",
                            VG_(fnptr_to_fnentry)(&executeTernaryShadowOp),
                            mkIRExprVec_1(mkU64((uintptr_t)opInfo)));
        addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp));
        break;
      default:
        break;
      }
    }
    break;
  case Iex_Qop:
    {
      QuadnaryOp_Info* opInfo;

      // Determine the argument sizes of each operation we might
      // encounter so we can allocate the right amount of space in the
      // argument structure to the runtime shadow execution code.
      switch (expr->Iex.Qop.details->op){
      case Iop_MAddF32:
      case Iop_MSubF32:
        arg_size = sizeof(float);
        result_size = sizeof(float);
        break;
      case Iop_MAddF64:
      case Iop_MSubF64:
      case Iop_MAddF64r32:
      case Iop_MSubF64r32:
        arg_size = sizeof(double);
        result_size = sizeof(double);
        break;
      default:
        return;
      }

      // Only bother instrumenting operations that we care about and
      // can do something useful with.
      switch (expr->Iex.Qop.details->op){
        // Add all supported quadnary ops to this list
      case Iop_MAddF32:
      case Iop_MSubF32:
      case Iop_MAddF64:
      case Iop_MSubF64:
      case Iop_MAddF64r32:
      case Iop_MSubF64r32:
        // Allocate the memory for the argument structure
        ALLOC(opInfo, "hg.op_alloc.1", 1, sizeof(QuadnaryOp_Info));

        // Populate the values we know at instrument time now.
        opInfo->op = expr->Iex.Qop.details->op;
        opInfo->arg1_tmp = expr->Iex.Qop.details->arg1->Iex.RdTmp.tmp;
        opInfo->arg2_tmp = expr->Iex.Qop.details->arg2->Iex.RdTmp.tmp;
        opInfo->arg3_tmp = expr->Iex.Qop.details->arg3->Iex.RdTmp.tmp;
        opInfo->arg4_tmp = expr->Iex.Qop.details->arg4->Iex.RdTmp.tmp;
        opInfo->dest_tmp = offset;

        // Store the operations debugging information for printing the
        // errors later.
        opInfo->op_addr = opAddr;
        VG_(get_filename_linenum)(opAddr, &(opInfo->src_filename), NULL, &(opInfo->src_line));
        VG_(get_fnname)(opAddr, &(opInfo->fnname));

        // Allocate the space for the values we won't know until
        // runtime, but know their size now.
        ALLOC(opInfo->arg1_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->arg2_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->arg3_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->arg4_value, "hg.arg_alloc", 1, arg_size);
        ALLOC(opInfo->dest_value, "hg.arg_alloc", 1, result_size);

        // Add statements to populate the values we don't know until
        // runtime.
        addStore(sb, expr->Iex.Qop.details->arg1, opInfo->arg1_value);
        addStore(sb, expr->Iex.Qop.details->arg2, opInfo->arg2_value);
        addStore(sb, expr->Iex.Qop.details->arg3, opInfo->arg3_value);
        addStore(sb, expr->Iex.Qop.details->arg3, opInfo->arg4_value);
        addStore(sb, IRExpr_RdTmp(offset), opInfo->dest_value);

        // Finally, add the statement to call the shadow op procedure.
        executeShadowOp =
          unsafeIRDirty_0_N(1,
                            "executeQuadnaryShadowOp",
                            VG_(fnptr_to_fnentry)(&executeQuadnaryShadowOp),
                            mkIRExprVec_1(mkU64((uintptr_t)opInfo)));
        addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp));
        break;
      default:
        break;
      }
      break;
    }
  default:
    VG_(dmsg)("BAD THINGS!!!!\n");
  }
}
