#include "pub_tool_clreq.h"
#include "herbgrind.h"
#include "hg_mathreplace_funcs.h"
#include "pub_tool_redir.h"

#define LIBM libmZdsoZd6

double VG_WRAP_FUNCTION_ZU(LIBM, sqrt)(double x);
double VG_WRAP_FUNCTION_ZU(LIBM, sqrt)(double x){
  double result;
  double args[1];
  args[0] = x;
  HERBGRIND_PERFORM_OP(OP_SQRT, &result, args);
  return result;
}

double VG_WRAP_FUNCTION_ZU(LIBM, pow)(double x, double y);
double VG_WRAP_FUNCTION_ZU(LIBM, pow)(double x, double y){
  double result;
  double args[2];
  args[0] = x;
  args[1] = y;
  HERBGRIND_PERFORM_OP(OP_POW, &result, args);
  return result;
}
