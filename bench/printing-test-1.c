#include <stdio.h>
#include <stdarg.h>

void simple_printf(const char* fmt, ...);
int main(){
  volatile double reallybig, reallysmall;
  reallybig = 1e16;
  reallysmall = -1e-20;
  double result = (reallybig + reallysmall) - reallybig;
  simple_printf("%f, %d, %f, %d, %f, %f, %f, %f, %f, %f, %f\n", result, 0xDEADBEEF, 4.0, 420, 6.0, 42.0, 56.0, 70.0, 8008.0, 3.14, 2.17);
  return 0;
}
void simple_printf(const char* fmt, ...)
{
  printf("fmt = %p\n", fmt);
  va_list args;
  va_start(args, fmt);

  while (*fmt != '\0') {
    if (*fmt == 'd') {
      int i = va_arg(args, int);
      printf("%d", i);
    } else if (*fmt == 'c') {
      // note automatic conversion to integral type
      int c = va_arg(args, int);
      printf("%c\n", c);
    } else if (*fmt == 'f') {
      double d = va_arg(args, double);
      printf("%f", d);
    } else if (*fmt == 'X') {
      long unsigned int val = va_arg(args, long unsigned int);
      printf("%lX", val);
    } else {
      printf("%c", *fmt);
    }
    ++fmt;
  }

  va_end(args);
}
