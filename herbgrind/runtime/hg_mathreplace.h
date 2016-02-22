#ifndef _HG_MATHREPLACE
#define _HG_MATHREPLACE

#include "../include/hg_mathreplace_funcs.h"
#include "../types/hg_shadowvals.h"

// This gets us a hash table data structure that's safe to use with
// valgrind, so we can map specific calls to wrapped functions to
// their meta-data about the operation.
#include "pub_tool_hashtable.h"

extern VgHashTable* callToOpInfoMap;

void performOp(OpType op, double* result, double* args);
ShadowLocation* getShadowLocMem(Addr addr, double float_arg);

VG_REGPARM(1) void updateLastAbiAddr(Addr addr);

#endif
