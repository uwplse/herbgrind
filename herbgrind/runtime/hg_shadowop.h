#ifndef _HG_SHADOWOP
#define _HG_SHADOWOP

#include "mpfr.h"
#include "hg_evaluate.h"
#include "hg_hiprec_ops.h"
#include "../hg_types.h"

// The functions that we'll insert into the program to execute shadow
// operations alongside the normal operations.
VG_REGPARM(1) void executeUnaryShadowOp(UnaryOp_Info* opInfo);
VG_REGPARM(1) void executeBinaryShadowOp(BinaryOp_Info* opInfo);
VG_REGPARM(1) void executeTernaryShadowOp(TernaryOp_Info* opInfo);
VG_REGPARM(1) void executeQuadnaryShadowOp(QuadnaryOp_Info* opInfo);

// Get a shadow value for the given temporary. If the temporary does
// not have a shadow value, create one with the given type using the
// given bytes to initialize the value.
ShadowLocation* getShadowLocation(UWord tmp_num, LocType type, UWord* float_vals);
// Convert IR rounding mode codes to MPFR rounding mode codes.
mpfr_rnd_t roundmodeIRtoMPFR(IRRoundingMode round);
#endif //_HG_SHADOWOP
