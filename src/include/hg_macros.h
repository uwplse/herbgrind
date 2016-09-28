
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            hg_macros.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016 Alex Sanchez-Stern

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#ifndef _MACROS
#define _MACROS

#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"

#include "hg_options.h"
#include <stdint.h>

extern intptr_t minptr;
extern intptr_t maxptr;

#define ALLOC(dest, name, num_elems, elem_size)         \
  dest = VG_(calloc)(name, num_elems, elem_size);       \
                    if (print_mallocs) VG_(printf)("Allocing for %s at %p\n", name, dest); /*\
                                                                                             // Uncomment this to get pointer checking to work.
                    if ((intptr_t)dest < minptr) minptr = (intptr_t)dest; \
                    if ((intptr_t)dest > maxptr) maxptr = (intptr_t)dest;*/
#define DEBUG(...) VG_(printf)(__VA_ARGS__)
#define CHECK_PTR(ptr) tl_assert2(ptr == NULL || (minptr <= (intptr_t)ptr && (intptr_t)ptr <= maxptr), "Bad pointer %p", ptr);
#endif
