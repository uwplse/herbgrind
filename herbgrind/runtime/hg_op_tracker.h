#ifndef _OP_TRACKER
#define _OP_TRACKER

#include "../types/hg_opinfo.h"
// For size_t
#include <stdlib.h>
// For allocating the array
#include "pub_tool_mallocfree.h"

// Basically a resizable vector of ops

extern Op_Info** tracked_ops;
extern size_t num_tracked_ops;

void startTrackingOp(Op_Info* opinfo);

#endif
