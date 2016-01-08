#ifndef _HG_EVALUATE
#define _HG_EVALUATE

#include "hg_types.h"
#include "pub_tool_libcprint.h"

// Get the number of floating point values between x and y, including
// y.
unsigned long long ulpd(double x, double y);

// Evaluate the error of an executed operation.
void evaluateOpError(ShadowValue* shadowVal, double actualVal);


#endif
