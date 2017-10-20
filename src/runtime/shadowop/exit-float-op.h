/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie        exit-float-op.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Herbgrind, a valgrind tool for diagnosing
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

#ifndef _EXIT_FLOAT_OP_H
#define _EXIT_FLOAT_OP_H

#include "pub_tool_basics.h"
#include "../value-shadowstate/shadowval.h"
#include "../op-shadowstate/marks.h"

VG_REGPARM(1) void checkCompare(ShadowCmpInfo* info);
VG_REGPARM(3) void checkConvert(ValueType argPrecision, IRTemp tmp,
                                Addr curAddr);

#endif
