
#include <stdarg.h>
#include "herbgrind.h"

double sum(int nargs, ...){
  va_list args;
  va_start(args, nargs);
  double sum;
  for(int i = 0; i < nargs; ++i){
    double num = va_arg(args, double);
    HERBGRIND_MARK_IMPORTANT(num);
    sum += num;
  }
  return sum;
}
