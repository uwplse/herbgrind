#include "hg_runtime.h"
#include "hg_hiprec_ops.h"

// Execute a shadow operation, storing the result of the high
// precision operation applied to the shadow value at the arg_tmp
// offset, into the shadow value at the dest_tmp offset. Depending on
// the value of op, we'll know which operation to apply. op will be
// mapped to operations by the enumeration at libvex_ir.h:415.
VG_REGPARM(1) void executeUnaryShadowOp(UnaryOp_Info* opInfo){
  ShadowLocation* argLocation;
  ShadowLocation* destLocation;

  switch(opInfo->op){
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
    {
      size_t num_vals;
      LocType argType;
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_rnd_t);

      switch(opInfo->op){
      case Iop_RecipEst32Fx8:
      case Iop_RecipEst32Fx4:
      case Iop_RecipEst32Fx2:
      case Iop_RecipEst64Fx2:
        mpfr_func = hiprec_recip;
        break;
      case Iop_RSqrtEst32Fx8:
      case Iop_RSqrtEst32Fx4:
      case Iop_RSqrtEst32Fx2:
      case Iop_RSqrtEst64Fx2:
        mpfr_func = mpfr_rec_sqrt;
        break;
      case Iop_Neg64Fx2:
      case Iop_Neg32Fx2:
      case Iop_Neg32Fx4:
        mpfr_func = mpfr_neg;
        break;
      case Iop_Abs64Fx2:
      case Iop_Abs32Fx2:
      case Iop_Abs32Fx4:
        mpfr_func = mpfr_abs;
        break;
      default:
        break;
      }
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
      default:
        break;
      }
      // Pull the shadow values for the argument. If we don't already
      // have shadow values for this argument, we'll generate a fresh
      // one from the runtime float value.
      argLocation = getShadowLocation(opInfo->arg_tmp, argType, opInfo->arg_value);
      destLocation = mkShadowLocation(argType);
      for (int i = 0; i < num_vals; ++i){
        mpfr_func(destLocation->values[i].value, argLocation->values[i].value, MPFR_RNDN);
        // Evaluate the computed value against the high precision shadow result.
        switch(argType){
        case Lt_Floatx2:
        case Lt_Floatx4:
        case Lt_Floatx8:
          evaluateOpError(&(destLocation->values[i]), ((float*)opInfo->dest_value)[i]);
          break;
        case Lt_Doublex2:
          evaluateOpError(&(destLocation->values[i]), ((double*)opInfo->dest_value)[i]);
          break;
        }
      }
    }
    break;
  case Iop_ZeroHI64ofV128:
  case Iop_ZeroHI96ofV128:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_V128to32:
    {
      LocType argType, resultType;
      switch(opInfo->op){
      case Iop_ZeroHI64ofV128:
        argType = Lt_Doublex2;
        resultType = Lt_Doublex2;
        break;
      case Iop_ZeroHI96ofV128:
        argType = Lt_Floatx4;
        resultType = Lt_Floatx4;
        break;
      case Iop_F128HItoF64:
      case Iop_F128LOtoF64:
      case Iop_V128to64:
      case Iop_V128HIto64:
        argType = Lt_Doublex2;
        resultType = Lt_Double;
        break;
      case Iop_V128to32:
        argType = Lt_Floatx4;
        resultType = Lt_Float;
        break;
      }
      argLocation = getShadowLocation(opInfo->arg_tmp, argType, opInfo->arg_value);
      destLocation = mkShadowLocation(resultType);
      switch(opInfo->op){
      case Iop_V128HIto64:
      case Iop_F128HItoF64:
        destLocation->values[0] = argLocation->values[1];
        break;
      case Iop_ZeroHI64ofV128:
      case Iop_ZeroHI96ofV128:
      case Iop_V128to64:
      case Iop_V128to32:
      case Iop_F128LOtoF64:
        destLocation->values[0] = argLocation->values[0];
        break;
      default:
        break;
      }
    }
    break;
  case Iop_F32toF64:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF32:
    {
      LocType argType, resultType;
      switch(opInfo->op){
      case Iop_F32toF64:
        argType = Lt_Float;
        resultType = Lt_Double;
        break;
      case Iop_TruncF64asF32:
        argType = Lt_Double;
        resultType = Lt_Float;
        break;
      case Iop_RoundF64toF32:
        argType = Lt_Double;
        resultType = Lt_Double;
        break;
      default:
        break;
      }
      argLocation = getShadowLocation(opInfo->arg_tmp, argType, opInfo->arg_value);
      destLocation = mkShadowLocation(resultType);
      destLocation->values[0] = argLocation->values[0];
    }
    break;
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
    argLocation = getShadowLocation(opInfo->arg_tmp, Lt_Double, opInfo->arg_value);
    destLocation = mkShadowLocation(Lt_Double);
    switch(opInfo->op){
    case Iop_RoundF64toF64_NEAREST:
      mpfr_round(destLocation->values[0].value, argLocation->values[0].value);
      break;
    case Iop_RoundF64toF64_NegINF:
      mpfr_floor(destLocation->values[0].value, argLocation->values[0].value);
      break;
    case Iop_RoundF64toF64_PosINF:
      mpfr_ceil(destLocation->values[0].value, argLocation->values[0].value);
      break;
    case Iop_RoundF64toF64_ZERO:
      mpfr_trunc(destLocation->values[0].value, argLocation->values[0].value);
      break;
    default:
      break;
    }
    break;
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
      LocType argType;
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_rnd_t);

      // Determine the type of the arguments
      switch(opInfo->op){
      case Iop_NegF32:
      case Iop_AbsF32:
        argType = Lt_Float;
        break;
      case Iop_NegF64:
      case Iop_AbsF64:
        argType = Lt_Double;
        break;
      case Iop_RecipEst32F0x4:
      case Iop_Sqrt32F0x4:
      case Iop_RSqrtEst32F0x4:
        argType = Lt_Floatx4;
        break;
      case Iop_Sqrt64F0x2:
        argType = Lt_Doublex2;
        break;
      default:
        break;
      }
      // Determine the mpfr shadow function
      switch(opInfo->op){
      case Iop_NegF64:
      case Iop_NegF32:
        mpfr_func = mpfr_neg;
        break;
      case Iop_AbsF64:
      case Iop_AbsF32:
        mpfr_func = mpfr_abs;
        break;
      case Iop_Sqrt32F0x4:
      case Iop_Sqrt64F0x2:
        mpfr_func = mpfr_sqrt;
        break;
      case Iop_RecipEst32F0x4:
        mpfr_func = hiprec_recip;
        break;
      case Iop_RSqrtEst32F0x4:
        mpfr_func = mpfr_rec_sqrt;
        break;
      default:
        break;
      }

      // Pull the shadow location for the argument. If we don't already
      // have a shadow location for this argument, we'll generate a fresh
      // one from the runtime float value.
      argLocation = getShadowLocation(opInfo->arg_tmp, argType, opInfo->arg_value);
    
      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation(argType);

      // Set the low order bits to the result of the sqrt, but in
      // higher precision.
      mpfr_func(destLocation->values[0].value, argLocation->values[0].value, MPFR_RNDN);

      // Do any extra work for transferring shadow values.
      switch(opInfo->op){
      case Iop_Sqrt64F0x2:
        // Copy across the high order bits shadow value.
        destLocation->values[1] = argLocation->values[1];
        break;
      default:
        break;
      }

      // Now, we'll evaluate the shadow value against the result value.
      evaluateOpError(&(destLocation->values[0]), ((double*)opInfo->dest_value)[0]);
    }
    break;

  default:
    break;
  }

  // Put the resulting location in the space for the dest temp.
  localTemps[opInfo->dest_tmp] = destLocation;

}
VG_REGPARM(1) void executeBinaryShadowOp(BinaryOp_Info* opInfo){
  // The shadowing locations for the first argument, second argument,
  // and destination respectively.
  ShadowLocation* arg1Location;
  ShadowLocation* arg2Location;
  ShadowLocation* destLocation;
  switch(opInfo->op){
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
    // Pull the shadow values for the arguments. If we don't already
    // have shadow values for these arguments, we'll generate fresh
    // ones from the runtime float values.
    arg1Location = getShadowLocation(opInfo->arg1_tmp, Lt_Double, opInfo->arg1_value);
    arg2Location = getShadowLocation(opInfo->arg2_tmp, Lt_Double, opInfo->arg2_value);

    // Now we'll allocate memory for the shadowed result of this
    // operation.
    destLocation = mkShadowLocation(Lt_Doublex2);

    // Finally, take the 64 bits of each argument, and put them in the
    // two halves of the result.
    destLocation->values[0] = arg1Location->values[0];
    destLocation->values[1] = arg2Location->values[0];
    break;
  case Iop_RoundF64toInt:
  case Iop_RoundF32toInt:
    {
      LocType argType;
      switch(opInfo->op){
      case Iop_RoundF64toInt:
        argType = Lt_Double;
        break;
      case Iop_RoundF32toInt:
        argType = Lt_Float;
        break;
      default:
        break;
      }
    arg2Location = getShadowLocation(opInfo->arg2_tmp, argType, opInfo->arg2_value);
    destLocation = mkShadowLocation(argType);
    mpfr_round(destLocation->values[0].value, arg2Location->values[0].value);
    }
    break;
  case Iop_F64toF32:
    // For semantic conversions between floating point types we can
    // just copy across the values.
    arg2Location = getShadowLocation(opInfo->arg2_tmp, Lt_Double, opInfo->arg2_value);
    destLocation = mkShadowLocation(Lt_Float);
    destLocation->values[0] = arg2Location->values[0];
    break;
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
    {
      LocType argType;
      size_t num_values;
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_rnd_t);

      // Determine the type of the arguments.
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
      case Iop_SqrtF64x4:
        argType = Lt_Doublex4;
        num_values = 4;
        break;
      case Iop_Sqrt64Fx2:
        argType = Lt_Doublex2;
        num_values = 2;
        break;
      default:
        break;
      }

      // Determine the mpfr shadow function
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
        break;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, argType, opInfo->arg2_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation(argType);

      for (int i = 0; i < num_values; ++i){
        // Set the low order bits to the result of the addition, but in
        // higher precision.
        mpfr_func(destLocation->values[i].value, arg2Location->values[i].value, MPFR_RNDN);

        // Now, we'll evaluate the low order shadow value against the low
        // order 64-bits of the result value.
        switch(argType){
        case Lt_Doublex2:
        case Lt_Double:
          evaluateOpError(&(destLocation->values[i]), ((double*)opInfo->dest_value)[i]);
          break;
        case Lt_Floatx4:
        case Lt_Floatx2:
        case Lt_Float:
          evaluateOpError(&(destLocation->values[i]), ((float*)opInfo->dest_value)[i]);
          break;
        default:
          break;
        }
      }
    }
  case Iop_Add32F0x4:
  case Iop_Sub32F0x4:
  case Iop_Mul32F0x4:
  case Iop_Div32F0x4:
    {
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_t, mpfr_rnd_t);
      switch(opInfo->op){
      case Iop_Add32F0x4:
        mpfr_func = mpfr_add;
        break;
      case Iop_Sub32F0x4:
        mpfr_func = mpfr_sub;
        break;
      case Iop_Mul32F0x4:
        mpfr_func = mpfr_mul;
        break;
      case Iop_Div32F0x4:
        mpfr_func = mpfr_div;
        break;
      default:
        break;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg1Location =
        getShadowLocation(opInfo->arg1_tmp, Lt_Floatx4, opInfo->arg1_value);
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, Lt_Floatx4, opInfo->arg2_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation(Lt_Floatx4);

      // Set the low order destination shadow value to the result of
      // the operation, and the other shadow values to the the shadow
      // values from the first argument.
      mpfr_func(destLocation->values[0].value, arg1Location->values[0].value,
                arg2Location->values[0].value, MPFR_RNDN);
      destLocation->values[1] = arg1Location->values[1];
      destLocation->values[2] = arg1Location->values[2];
      destLocation->values[3] = arg1Location->values[3];
    }
    break;
  case Iop_RecipStep32Fx4:
  case Iop_RSqrtStep32Fx4:
  case Iop_RSqrtStep32Fx2:
  case Iop_RecipStep32Fx2:
  case Iop_RecipStep64Fx2:
  case Iop_RSqrtStep64Fx2:
  case Iop_Add32Fx2:
  case Iop_Sub32Fx2:
    {
      LocType argType;
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_t, mpfr_rnd_t);
      size_t num_values;
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
      case Iop_Add32Fx2:
        mpfr_func = mpfr_add;
        break;
      case Iop_Sub32Fx2:
        mpfr_func = mpfr_sub;
        break;
      default:
        break;
      }
      switch(opInfo->op){
      case Iop_Add32Fx2:
      case Iop_Sub32Fx2:
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
        argType = Lt_Doublex4;
        num_values = 4;
        break;
      default:
        break;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg1Location =
        getShadowLocation(opInfo->arg1_tmp, argType, opInfo->arg1_value);
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, argType, opInfo->arg2_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation(argType);

      for (int i = 0; i < num_values; ++i){
        // Set the destination shadow values to the result of a
        // high-precision shadowing operation, for each channel in which
        // it occurs.
        mpfr_func(destLocation->values[i].value, arg1Location->values[i].value,
                  arg2Location->values[i].value, MPFR_RNDN);
        // Now, we'll evaluate the shadow value against the result
        // value, for each of it's channels.
        switch(argType){
        case Lt_Float:
        case Lt_Floatx2:
        case Lt_Floatx4:
          evaluateOpError(&(destLocation->values[i]), ((float*)opInfo->dest_value)[i]);
          break;
        case Lt_Double:
        case Lt_Doublex2:
          evaluateOpError(&(destLocation->values[i]), ((double*)opInfo->dest_value)[i]);
          break;
        default:
          break;
        }
      }

    }
    break;
  case Iop_QAdd32S:
  case Iop_QSub32S:
    {
      // Determine what the underlying operation is
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_t, mpfr_rnd_t);
      switch(opInfo->op){
      case Iop_RSqrtStep32Fx2:
        mpfr_func = hiprec_rsqrtstep;
        break;
      case Iop_RecipStep32Fx2:
        mpfr_func = hiprec_recipstep;
        break;
      case Iop_QAdd32S:
        mpfr_func = mpfr_add;
        break;
      case Iop_QSub32S:
        mpfr_func = mpfr_sub;
        break;
      default:
        break;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg1Location =
        getShadowLocation(opInfo->arg1_tmp, Lt_Float, opInfo->arg1_value);
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, Lt_Float, opInfo->arg2_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation(Lt_Float);

      // Set the destination shadow value to the result of a
      // high-precision shadowing operation.
      mpfr_func(destLocation->values[0].value, arg1Location->values[0].value,
                arg2Location->values[0].value, MPFR_RNDN);

      // Now, we'll evaluate the shadow value against the result value.
      evaluateOpError(&(destLocation->values[0]), ((float*)opInfo->dest_value)[0]);
    }
    break;
  case Iop_Add64F0x2:
  case Iop_Sub64F0x2:
  case Iop_Mul64F0x2:
  case Iop_Div64F0x2:
    {
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_t, mpfr_rnd_t);
      switch(opInfo->op){
      case Iop_Add64F0x2:
        mpfr_func = mpfr_add;
        break;
      case Iop_Sub64F0x2:
        mpfr_func = mpfr_sub;
        break;
      case Iop_Mul64F0x2:
        mpfr_func = mpfr_mul;
        break;
      case Iop_Div64F0x2:
        mpfr_func = mpfr_div;
        break;
      default:
        break;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg1Location =
        getShadowLocation(opInfo->arg1_tmp, Lt_Doublex2, opInfo->arg1_value);
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, Lt_Doublex2, opInfo->arg2_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation, which is a 128-bit SIMD value. The high order
      // 64-bits are taken from the first argument, while the low order
      // 64-bits are the result of the operation.
      destLocation = mkShadowLocation(Lt_Doublex2);

      // Copy across the high order bits shadow value.
      destLocation->values[1] = arg1Location->values[1];

      // Set the low order bits to the result of the operation, but in
      // higher precision.
      mpfr_func(destLocation->values[0].value, arg1Location->values[0].value,
                arg2Location->values[0].value, MPFR_RNDN);

      // Now, we'll evaluate the low order shadow value against the low
      // order 64-bits of the result value.
      evaluateOpError(&(destLocation->values[0]), ((double*)opInfo->dest_value)[0]);
    }
    break;
  case Iop_SetV128lo32:
  case Iop_SetV128lo64:
    {
      LocType type;
      size_t num_vals;
      switch(opInfo->op){
      case Iop_SetV128lo32:
        type = Lt_Floatx4;
        num_vals = 4;
        break;
      case Iop_SetV128lo64:
        type = Lt_Doublex2;
        num_vals = 2;
        break;
      default:
        break;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg1Location =
        getShadowLocation(opInfo->arg1_tmp, type, opInfo->arg1_value);
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, type, opInfo->arg2_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation, which is a 128-bit SIMD value. The high order
      // 64-bits are taken from the first argument, while the low order
      // 64-bits are taken from the second argument.
      destLocation = mkShadowLocation(type);

      // Copy the low order bits shadow value from the second argument.
      destLocation->values[0] = arg2Location->values[0];

      // Copy across the higher order bits shadow value from the first
      // argument.
      for (int i = 1; i < num_vals; ++i)
        destLocation->values[i] = arg1Location->values[i];

      // This isn't really a "real" op in the math-y sense, so let's not
      // evaluate it's error.
    }
    break;
  default:
    break;
  }
  // Put the resulting location in the space for the dest temp.
  localTemps[opInfo->dest_tmp] = destLocation;

}
VG_REGPARM(1) void executeTernaryShadowOp(TernaryOp_Info* opInfo){
  // The shadowing locations for the arguments and the destination.
  ShadowLocation *arg1Location, *arg2Location, *arg3Location, *destLocation;
  // We're going to use a lot of switch statements here because there
  // are a lot of similar cases, and we don't want to repeat too much
  // code. Hopefully, it will be moderately well organized.
  switch(opInfo->op){
    // This class of operations is all pretty similar
  case Iop_AtanF64:
  case Iop_Yl2xF64:
  case Iop_Yl2xp1F64:
  case Iop_ScaleF64:
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
    {
      LocType argType;
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_t, mpfr_rnd_t);
      // Determine the type of the arguments
      switch(opInfo->op){
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
        argType = Lt_Double;
        break;
      case Iop_AddF32:
      case Iop_SubF32:
      case Iop_MulF32:
      case Iop_DivF32:
        argType = Lt_Float;
      default:
        break;
      }
      // Determine the mpfr shadow function
      switch(opInfo->op){
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
      case Iop_AddF64:
      case Iop_AddF32:
      case Iop_AddF64r32:
        mpfr_func = mpfr_add;
        break;
      case Iop_SubF64:
      case Iop_SubF32:
      case Iop_SubF64r32:
        mpfr_func = mpfr_sub;
        break;
      case Iop_MulF64:
      case Iop_MulF32:
      case Iop_MulF64r32:
        mpfr_func = mpfr_mul;
        break;
      case Iop_DivF64:
      case Iop_DivF32:
      case Iop_DivF64r32:
        mpfr_func = mpfr_div;
        break;
      default:
        break;
      }
      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, argType, opInfo->arg2_value);
      arg3Location =
        getShadowLocation(opInfo->arg3_tmp, argType, opInfo->arg3_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation(argType);

      // Set the destination shadow value to the result of a
      // high-precision shadowing addition.
      mpfr_func(destLocation->values[0].value, arg2Location->values[0].value,
                arg3Location->values[0].value, roundmodeIRtoMPFR(((IRRoundingMode*)opInfo->arg1_value)[0]));
    
      // Now, we'll evaluate the shadow value against the result value.
      switch(argType){
      case Lt_DoubleDouble:
        VG_(dmsg)("\
Wow, you're working with some really big floats. We can't evaluate the \n\
precision of those operations right now, but we're sure as hell \n\
keeping track of them.");
        break;
      case Lt_Double:
        evaluateOpError(&(destLocation->values[0]), ((double*)opInfo->dest_value)[0]);
        break;
      case Lt_Float:
        evaluateOpError(&(destLocation->values[0]), ((float*)opInfo->dest_value)[0]);
        break;
      default:
        break;
      }
    }
    break;
  case Iop_Add32Fx8:
  case Iop_Sub32Fx8:
  case Iop_Mul32Fx8:
  case Iop_Div32Fx8:
  case Iop_Add64Fx4:
  case Iop_Sub64Fx4:
  case Iop_Mul64Fx4:
  case Iop_Div64Fx4:
  case Iop_Add64Fx2:
  case Iop_Sub64Fx2:
  case Iop_Mul64Fx2:
  case Iop_Div64Fx2:
  case Iop_Add32Fx4:
  case Iop_Sub32Fx4:
  case Iop_Mul32Fx4:
  case Iop_Div32Fx4:
    {
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_t, mpfr_rnd_t);
      LocType type;
      int num_vals;
      switch(opInfo->op){
      case Iop_Add32Fx4:
      case Iop_Add32Fx8:
      case Iop_Add64Fx2:
      case Iop_Add64Fx4:
        mpfr_func = mpfr_add;
        break;
      case Iop_Sub32Fx4:
      case Iop_Sub32Fx8:
      case Iop_Sub64Fx2:
      case Iop_Sub64Fx4:
        mpfr_func = mpfr_sub;
        break;
      case Iop_Mul32Fx4:
      case Iop_Mul32Fx8:
      case Iop_Mul64Fx2:
      case Iop_Mul64Fx4:
        mpfr_func = mpfr_mul;
        break;
      case Iop_Div32Fx4:
      case Iop_Div32Fx8:
      case Iop_Div64Fx2:
      case Iop_Div64Fx4:
        mpfr_func = mpfr_div;
        break;
      default:
        break;
      }
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
      default:
        break;
      }

      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, type, opInfo->arg2_value);
      arg3Location =
        getShadowLocation(opInfo->arg3_tmp, type, opInfo->arg3_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation(type);

      for (int i = 0; i < num_vals; ++i){
        // Set the destination shadow value to the result of a
        // high-precision shadowing operation.
        mpfr_func(destLocation->values[i].value, arg2Location->values[i].value,
                  arg2Location->values[i].value, roundmodeIRtoMPFR(((IRRoundingMode*)opInfo->arg1_value)[0]));
        // Now let's compare the computed value to the high precision result.
        switch(type){
        case Lt_Doublex4:
        case Lt_Doublex2:
          evaluateOpError(&(destLocation->values[i]), ((double*)opInfo->dest_value)[i]);
          break;
        case Lt_Floatx8:
        case Lt_Floatx4:
          evaluateOpError(&(destLocation->values[i]), ((float*)opInfo->dest_value)[i]);
          break;
        default:
          break;
        }
      }
    }
  default:
    break;
  }
  localTemps[opInfo->dest_tmp] = destLocation;
}
VG_REGPARM(1) void executeQuadnaryShadowOp(QuadnaryOp_Info* opInfo){
  // The shadowing locations for the arguments and the destination.
  ShadowLocation *arg1Location, *arg2Location, *arg3Location, *arg4Location, *destLocation;
  // We're going to use a lot of switch statements here because there
  // are a lot of similar cases, and we don't want to repeat too much
  // code. Hopefully, it will be moderately well organized.
  switch(opInfo->op){
  case Iop_MAddF32:
  case Iop_MSubF32:
  case Iop_MAddF64:
  case Iop_MSubF64:
  case Iop_MAddF64r32:
  case Iop_MSubF64r32:
    {
      LocType argType;
      int (*mpfr_func)(mpfr_t, mpfr_t, mpfr_t, mpfr_t, mpfr_rnd_t);
      // Determine the type of the arguments
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
        break;
      }
      // Determine the mpfr shadow function
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
        break;
      }

      // Pull the shadow values for the arguments. If we don't already
      // have shadow values for these arguments, we'll generate fresh
      // ones from the runtime float values.
      arg2Location =
        getShadowLocation(opInfo->arg2_tmp, argType, opInfo->arg2_value);
      arg3Location =
        getShadowLocation(opInfo->arg3_tmp, argType, opInfo->arg3_value);
      arg4Location =
        getShadowLocation(opInfo->arg4_tmp, argType, opInfo->arg4_value);

      // Now we'll allocate memory for the shadowed result of this
      // operation.
      destLocation = mkShadowLocation(argType);

      // Set the destination shadow value to the result of a
      // high-precision shadowing operation.
      mpfr_func(destLocation->values[0].value, arg2Location->values[0].value,
                arg3Location->values[0].value, arg4Location->values[0].value,
                roundmodeIRtoMPFR(((IRRoundingMode*)opInfo->arg1_value)[0]));

      // Now, we'll evaluate the shadow value against the result value.
      switch(argType){
      case Lt_DoubleDouble:
        VG_(dmsg)("\
Wow, you're working with some really big floats. We can't evaluate the \n\
precision of those operations right now, but we're sure as hell \n\
keeping track of them.");
        break;
      case Lt_Double:
        evaluateOpError(&(destLocation->values[0]), ((double*)opInfo->dest_value)[0]);
        break;
      case Lt_Float:
        evaluateOpError(&(destLocation->values[0]), ((float*)opInfo->dest_value)[0]);
        break;
      default:
        break;
      }
    }
    break;
  default:
    break;
  }
  localTemps[opInfo->dest_tmp] = destLocation;
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

