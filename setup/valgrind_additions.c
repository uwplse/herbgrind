/*
  The following additions file is taken from the FpDebug project, by
  Florian Benz.
  github.com/fbenz/FpDebug
 */
/*
fbenz: additions, needed if MPFR is used within a Valgrind tool
*/

#include "mpfr.h"
#include "mpfr-impl.h"

#undef mpfr_set_strlen_function

void
mpfr_set_strlen_function (size_t (*strlen_func) (const char*))
{
	__mpfr_strlen_func = strlen_func;
}

#undef mpfr_set_strcpy_function

void
mpfr_set_strcpy_function (char* (*strcpy_func) (char*, const char*))
{
	__mpfr_strcpy_func = strcpy_func;
}

#undef mpfr_set_memmove_function

void
mpfr_set_memmove_function (void* (*memmove_func) (void*, const void*, size_t))
{
	__mpfr_memmove_func = memmove_func;
}

#undef mpfr_set_memcmp_function

void
mpfr_set_memcmp_function (int (*memcmp_func) (const void*, const void*, size_t))
{
	__mpfr_memcmp_func = memcmp_func;
}

#undef mpfr_set_memset_function

void
mpfr_set_memset_function (void* (*memset_func) (void*, int, size_t))
{
	__mpfr_memset_func = memset_func;
}

