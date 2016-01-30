#ifndef _MACROS
#define _MACROS

/* #define PRINTINBLOCKS */
/* #define PRINTOUTBLOCKS */
/* #define PRINTERRORS */
/* #define PRINTERRORSLONG */
/* #define PRINTMOVES */
/* #define LONGPRINT_LEN 30 */
/* #define PRINTMALLOCS */
/* #define CHECKMALLOC 0x0000000802d8c455 */

#ifdef PRINTMALLOCS
#define ALLOC(dest, name, num_elems, elem_size)         \
  dest = VG_(calloc)(name, num_elems, elem_size);       \
                    VG_(printf)("Allocing for %s at %p\n", name, dest);
#elif defined CHECKMALLOC
#define ALLOC(dest, name, num_elems, elem_size)         \
  dest = VG_(calloc)(name, num_elems, elem_size);       \
                    if (dest == CHECKMALLOC) VG_(printf)("Allocing for %s at %p.\n", name, dest);
#else
#define ALLOC(dest, name, num_elems, elem_size)  \
  dest = VG_(calloc)(name, num_elems, elem_size);
#endif

#endif
