#ifndef _HG_EVALUATE
#define _HG_EVALUATE

#include "pub_tool_basics.h"
#include "pub_tool_libcprint.h"
#include "../types/hg_opinfo.h"
#include "../types/hg_shadowvals.h"

// Get the number of floating point values between x and y, including
// y.
unsigned long long ulpd(double x, double y);

// Evaluate the error of an executed operation.
void evaluateOpError(ShadowValue* shadowVal, double actualVal, OpDebug_Info* debuginfo);
// A helper that makes it easier to extract the value from some arg
// bytes with the type and index. This will reinterpret valbytes as an
// array of whatever datatype bytestype indicates, and then index into
// it with el_index to get the actual floating point value that we'll
// test against the shadow value.
void evaluateOpError_helper(ShadowValue* shadowVal, UWord* valbytes, LocType bytestype, int el_index, OpDebug_Info* debuginfo);

#endif
