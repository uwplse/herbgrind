/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie              options.h ---*/
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

#ifndef _OPTIONS_H
#define _OPTIONS_H

#include "pub_tool_basics.h"

// Options for printing the VEX blocks that pass through
// Herbgrind. print_in_blocks prints the VEX super blocks that
// Herbgrind receives, and print_out_blocks prints the VEX blocks that
// Herbgrind passes back to Valgrind for execution.
extern Bool print_in_blocks;
extern Bool print_out_blocks;
extern Bool print_block_boundries;
extern Bool print_run_blocks;

extern Bool print_temp_moves;
extern Bool print_value_moves;
extern Bool print_semantic_ops;
extern Bool print_conversions;
extern Bool print_types;
extern Bool print_allocs;
extern Bool print_errors;
extern Bool print_errors_long;
extern Bool print_expr_updates;
extern Bool print_flagged;

extern Bool running;
extern Bool always_on;

extern Bool ignore_pure_zeroes;
extern Bool expr_colors;
extern Bool print_object_files;
extern Bool print_subexpr_locations;
extern Bool output_mark_exprs;
extern Bool output_sexp;
extern Bool sound_simplify;

extern Int longprint_len;
extern Int precision;
extern double error_threshold;
extern const char* output_filename;

#define USE_MPFR

Bool hg_process_cmd_line_option(const HChar* arg);

void hg_print_usage(void);
void hg_print_debug_usage(void);
#define PRINT_VALUE_MOVES (print_value_moves && (running || always_on))
#define PRINT_IN_BLOCKS (print_in_blocks && (running || always_on))
#define PRINT_OUT_BLOCKS (print_out_blocks && (running || always_on))
#define PRINT_IN_VEXST (print_in_vexst && (running || always_on))
#define PRINT_RUN_BLOCKS (print_run_blocks && (running || always_on))
#define PRINT_BLOCK_BOUNDRIES (print_block_boundries && (running || always_on))
#define PRINT_TYPES (print_types && (running || always_on))
#endif
