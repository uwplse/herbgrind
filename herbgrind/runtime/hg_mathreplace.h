#ifndef _HG_MATHREPLACE
#define _HG_MATHREPLACE

#include "hg_mathreplace_funcs.h"
#include "hg_types.h"

void performOp(OpType op, double* result, double* args);
ShadowLocation* getShadowLocMem(Addr addr, double float_arg);

#endif
