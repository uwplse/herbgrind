/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie              options.h ---*/
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

#ifndef _OPTIONS_H
#define _OPTIONS_H

#include "pub_tool_basics.h"

// Options for printing the VEX blocks that pass through
// Herbgrind. print_in_blocks prints the VEX super blocks that
// Herbgrind receives, and print_out_blocks prints the VEX blocks that
// Herbgrind passes back to Valgrind for execution.
extern Bool print_in_blocks;
extern Bool print_out_blocks;
extern Int precision;

Bool hg_process_cmd_line_option(const HChar* arg);

void hg_print_usage(void);
void hg_print_debug_usage(void);
#endif