/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie              options.c ---*/
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

#include "options.h"

#include "pub_tool_options.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mpfr.h"

int running_depth = 1;
Bool always_on = False;

Bool print_in_blocks = False;
Bool print_out_blocks = False;
Bool print_block_boundries = False;
Bool print_run_blocks = False;
Bool print_run_instrs = False;

Bool print_temp_moves = False;
Bool print_value_moves = False;
Bool print_expr_refs = False;
Bool print_semantic_ops = False;
Bool print_conversions = False;
Bool print_types = False;
Bool print_allocs = False;
Bool print_errors = False;
Bool print_errors_long = False;
Bool print_inputs = False;
Bool print_expr_updates = False;
Bool print_flagged = False;
Bool print_compares = False;
Bool print_type_inference = False;
Bool print_inferred_types = False;
Bool print_statement_numbers = False;
Int longprint_len = 15;

Bool dont_ignore_pure_zeroes = False;
Bool expr_colors = False;
Bool print_object_files = False;
Bool print_subexpr_locations = False;
Bool output_mark_exprs = False;
Bool detailed_ranges = False;
Bool output_sexp = False;
Bool fpcore_ranges = True;
Bool sound_simplify = True;
Bool shortmark_all_exprs = False;
Bool mark_on_escape = True;
Bool compensation_detection = True;
Bool only_improvable = False;
Bool var_swallow = True;
Bool unsound_var_swallow = False;
Bool follow_real_execution = False;
Bool double_comparisons = False;
Bool flip_ranges = False;
Bool generalize_to_constant = True;

Bool no_exprs = False;
Bool no_influences = False;
Bool no_reals = False;
Bool use_ranges = True;
Bool dummy = False;

Int precision = 1000;
Int max_expr_block_depth = 5;
double error_threshold = 5.0;
Int max_influences = 20;
const char* output_filename = NULL;

// Called to process each command line option.
Bool hg_process_cmd_line_option(const HChar* arg){
  if VG_XACT_CLO(arg, "--print-in-blocks", print_in_blocks, True) {}
  else if VG_XACT_CLO(arg, "--print-out-blocks", print_out_blocks, True) {}
  else if VG_XACT_CLO(arg, "--print-block-boundries", print_block_boundries, True) {}
  else if VG_XACT_CLO(arg, "--print-run-blocks", print_run_blocks, True) {}
  else if VG_XACT_CLO(arg, "--print-run-instrs", print_run_instrs, True) {}
  else if VG_XACT_CLO(arg, "--print-temp-moves", print_temp_moves, True) {}
  else if VG_XACT_CLO(arg, "--print-value-moves", print_value_moves, True) {}
  else if VG_XACT_CLO(arg, "--print-expr-refs", print_expr_refs, True) {}
  else if VG_XACT_CLO(arg, "--print-semantic-ops", print_semantic_ops, True) {}
  else if VG_XACT_CLO(arg, "--print-conversions", print_conversions, True) {}
  else if VG_XACT_CLO(arg, "--print-types", print_types, True) {}
  else if VG_XACT_CLO(arg, "--print-allocs", print_allocs, True) {}
  else if VG_XACT_CLO(arg, "--print-errors", print_errors, True) {}
  else if VG_XACT_CLO(arg, "--print-errors-long", print_errors_long, True) {}
  else if VG_XACT_CLO(arg, "--print-inputs", print_inputs, True) {}
  else if VG_XACT_CLO(arg, "--print-expr-updates", print_expr_updates, True) {}
  else if VG_XACT_CLO(arg, "--print-flagged", print_flagged, True) {}
  else if VG_XACT_CLO(arg, "--print-object-files", print_object_files, True) {}
  else if VG_XACT_CLO(arg, "--print-compares", print_compares, True) {}
  else if VG_XACT_CLO(arg, "--print-type-inference", print_type_inference, True) {}
  else if VG_XACT_CLO(arg, "--print-inferred-types", print_inferred_types, True) {}
  else if VG_XACT_CLO(arg, "--print-statement-numbers", print_statement_numbers, True) {}
  else if VG_XACT_CLO(arg, "--output-subexpr-sources", print_subexpr_locations, True) {}
  else if VG_XACT_CLO(arg, "--dont-ignore-pure-zeroes", dont_ignore_pure_zeroes, True) {}
  else if VG_XACT_CLO(arg, "--no-sound-simplify", sound_simplify, False) {}
  else if VG_XACT_CLO(arg, "--no-var-swallow", var_swallow, False) {}
  else if VG_XACT_CLO(arg, "--unsound-var-swallow", unsound_var_swallow, True) {}
  else if VG_XACT_CLO(arg, "--no-generalize-to-constant", generalize_to_constant, False) {}
  else if VG_XACT_CLO(arg, "--follow-real-execution", follow_real_execution, False) {}
  else if VG_XACT_CLO(arg, "--double-comparisons", double_comparisons, True) {}
  else if VG_XACT_CLO(arg, "--flip-ranges", flip_ranges, True) {}
  else if VG_XACT_CLO(arg, "--expr-colors", expr_colors, True) {}
  else if VG_XACT_CLO(arg, "--output-mark-exprs", output_mark_exprs, True) {}
  else if VG_XACT_CLO(arg, "--detailed-ranges", detailed_ranges, True) {}
  else if VG_XACT_CLO(arg, "--shortmark-all-exprs", shortmark_all_exprs, True) {}
  else if VG_XACT_CLO(arg, "--only-improvable", only_improvable, True) {}
  else if VG_XACT_CLO(arg, "--start-off", running_depth, 0) {}
  else if VG_XACT_CLO(arg, "--always-on", always_on, True) {}
  else if VG_XACT_CLO(arg, "--output-sexp", output_sexp, True) {}
  else if VG_XACT_CLO(arg, "--no-fpcore-ranges", fpcore_ranges, False) {}
  else if VG_XACT_CLO(arg, "--no-mark-on-escape", mark_on_escape, False) {}
  else if VG_XACT_CLO(arg, "--no-compensation-detection", compensation_detection, False)
                       {}
  else if VG_XACT_CLO(arg, "--no-exprs", no_exprs, True) {}
  else if VG_XACT_CLO(arg, "--no-influences", no_influences, True) {}
  else if VG_XACT_CLO(arg, "--no-reals", no_reals, True) {}
  else if VG_XACT_CLO(arg, "--no-ranges", use_ranges, False) {}
  else if VG_XACT_CLO(arg, "--dummy", dummy, True) {}

  else if VG_BINT_CLO(arg, "--longprint-len", longprint_len, 1, 1000) {}
  else if VG_BINT_CLO(arg, "--precision", precision, MPFR_PREC_MIN, MPFR_PREC_MAX){}
  else if VG_BINT_CLO(arg, "--max-expr-block-depth", max_expr_block_depth, 1, 100) {}
  else if VG_DBL_CLO(arg, "--error-threshold", error_threshold) {}
  else if VG_BINT_CLO(arg, "--max-influences", max_influences, 1, 1000) {}
  else if VG_STR_CLO(arg, "--outfile", output_filename) {}
  else return False;
  return True;
}

