
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie              hg_main.c ---*/
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


/*--------------------------------------------------------------------*/
/*--- FpDebug: Floating-point arithmetic debugger	 fd_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of FpDebug, a heavyweight Valgrind tool for
   detecting floating-point accuracy problems.

   Copyright (C) 2010-2011 Florian Benz 
      florianbenz1@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
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

/*
  Most of the following additions file is taken from the FpDebug project, by
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

#undef mpfr_set_strtol_function

/* These two are modeled after the FpDebug functions, but written by
   me (Alex Sanchez-Stern). */

void
mpfr_set_strtol_function (long int (*strtol_func) (const char*, char**, int))
{
	__mpfr_strtol_func = strtol_func;
}

#undef mpfr_set_isspace_function

void
mpfr_set_isspace_function (int (*isspace_func) (int))
{
	__mpfr_isspace_func = isspace_func;
}

