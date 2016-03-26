#ifndef _OP_TRACKER
#define _OP_TRACKER

#include "../types/hg_opinfo.h"
// For allocating the array
#include "pub_tool_mallocfree.h"
#include "pub_tool_xarray.h"

// Basically handles a resizable vector of ops

// A growable vector of tracked operations.
extern XArray* tracked_ops;

// Add an operation to be tracked.
void startTrackingOp(Op_Info* opinfo);
// Remove a tracked operation
void clearTrackedOp(Op_Info* opinfo);
// A compare function which sorts Op_Info's in order of most max error
// first.
Int cmp_debuginfo(const void* a, const void* b);
// Removes all subexpressions of the given node from the tracker.
void recursivelyClearChildren(OpASTNode* node);

// Write out info about each tracked operation to the specified file.
void writeReport(const char* filename);

#endif
