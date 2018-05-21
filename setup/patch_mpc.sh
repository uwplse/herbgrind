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
# Copyright (C) 2016-2017 Alex Sanchez-Stern
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

# check to see if we should use GNU sed on homebrew
if hash gsed > /dev/null 2>&1 ; then
  SED="gsed"
else
  SED="sed"
fi

$SED -i \
     -e '/#include "mpc-impl.h"/a #include "gmp-impl.h"' \
     -e 's/malloc/(*__gmp_allocate_func)/g' \
     -e 's/free\s*(\(.*\));/(*__gmp_free_func)(\1,1);/g' \
     ../deps/mpc-$1/src/mul.c

$SED -i \
     -e '/CPPFLAGS="-I$withval\/include $CPPFLAGS"/a CPPFLAGS="-I$withval $CPPFLAGS"' \
     ../deps/mpc-$1/configure.ac
