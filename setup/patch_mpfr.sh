#!/usr/bin/env bash

#
#--------------------------------------------------------------------*/
#--- HerbGrind: a valgrind tool for Herbie              hg_main.c ---*/
#--------------------------------------------------------------------*/
#
#
# This file is part of HerbGrind, a valgrind tool for diagnosing
# floating point accuracy problems in binary programs and extracting
# problematic expressions.
#
# Copyright (C) 2016 Alex Sanchez-Stern
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA.
#
# The GNU General Public License is contained in the file COPYING.
#

#=============================NOTE===================================
#   Part of the sed command in this file that modifies valgrind.h
#   comes from the FpDebug project, so I'm copying their copyright
#   notice here.
#====================================================================

#--------------------------------------------------------------------*/
#--- FpDebug: Floating-point arithmetic debugger        fd_main.c ---*/
#--------------------------------------------------------------------*/
#
#
# This file is part of FpDebug, a heavyweight Valgrind tool for
# detecting floating-point accuracy problems.
#
# Copyright (C) 2010-2011 Florian Benz 
#    florianbenz1@gmail.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA.
#
# The GNU General Public License is contained in the file COPYING.
#


MPFR_DIR=../deps/mpfr-$1/src

# check to see if we should use GNU sed on homebrew
if hash gsed > /dev/null 2>&1 ; then
  SED="gsed"
else
  SED="sed"
fi


# This adds in the functions for setting the memory functions. I
# really don't mean to obfuscate this, I just couldn't figure out how
# to get sed to play nicely with line breaks. Basically, this whole
# sed command just finds the right place in the file, and inserts a
# big block of definitions.
$SED -i -e '0,/^__MPFR_DECLSPEC int    mpfr_custom_get_kind   _MPFR_PROTO ((mpfr_srcptr));/s//&\n\n\/* The following abstraction of MPFRs memory functions is taken from the FpDebug project, by Florian Benz *\/\n\/* github.com\/fbenz\/FpDebug*\/\n\/* fbenz: additions, needed if MPFR is used within a Valgrind tool *\/\n__MPFR_DECLSPEC void mpfr_set_strlen_function _MPFR_PROTO((size_t (*)(const char*)));\n__MPFR_DECLSPEC void mpfr_set_strcpy_function _MPFR_PROTO((char* (*)(char*, const char*)));\n__MPFR_DECLSPEC void mpfr_set_memmove_function _MPFR_PROTO((void* (*)(void*, const void*, size_t)));\n__MPFR_DECLSPEC void mpfr_set_memcmp_function _MPFR_PROTO((int (*)(const void*, const void*, size_t)));\n__MPFR_DECLSPEC void mpfr_set_memset_function _MPFR_PROTO((void* (*)(void*, int, size_t)));\n\/* fbenz: this part should better be hidden in mpfe-impl.h\n   but it does not work at the moment *\/\n__MPFR_DECLSPEC size_t (*__mpfr_strlen_func) _MPFR_PROTO ((const char*));\n#define __MPFR_STRLEN(s) \\\n  ((*__mpfr_strlen_func) (s))\n__MPFR_DECLSPEC char* (*__mpfr_strcpy_func) _MPFR_PROTO ((char*, const char*));\n#define __MPFR_STRCPY(s1,s2) \\\n  ((*__mpfr_strcpy_func) (s1,s2))\n__MPFR_DECLSPEC void* (*__mpfr_memmove_func) _MPFR_PROTO ((void*, const void*, size_t));\n#define __MPFR_MEMMOVE(p1,p2,t) \\\n  ((*__mpfr_memmove_func) (p1,p2,t))\n__MPFR_DECLSPEC int (*__mpfr_memcmp_func) _MPFR_PROTO ((const void*, const void*, size_t));\n#define __MPFR_MEMCMP(p1,p2,t) \\\n  ((*__mpfr_memmove_func) (p1,p2,t))\n__MPFR_DECLSPEC void* (*__mpfr_memset_func) _MPFR_PROTO ((void*, int, size_t));\n#define __MPFR_MEMSET(p,i,t) \\\n  ((*__mpfr_memset_func) (p,i,t))\n\n\/* My own personal additions to these macros, from HerbGrind.*\/\n__MPFR_DECLSPEC void mpfr_set_strtol_function _MPFR_PROTO((long int (*)(const char*, char**, int)));\n__MPFR_DECLSPEC long int (*__mpfr_strtol_func) _MPFR_PROTO((const char*, char**, int));\n#define __MPFR_STRTOL(s,p,i) \\\n((*__mpfr_strtol_func) (s,p,i))\n__MPFR_DECLSPEC void mpfr_set_isspace_function _MPFR_PROTO((int (*)(int)));\n__MPFR_DECLSPEC int (*__mpfr_isspace_func) _MPFR_PROTO((int));\n#define __MPFR_ISSPACE(c) \\\n((*__mpfr_isspace_func) (c))/g' \
    $MPFR_DIR/mpfr.h

# This actually replaces the critical instances of memory functions
# with our own version.
$SED -i \
     -e 's/strlen/__MPFR_STRLEN/' \
     -e 's/strcpy/__MPFR_STRCPY/' \
     $MPFR_DIR/get_str.c \
     $MPFR_DIR/strtofr.c
$SED -i \
     -e 's/strtol/__MPFR_STRTOL/' \
     -e 's/isspace/__MPFR_ISSPACE/' \
     $MPFR_DIR/strtofr.c
$SED -i -e 's/memcmp/__MPFR_MEMCMP/' $MPFR_DIR/set_d.c

# Comment out uses of memory function in the default memory functions
# for mpfr. We include a space before the pattern matched so that
# strings like mpfr_default_free don't match.
$SED -i \
     -e 's/ malloc.*(.*);/ 0;\/\/&/' \
     -e 's/ realloc.*(.*);/ 0;\/\/&/' \
     -e 's/ free.*(.*);/ ;\/\/&/' \
     $MPFR_DIR/mpfr-gmp.c

# This part should do approximately the same thing for fprintf, but
# should still work when the fprintf is spread across multiple
# lines. Basically got this from:
# http://stackoverflow.com/questions/1164925/match-any-character-including-newlines-in-sed

# First, let's do a normal replace for fprintfs that don't span a
# line, so we can replace them with semicolons and not break if
# statements that branch to them.
$SED -i \
     -e 's/fprintf.*(.*);/;/' \
     -e 's/abort.*(.*);/;\/\/&/'\
     $MPFR_DIR/mpfr-gmp.c \
     $MPFR_DIR/abort_prec_max.c
# Then let's do the complex replace. This could still potentially
# break things, but on the version of mpfr I have, it doesn't.
$SED -i \
     -e '/fprintf.*(/b fprintf
         b
         : fprintf
         s/ fprintf.*(.*);//
         t
         N
         b fprintf' \
     $MPFR_DIR/mpfr-gmp.c

# Add the valgrind additions c file which implements the memory
# function setters to the project.

cp valgrind_additions.c $MPFR_DIR/valgrind_additions.c
$SED -i -e 's/libmpfr_la_SOURCES = .*\\/&\nvalgrind_additions.c \\/' \
     $MPFR_DIR/Makefile.am

# Add debugging symbols
echo "AM_CFLAGS = -g" >> $MPFR_DIR/Makefile.am
echo "AM_CFLAGS = -g" >> $MPFR_DIR/../Makefile.am

# Turn off locale specific parsing of floats, since it breaks things.
echo "AM_CFLAGS += -DMPFR_LCONV_DPTS=0" >> $MPFR_DIR/Makefile.am
