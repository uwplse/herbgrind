/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie   mpfr_valgrind_glue.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017 Alex Sanchez-Stern

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

#ifndef _MPFR_VALGRIND_GLUE_H
#define _MPFR_VALGRIND_GLUE_H

#include <stddef.h>

void* gmp_alloc(size_t t);
void* gmp_realloc(void* p, size_t t1, size_t t2);
void gmp_free(void* p, size_t t);

size_t mpfr_strlen(const char* str);
long int mpfr_strtol(const char* str, char** endptr, int _base);

int mpfr_isspace(int c);
void* mpfr_memmove(void* dest, const void* src, size_t len);
int mpfr_memcmp(const void* ptr1, const void* ptr2, size_t len);
void* mpfr_memset(void* dest, int val, size_t size);

void setup_mpfr_valgrind_glue(void);
#endif
