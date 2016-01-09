#include "pub_tool_redir.h"
#include "herbgrind.h"

double VG_WRAP_FUNCTION_ZU(sqrt, hg_sqrt)(double x){
  double result;
  HERBGRIND_END();
  result = sqrt(x);
  HERBGRIND_BEGIN();
  return result;
}