void hg_print_usage(void){
  VG_(printf)("    --precision=value    "
              "Sets the mantissa size of the shadow \"real\" values. [1000]\n"
              "    --error-threshold=bits    "
              "The number of bits of error at which to start "
              "tracking a computation. [5.0]\n"
              "    --max-expr-block-depth=depth    "
              "Sets the maximum depth to which expressions will "
              "maintain proper equivalence information.\n"
              "    --outfile=name    "
              "The name of the file to write out. If no name is "
              "specified, will use <executable-name>.gh.\n"
              "    --output-sexp    "
              "Output in an easy-to-parse s-expression based format.\n"
              "    --output-subexpr-sources    "
              "Print the source locations of every subexpression that "
              "isn't in the same function as its parent.\n"
              "    --ignore-pure-zeroes    "
              "Ignores influences from values which are multiplied "
              "by zero.\n"
              "    --expr-colors    "
              "Use colors to mark the boundries between different "
              "functions in expressions.\n"
              "    --print-object-files    "
              "Anywhere an address would be printed, also print "
              "which object file it came from.\n"
              "    --output-mark-exprs     "
              "Print the full expressions for marks.\n"
              "    --detailed-ranges     "
              "Print more detailed information about the input ranges.\n"
              "    --no-sound-simplify    "
              "Don't simplify expressions in simple ways which don't affect "
              "their floating point behavior.\n"
              "    --shortmark-all-exprs    "
              "Mark every subexpression with its address, without any "
              "additional debug info. If --output-subexpr-source is "
              "also on, it overrides this for subexpressions that "
              "aren't in the same function as their parent.\n"
              "    --no-mark-on-escape    "
              "Turn off automatic marks on conversion to int and "
              "comparison.\n"
              "    --no-compensation-detection    "
              "Don't attempt to detect compensating terms and prune "
              "influences accordingly.\n"
              "    --follow-real-exeuction    "
              "Use high-precision values when converting to integers and booleans.\n"
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
              " --print-run-blocks "
              "Prints the addresses of each run block.\n"
              " --print-temp-moves "
              "Prints each shadow temp movement.\n"
              " --print-value-moves "
              "Prints each shadow value movement.\n"
              " --print-expr-refs "
              "Prints the reference counts for expressions when they are updated.\n"
              " --print-semantic-ops "
              "Prints each semantic op executed\n"
              " --print-conversions "
              "Prints each conversion op executed\n"
              " --print-types "
              "Prints some type inferences\n"
              " --print-allocs "
              "Prints for each major allocation.\n"
              " --print-errors "
              "Prints the error of the result of each operation.\n"
              " --print-errors-long "
              "Prints the error of the result of each operation, "
              "with long mpfr values.\n"
              " --print-inputs "
              "Prints the input values to each function.\n"
              " --print-expr-updates "
              "Prints the expressions that are derived for each "
              "operation.\n"
              " --print-object-files "
              "Print's the object file name along other debug "
              "info when printing addresses.\n"
              " --start-off "
              "Start's the analysis with the running flag set to off\n"
              " --always-on "
              "Ignore calls to HERBGRIND_END()\n"
              " --longprint-len=length "
              "How many digits of long real values to print.\n"
              " --print-flagged "
              "Print every operation that is flagged.\n");
}
