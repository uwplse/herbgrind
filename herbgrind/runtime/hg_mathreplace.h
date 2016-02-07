#ifndef _HG_MATHREPLACE
#define _HG_MATHREPLACE

#include "../include/hg_mathreplace_funcs.h"
#include "../types/hg_shadowvals.h"

void performOp(OpType op, double* result, double* args);
ShadowLocation* getShadowLocMem(Addr addr, double float_arg);

#endif
