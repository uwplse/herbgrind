#include "pub_tool_redir.h"
#include "herbgrind.h"

double VG_WRAP_FUNCTION_ZU(libmZdso, sqrt)(double x);
double VG_WRAP_FUNCTION_ZU(libmZdso, sqrt)(double x){
  double result;
  OrigFn sqrt;
  HERBGRIND_END();
  CALL_FN_W_W(result, sqrt, x);
  HERBGRIND_BEGIN();
  return result;
}
