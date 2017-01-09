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

#include "options.h"

#include "pub_tool_options.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mpfr.h"

Bool print_in_blocks = False;
Bool print_out_blocks = False;
Bool print_block_boundries = False;
Bool print_run_blocks = False;
Bool print_moves = False;
Int precision = 1000;

// Called to process each command line option.
Bool hg_process_cmd_line_option(const HChar* arg){
  if VG_XACT_CLO(arg, "--print-in-blocks", print_in_blocks, True) {}
  else if VG_XACT_CLO(arg, "--print-out-blocks", print_out_blocks, True) {}
  else if VG_XACT_CLO(arg, "--print-block-boundries", print_block_boundries, True) {}
  else if VG_XACT_CLO(arg, "--print-run-blocks", print_run_blocks, True) {}
  else if VG_XACT_CLO(arg, "--print-moves", print_moves, True) {}
  else if VG_BINT_CLO(arg, "--precision", precision, MPFR_PREC_MIN, MPFR_PREC_MAX){}
  else return False;
  return True;
}

void hg_print_usage(void){
  VG_(printf)("--precision=value "
              "Sets the mantissa size of the shadow \"real\" values.\n"
              );
}
void hg_print_debug_usage(void){
  VG_(printf)(" --print-in-blocks "
              "Prints the VEX superblocks that Herbgrind receives "
              "from Valgrind.\n"
              " --print-out-blocks "
              "Prints the instrumented VEX superblocks that Herbgrind "
              "returns to Valgrind.\n"
              " --print-block-boundries "
              "Prints +++++ between each executed block.\n"
              " --print-run-blocks.\n"
              "Prints the addresses of each run block.\n"
              " --print-moves.\n"
              "Prints each shadow value movement.\n");
}
