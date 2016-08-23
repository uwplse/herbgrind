
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie           hg_options.h ---*/
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

#ifndef _OPTIONS
#define _OPTIONS

#include "mpfr.h"

extern mpfr_prec_t precision;
extern double error_threshold;
extern Bool human_readable;

extern SizeT longprint_len;
extern Bool print_in_blocks;
extern Bool print_out_blocks;
extern Bool print_inputs;
extern Bool print_errors;
extern Bool print_errors_long;
extern Bool print_moves;
extern Bool print_counts;
extern Bool print_mallocs;
extern Bool print_expr_updates;
extern Bool report_exprs;

#endif
