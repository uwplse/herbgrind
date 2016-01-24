#include "pub_tool_basics.h"
#include "pub_tool_redir.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_clreq.h"
#include "herbgrind.h"

double VG_WRAP_FUNCTION_ZU(libmZdsoZd6, sqrt)(double x);
double VG_WRAP_FUNCTION_ZU(libmZdsoZd6, sqrt)(double x){
  double result;
  OrigFn sqrt_fn;
  VALGRIND_GET_ORIG_FN(sqrt_fn);

  HERBGRIND_PRINT("Input value is: ");
  HERBGRIND_PRINT("klsjdflk");
  HERBGRIND_PRINT("\n");
  /* HERBGRIND_PRINTNUM(x); */

  HERBGRIND_END();
  CALL_FN_W_W(result, sqrt_fn, x);
  HERBGRIND_BEGIN();
  return result;
}
