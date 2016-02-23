#ifndef _OP_TRACKER
#define _OP_TRACKER

#include "../types/hg_opinfo.h"
// For allocating the array
#include "pub_tool_mallocfree.h"

// Basically a resizable vector of ops

// A growable vector of tracked operations.
extern Op_Info** tracked_ops;
extern SizeT num_tracked_ops;

// Add an operation to be tracked.
void startTrackingOp(Op_Info* opinfo);
void clearTrackedOp(Op_Info* opinfo);
// Write out info about each tracked operation to the specified file.
void writeReport(const char* filename);

#endif