ShadowLocation* getShadowLocation(UWord tmp_num, LocType type, UWord* float_vals){
  // If we already have a shadow location here, just return it.
  ShadowLocation* location = localTemps[tmp_num];
  if (location != NULL) return location;

  // Otherwise we need to create a new one. How we do this will depend
  // on the expected type of the location, passed as "type".
  location = mkShadowLocation(type);
  switch(type){
    // Intialize the shadow values from the float_vals we were
    // given.
    //
    // The casts here are actually super important. Without casting
    // the array, we'll pull out the bits that make up the double as
    // basically an int, and then do a semantic conversion of that int
    // to a double, instead of reinterpreting the bytes as they should
    // have always been interpreted, as double bytes.
  case Lt_Doublex4:
    mpfr_set_d(location->values[3].value, ((double*)float_vals)[3], MPFR_RNDN);
    mpfr_set_d(location->values[2].value, ((double*)float_vals)[2], MPFR_RNDN);
  case Lt_Doublex2:
    mpfr_set_d(location->values[1].value, ((double*)float_vals)[1], MPFR_RNDN);
  case Lt_Double:
    mpfr_set_d(location->values[0].value, ((double*)float_vals)[0], MPFR_RNDN);
    return location;
  case Lt_Floatx8:
    mpfr_set_d(location->values[7].value, ((float*)float_vals)[7], MPFR_RNDN);
    mpfr_set_d(location->values[6].value, ((float*)float_vals)[6], MPFR_RNDN);
    mpfr_set_d(location->values[5].value, ((float*)float_vals)[5], MPFR_RNDN);
    mpfr_set_d(location->values[4].value, ((float*)float_vals)[4], MPFR_RNDN);
  case Lt_Floatx4:
    mpfr_set_d(location->values[3].value, ((float*)float_vals)[3], MPFR_RNDN);
    mpfr_set_d(location->values[2].value, ((float*)float_vals)[2], MPFR_RNDN);
  case Lt_Floatx2:
    mpfr_set_d(location->values[1].value, ((float*)float_vals)[1], MPFR_RNDN);
  case Lt_Float:
    mpfr_set_d(location->values[0].value, ((float*)float_vals)[0], MPFR_RNDN);
    return location;
  default:
    VG_(dmsg)("We don't know how to initialize shadow locations of that type!");
    return NULL;
  }
}
