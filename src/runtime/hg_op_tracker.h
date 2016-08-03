
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie        hg_op_tracker.h ---*/
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

#ifndef _OP_TRACKER
#define _OP_TRACKER

#include "../types/hg_opinfo.h"
// For allocating the array
#include "pub_tool_mallocfree.h"
#include "pub_tool_xarray.h"

// Basically handles a resizable vector of ops

// A growable vector of tracked operations.
extern XArray* tracked_ops;

// Add an operation to be tracked.
void startTrackingOp(Op_Info* opinfo);
// Remove a tracked operation
void clearTrackedOp(Op_Info* opinfo);
// A compare function which sorts Op_Info's in order of most max error
// first.
Int cmp_debuginfo(const void* a, const void* b);
// Removes all subexpressions of the given node from the tracker.
void recursivelyClearChildren(TeaNode* node);

// Write out info about each tracked operation to the specified file.
void writeReport(const char* filename);

#endif
