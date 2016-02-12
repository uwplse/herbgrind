#ifndef _MACROS
#define _MACROS

#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"

#include "hg_options.h"

#define ALLOC(dest, name, num_elems, elem_size)         \
  dest = VG_(calloc)(name, num_elems, elem_size);       \
                    if (print_mallocs) VG_(printf)("Allocing for %s at %p\n", name, dest);

#endif
