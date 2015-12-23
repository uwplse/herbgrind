#!/bin/env bash

# This adds in the functions for setting the memory functions.
sed -i -e '0,/^__MPFR_DECLSPEC int    mpfr_custom_get_kind   _MPFR_PROTO ((mpfr_srcptr));/s//&\n\n\/* The following abstraction of MPFRs memory functions is taken from the FpDebug project, by Florian Benz *\/\n\/* github.com\/fbenz\/FpDebug*\/\n\/* fbenz: additions, needed if MPFR is used within a Valgrind tool *\/\n__MPFR_DECLSPEC void mpfr_set_strlen_function _MPFR_PROTO((size_t (*)(const char*)));\n__MPFR_DECLSPEC void mpfr_set_strcpy_function _MPFR_PROTO((char* (*)(char*, const char*)));\n__MPFR_DECLSPEC void mpfr_set_memmove_function _MPFR_PROTO((void* (*)(void*, const void*, size_t)));\n__MPFR_DECLSPEC void mpfr_set_memcmp_function _MPFR_PROTO((int (*)(const void*, const void*, size_t)));\n__MPFR_DECLSPEC void mpfr_set_memset_function _MPFR_PROTO((void* (*)(void*, int, size_t)));\n\/* fbenz: this part should better be hidden in mpfe-impl.h\n   but it does not work at the moment *\/\n__MPFR_DECLSPEC size_t (*__mpfr_strlen_func) _MPFR_PROTO ((const char*));\n#define __MPFR_STRLEN(s) \\\n  ((*__mpfr_strlen_func) (s))\n__MPFR_DECLSPEC char* (*__mpfr_strcpy_func) _MPFR_PROTO ((char*, const char*));\n#define __MPFR_STRCPY(s1,s2) \\\n  ((*__mpfr_strcpy_func) (s1,s2))\n__MPFR_DECLSPEC void* (*__mpfr_memmove_func) _MPFR_PROTO ((void*, const void*, size_t));\n#define __MPFR_MEMMOVE(p1,p2,t) \\\n  ((*__mpfr_memmove_func) (p1,p2,t))\n__MPFR_DECLSPEC int (*__mpfr_memcmp_func) _MPFR_PROTO ((const void*, const void*, size_t));\n#define __MPFR_MEMCMP(p1,p2,t) \\\n  ((*__mpfr_memmove_func) (p1,p2,t))\n__MPFR_DECLSPEC void* (*__mpfr_memset_func) _MPFR_PROTO ((void*, int, size_t));\n#define __MPFR_MEMSET(p,i,t) \\\n  ((*__mpfr_memset_func) (p,i,t))\n/g' ../mpfr/src/mpfr.h

# This actually replaces the critical instances of memory functions
# with our own version.
sed -i -e 's/strlen/__MPFR_STRLEN/' -e 's/strcpy/__MPFR_STRCPY/' ../mpfr/src/get_str.c
sed -i -e 's/memcmp/__MPFR_MEMCMP/' ../mpfr/src/set_d.c

# Comment out uses of memory function in the default memory functions
# for mpfr. We include a space before the pattern matched so that
# strings like mpfr_default_free don't match.
sed -i \
    -e 's/ malloc.*(.*);/ 0;\/\/&/' \
    -e 's/ realloc.*(.*);/ 0;\/\/&/' \
    -e 's/ free.*(.*);/ ;\/\/&/' \
    ../mpfr/src/mpfr-gmp.c

# This part should do approximately the same thing for fprintf, but
# should still work when the fprintf is spread across multiple
# lines. Basically got this from:
# http://stackoverflow.com/questions/1164925/match-any-character-including-newlines-in-sed

# First, let's do a normal replace for fprintfs that don't span a
# line, so we can replace them with semicolons and not break if
# statements that branch to them.
sed -i -e 's/fprintf.*(.*);/;/' \
    ../mpfr/src/mpfr-gmp.c
# Then let's do the complex replace. This could still potentially
# break things, but on the version of mpfr I have, it doesn't.
sed -i \
    -e '/fprintf.*(/b fprintf
        b
        : fprintf
        s/ fprintf.*(.*);//
        t
        N
        b fprintf' \
    ../mpfr/src/mpfr-gmp.c

# Add the valgrind additions c file which implements the memory
# function setters to the project.

cp valgrind_additions.c ../mpfr/src/valgrind_additions.c
sed -i -e 's/libmpfr_la_SOURCES = .*\\/&\nvalgrind_additions.c \\/' \
    ../mpfr/src/Makefile.am
