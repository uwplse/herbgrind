/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie             shadowop.h ---*/
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

#ifndef _SHADOWOP_H
#define _SHADOWOP_H

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_xarray.h"
#include "../value-shadowstate/shadowval.h"
#include "../op-shadowstate/shadowop-info.h"

VG_REGPARM(1) ShadowTemp* executeShadowOp(ShadowOpInfoInstance* instance);
ShadowTemp* getArg(int argIdx, IROp op, IRTemp argTemp);
ShadowValue* executeChannelShadowOp(ShadowOpInfo* opinfo,
                                    ShadowValue** args,
                                    double* computedArgs,
                                    double computedResult);

FloatBlocks numOpBlocks(IROp op);
#endif
