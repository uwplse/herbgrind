#include "hg_instrument.h"

// Add instrumenting expressions to sb for an operation, storing the
// result in the temporary at offset.
void instrumentOp(IRSB* sb, Int offset, IRExpr* expr){
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
  //
  // TODO: Fill in these cases.
  switch (expr->tag){
  case Iex_Unop:
    {
      UnaryOp_Info* opInfo;
      // Determine the argument sizes of each operation we might
      // encounter so we can allocate the right amount of space in the
      // argument structure to the runtime shadow execution code.
      switch(expr->Iex.Unop.op){
      case Iop_F128HItoF64:
      case Iop_F128LOtoF64:
        arg_size = sizeof(double) * 2;
        result_size = sizeof(double);
        break;
      case Iop_F32toF64:
        arg_size = sizeof(float);
        result_size = sizeof(double);
        break;
      case Iop_NegF32:
      case Iop_AbsF32:
        arg_size = sizeof(float);
        result_size = sizeof(float);
        break;
      case Iop_NegF64:
      case Iop_AbsF64:
        arg_size = sizeof(double);
        result_size = sizeof(double);
        break;
      case Iop_Sqrt64F0x2:
        arg_size = sizeof(double) * 2;
        result_size = sizeof(double) * 2;
        break;
      default:
        break;
      }

      // Only bother instrumenting operations that we care about and
      // can do something useful with.
      switch (expr->Iex.Unop.op){
        // Add all supported unary ops to this list.
      case Iop_F128HItoF64:
      case Iop_F128LOtoF64:
      case Iop_F32toF64:
      case Iop_NegF32:
      case Iop_AbsF32:
      case Iop_NegF64:
      case Iop_AbsF64:
      case Iop_Sqrt64F0x2:
        // Allocate the memory for the argument structure
        opInfo = VG_(malloc)("hg.op_alloc.1", sizeof(UnaryOp_Info));

        // Populate the values we know at instrument time now.
        opInfo->op = expr->Iex.Binop.op;
        opInfo->arg_tmp = expr->Iex.Unop.arg->Iex.RdTmp.tmp;
        opInfo->dest_tmp = offset;

        // Allocate the space for the values we won't know until
        // runtime, but know their size now.
        opInfo->arg_value = VG_(malloc)("hg.arg_alloc", arg_size);
        opInfo->dest_value = VG_(malloc)("hg.arg_alloc", result_size);

        // Add statements to populate the values we don't know until
        // runtime.
        addStore(sb, expr->Iex.Unop.arg, opInfo->arg_value);
        addStore(sb, IRExpr_RdTmp(offset), opInfo->dest_value);

        // Finally, add the statement to call the shadow op procedure.
        executeShadowOp =
          unsafeIRDirty_0_N(1,
                            "executeUnaryShadowOp",
                            VG_(fnptr_to_fnentry)(&executeUnaryShadowOp),
                            mkIRExprVec_1(mkU64((ULong)opInfo)));
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
      case Iop_F64HLtoF128:
        arg_size = sizeof(double);
        result_size = sizeof(double)*2;
        break;
      case Iop_F64toF32:
        arg_size = sizeof(double);
        result_size = sizeof(float);
        break;
      case Iop_SqrtF64:
        arg_size = sizeof(double);
        result_size = sizeof(double);
        break;
      case Iop_SqrtF32:
        arg_size = sizeof(float);
        result_size = sizeof(float);
        break;
      case Iop_Add64F0x2:
      case Iop_SetV128lo64:
        arg_size = sizeof(double) * 2;
        result_size = sizeof(double) * 2;
        break;
      default:
        break;
      }

      // Only bother instrumenting operations that we care about and
      // can do something useful with.
      switch (expr->Iex.Binop.op){
        // Add all supported binary ops to this list
      case Iop_F64HLtoF128:
      case Iop_F64toF32:
      case Iop_SqrtF64:
      case Iop_SqrtF32:
      case Iop_Add64F0x2:
      case Iop_SetV128lo64:
        // Allocate the memory for the argument structure
        opInfo = VG_(malloc)("hg.op_alloc.1", sizeof(BinaryOp_Info));

        // Populate the values we know at instrument time now.
        opInfo->op = expr->Iex.Binop.op;
        opInfo->arg1_tmp = expr->Iex.Binop.arg1->Iex.RdTmp.tmp;
        opInfo->arg2_tmp = expr->Iex.Binop.arg2->Iex.RdTmp.tmp;
        opInfo->dest_tmp = offset;

        // Allocate the space for the values we won't know until
        // runtime, but know their size now.
        opInfo->arg1_value = VG_(malloc)("hg.arg_alloc", arg_size);
        opInfo->arg2_value = VG_(malloc)("hg.arg_alloc", arg_size);
        opInfo->dest_value = VG_(malloc)("hg.arg_alloc", result_size);

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
                            mkIRExprVec_1(mkU64((ULong)opInfo)));
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
      case Iop_AddF32:
      case Iop_SubF32:
      case Iop_MulF32:
      case Iop_DivF32:
        arg_size = sizeof(float);
        result_size = sizeof(float);
      default:
        break;
      }

      // Only bother instrumenting operations that we care about and
      // can do something useful with.
      switch (expr->Iex.Triop.details->op){
        // Add all supported ternary ops to this list
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
        opInfo = VG_(malloc)("hg.op_alloc.1", sizeof(TernaryOp_Info));

        // Populate the values we know at instrument time now.
        opInfo->op = expr->Iex.Triop.details->op;
        opInfo->arg1_tmp = expr->Iex.Triop.details->arg1->Iex.RdTmp.tmp;
        opInfo->arg2_tmp = expr->Iex.Triop.details->arg2->Iex.RdTmp.tmp;
        opInfo->arg3_tmp = expr->Iex.Triop.details->arg3->Iex.RdTmp.tmp;
        opInfo->dest_tmp = offset;

        // Allocate the space for the values we won't know until
        // runtime, but know their size now.
        opInfo->arg1_value = VG_(malloc)("hg.arg_alloc", arg_size);
        opInfo->arg2_value = VG_(malloc)("hg.arg_alloc", arg_size);
        opInfo->arg3_value = VG_(malloc)("hg.arg_alloc", arg_size);
        opInfo->dest_value = VG_(malloc)("hg.arg_alloc", result_size);

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
                            mkIRExprVec_1(mkU64((ULong)opInfo)));
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
      default:
        break;
      }

      // Only bother instrumenting operations that we care about and
      // can do something useful with.
      switch (expr->Iex.Qop.details->op){
        // Add all supported quadnary ops to this list
      /* case 1: */
      /*   // Allocate the memory for the argument structure */
      /*   opInfo = VG_(malloc)("hg.op_alloc.1", sizeof(QuadnaryOp_Info)); */

      /*   // Populate the values we know at instrument time now. */
      /*   opInfo->op = expr->Iex.Qop.details->op; */
      /*   opInfo->arg1_tmp = expr->Iex.Qop.details->arg1->Iex.RdTmp.tmp; */
      /*   opInfo->arg2_tmp = expr->Iex.Qop.details->arg2->Iex.RdTmp.tmp; */
      /*   opInfo->arg3_tmp = expr->Iex.Qop.details->arg3->Iex.RdTmp.tmp; */
      /*   opInfo->arg4_tmp = expr->Iex.Qop.details->arg4->Iex.RdTmp.tmp; */
      /*   opInfo->dest_tmp = offset; */

      /*   // Allocate the space for the values we won't know until */
      /*   // runtime, but know their size now. */
      /*   opInfo->arg1_value = VG_(malloc)("hg.arg_alloc", arg_size); */
      /*   opInfo->arg2_value = VG_(malloc)("hg.arg_alloc", arg_size); */
      /*   opInfo->arg3_value = VG_(malloc)("hg.arg_alloc", arg_size); */
      /*   opInfo->arg4_value = VG_(malloc)("hg.arg_alloc", arg_size); */
      /*   opInfo->dest_value = VG_(malloc)("hg.arg_alloc", result_size); */

      /*   // Add statements to populate the values we don't know until */
      /*   // runtime. */
      /*   addStore(sb, expr->Iex.Qop.details->arg1, opInfo->arg1_value); */
      /*   addStore(sb, expr->Iex.Qop.details->arg2, opInfo->arg2_value); */
      /*   addStore(sb, expr->Iex.Qop.details->arg3, opInfo->arg3_value); */
      /*   addStore(sb, expr->Iex.Qop.details->arg3, opInfo->arg4_value); */
      /*   addStore(sb, IRExpr_RdTmp(offset), opInfo->dest_value); */

      /*   // Finally, add the statement to call the shadow op procedure. */
      /*   executeShadowOp = */
      /*     unsafeIRDirty_0_N(1, */
      /*                       "executeQuadnaryShadowOp", */
      /*                       VG_(fnptr_to_fnentry)(&executeQuadnaryShadowOp), */
      /*                       mkIRExprVec_1(mkU64((ULong)opInfo))); */
      /*   addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp)); */
      /*   break; */
      default:
        break;
      }
      break;
    }
  default:
    VG_(dmsg)("BAD THINGS!!!!\n");
  }
}
