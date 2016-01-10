#include "hg_runtime.h"

// Execute a shadow operation, storing the result of the high
// precision operation applied to the shadow value at the arg_tmp
// offset, into the shadow value at the dest_tmp offset. Depending on
// the value of op, we'll know which operation to apply. op will be
// mapped to operations by the enumeration at libvex_ir.h:415.
VG_REGPARM(3) void executeUnaryShadowOp(UnaryOp_Info* opInfo){
  ShadowLocation* argLocation;
  ShadowLocation* destLocation;

  switch(opInfo->op){
  case Iop_Sqrt64F0x2:
    // Pull the shadow location for the argument. If we don't already
    // have a shadow location for this argument, we'll generate a fresh
    // one from the runtime float value.
    argLocation = getShadowLocation(opInfo->arg_tmp, Lt_Doublex2, opInfo->arg_value);

    // Now we'll allocate memory for the shadowed result of this
    // operation, which is a 128-bit SIMD value. The high order
    // 64-bits are taken from the first argument, while the low order
    // 64-bits are the result of the operation.
    destLocation = mkShadowLocation(Lt_Doublex2);

    // Copy across the high order bits shadow value.
    destLocation->values[1] = argLocation->values[1];

    // Set the low order bits to the result of the sqrt, but in
    // higher precision.
    mpfr_init(destLocation->values[0].value);
    mpfr_sqrt(destLocation->values[0].value, argLocation->values[0].value, MPFR_RNDN);

    // Put the resulting location in the space for the dest temp.
    localTemps[opInfo->dest_tmp] = destLocation;

    // Now, we'll evaluate the low order shadow value against the low
    // order 64-bits of the result value.
    evaluateOpError(&(destLocation->values[0]), ((double*)opInfo->dest_value)[0]);

    break;
  default:
    break;
  }
}
VG_REGPARM(1) void executeBinaryShadowOp(BinaryOp_Info* opInfo){
  // The shadowing locations for the first argument, second argument,
  // and destination respectively.
  ShadowLocation* arg1Location;
  ShadowLocation* arg2Location;
  ShadowLocation* destLocation;
  switch(opInfo->op){
  case Iop_Add64F0x2:
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

    // Set the low order bits to the result of the addition, but in
    // higher precision.
    mpfr_init(destLocation->values[0].value);
    mpfr_add(destLocation->values[0].value, arg1Location->values[0].value,
             arg2Location->values[0].value, MPFR_RNDN);

    // Put the resulting location in the space for the dest temp.
    localTemps[opInfo->dest_tmp] = destLocation;

    // Now, we'll evaluate the low order shadow value against the low
    // order 64-bits of the result value.
    evaluateOpError(&(destLocation->values[0]), ((double*)opInfo->dest_value)[0]);
    break;
  case Iop_SetV128lo64:
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
    // 64-bits are taken from the second argument.
    destLocation = mkShadowLocation(Lt_Doublex2);

    // Copy across the high order bits shadow value from the first
    // argument.
    destLocation->values[1] = arg1Location->values[1];

    // Copy the low order bits shadow value from the second argument.
    destLocation->values[0] = arg2Location->values[0];

    // Put the resulting location in the space for the dest temp.
    localTemps[opInfo->dest_tmp] = destLocation;

    // This isn't really a "real" op in the math-y sense, so let's not
    // evaluate it's error.
    break;
  default:
    break;
  }
}
VG_REGPARM(3) void executeTernaryShadowOp(UWord op, UWord* args, UWord dest_tmp){
}
VG_REGPARM(3) void executeQuadnaryShadowOp(UWord op, UWord* args, UWord dest_tmp){
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
    mpfr_init_set_d(location->values[3].value, ((double*)float_vals)[3], MPFR_RNDN);
    mpfr_init_set_d(location->values[2].value, ((double*)float_vals)[2], MPFR_RNDN);
  case Lt_Doublex2:
    mpfr_init_set_d(location->values[1].value, ((double*)float_vals)[1], MPFR_RNDN);
  case Lt_Double:
    mpfr_init_set_d(location->values[0].value, ((double*)float_vals)[0], MPFR_RNDN);
    return location;
  case Lt_Floatx8:
    mpfr_init_set_d(location->values[7].value, ((float*)float_vals)[7], MPFR_RNDN);
    mpfr_init_set_d(location->values[6].value, ((float*)float_vals)[6], MPFR_RNDN);
    mpfr_init_set_d(location->values[5].value, ((float*)float_vals)[5], MPFR_RNDN);
    mpfr_init_set_d(location->values[4].value, ((float*)float_vals)[4], MPFR_RNDN);
  case Lt_Floatx4:
    mpfr_init_set_d(location->values[3].value, ((float*)float_vals)[3], MPFR_RNDN);
    mpfr_init_set_d(location->values[2].value, ((float*)float_vals)[2], MPFR_RNDN);
  case Lt_Floatx2:
    mpfr_init_set_d(location->values[1].value, ((float*)float_vals)[1], MPFR_RNDN);
  case Lt_Float:
    mpfr_init_set_d(location->values[0].value, ((float*)float_vals)[0], MPFR_RNDN);
    return location;
  default:
    VG_(dmsg)("We don't know how to initialize shadow locations of that type!");
    return NULL;
  }
}
