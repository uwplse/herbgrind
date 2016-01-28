#include "pub_tool_clreq.h"
#include "herbgrind.h"
#include "hg_mathreplace.h"
#include "pub_tool_redir.h"

double VG_WRAP_FUNCTION_ZU(libmZdsoZd6, sqrt)(double x);
double VG_WRAP_FUNCTION_ZU(libmZdsoZd6, sqrt)(double x){
  double args[1];
  double result;
  args[0] = x;
  HERBGRIND_PERFORM_OP(OP_SQRT, &result, args);
  return result;
}
