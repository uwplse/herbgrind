
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie              hg_main.c ---*/
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

#include "include/hg_include.h"
#include "include/herbgrind.h"

// This file has all the functions and data structures that will be
// called by the instrumented program.
#include "runtime/hg_runtime.h"
// This file has the code to instrument each statement in the client
// program.
#include "hg_instrument.h"

#include "include/hg_options.h"
#include "pub_tool_options.h"

#include "pub_tool_clientstate.h"
// Pull in this header file so that we can call the valgrind version
// of printf.
#include "pub_tool_libcprint.h"
// Pull in this header file so that we can set the strlen, strcpy,
// memmove, memcmp, and memset functions of mpfr to their valgrind
// library equivalents.
#include "pub_tool_libcbase.h"
#include "mpfr.h"

// This is where the magic happens. This function gets called to
// instrument every superblock.
static
IRSB* hg_instrument ( VgCallbackClosure* closure,
                      IRSB* bb,
                      const VexGuestLayout* layout, 
                      const VexGuestExtents* vge,
                      const VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy )
{
  // Print out the input blocks if the appropriate flags have been
  // turned on.
  if (print_in_blocks && running){
    VG_(printf)("Instrumenting block:\n");
    printSuperBlock(bb);
  }

  // Let's do some instrumentation!

  // First, we'll set up a data structure to hold our instrumented IR.
  // We'll copy the typing environment, and the next block to jump to,
  // as well as some info about the exit jump, from the old superblock.
  IRSB* sbOut = deepCopyIRSBExceptStmts(bb);

  // Add instrumentation that initializes per-block state.
  startBlock(sbOut);

  // The address cooresponding to the statement we're currently
  // instrumenting.
  Addr cur_addr = 0x0;
  int opNum = 0;
  // Now, let's loop through these statements, and instrument them to
  // add our shadow values.
  for (int i = 0; i < bb->stmts_used; i++){
    IRStmt* st = bb->stmts[i];
    // Use the IMarks to get a cooresponding address for each
    // statement.
    if (st->tag == Ist_IMark){
      cur_addr = st->Ist.IMark.addr;
      opNum = 0;
    }
    // Only instrument statements after the preamble, not before the
    // first IMark.
    if (cur_addr)
      // Take a look at hg_instrument.c to see what's going on here.
      instrumentStatement(st, sbOut, cur_addr, opNum);
    else
      addStmtToIRSB(sbOut, st);
    if (isOp(st))
      opNum ++;
  }

  // Add instrumentation that cleans up per-block state.
  finalizeBlock(sbOut);

  // Print out the output blocks, if the appropriate flags have been
  // turned on.
  if (print_out_blocks && running){
    VG_(printf)("Instrumented into:\n");
    printSuperBlock(sbOut);
  }

  return sbOut;
}

// This handles client requests, the macros that client programs stick
// in to send messages to the tool.
static Bool hg_handle_client_request(ThreadId tid, UWord* arg, UWord* ret) {
  if (!VG_IS_TOOL_USERREQ('H', 'B', arg[0])){
    return False;
  }
  switch(arg[0]) {
  case VG_USERREQ__BEGIN:
    startHerbGrind();
    break;
  case VG_USERREQ__END:
    stopHerbGrind();
    break;
  case VG_USERREQ__PRINT:
    VG_(printf)("%s", (char*)arg[1]);
    break;
  case VG_USERREQ__PRINTNUM:
    VG_(printf)("%f\n", (double)arg[1]);
    break;
  case VG_USERREQ__PERFORM_OP:
    performOp((OpType)arg[1], (double*)arg[2], (double*)arg[3]);
    break;
  default:
    return False;
  }
  *ret = 0;
  return True;
}

mpfr_prec_t precision = 1000;
double error_threshold = 1.0;
Bool human_readable = True;
const HChar* outfile_path = NULL;
SizeT max_print_depth = 10;
SizeT max_expr_string_size = 256;

SizeT longprint_len = 15;
Bool print_in_blocks = False;
Bool print_out_blocks = False;
Bool print_inputs = False;
Bool print_errors = False;
Bool print_errors_long = False;
Bool print_moves = False;
Bool print_counts = False;
Bool print_mallocs = False;
Bool print_expr_updates = False;
Bool report_exprs = True;

// Called to process each command line option.
static Bool hg_process_cmd_line_option(const HChar* arg){
  if VG_BINT_CLO(arg, "--precision", precision, MPFR_PREC_MIN, MPFR_PREC_MAX) {}
  else if VG_DBL_CLO(arg, "--error-threshold", error_threshold) {}
  else if VG_XACT_CLO(arg, "--human", human_readable, True) {}
  else if VG_XACT_CLO(arg, "--machine", human_readable, False) {}
  else if VG_INT_CLO(arg, "--max-print-depth", max_print_depth) {}
  else if VG_INT_CLO(arg, "--max-expr-string-size", max_expr_string_size) {}
  else if VG_INT_CLO(arg, "--long-print-len", longprint_len) {}
  else if VG_XACT_CLO(arg, "--print-in-blocks", print_in_blocks, True) {}
  else if VG_XACT_CLO(arg, "--print-out-blocks", print_out_blocks, True) {}
  else if VG_XACT_CLO(arg, "--print-inputs", print_inputs, True) {}
  else if VG_XACT_CLO(arg, "--print-errors", print_errors, True) {}
  else if VG_XACT_CLO(arg, "--print-errors-long", print_errors_long, True) {}
  else if VG_XACT_CLO(arg, "--print-moves", print_moves, True) {}
  else if VG_XACT_CLO(arg, "--print-counts", print_counts, True) {}
  else if VG_XACT_CLO(arg, "--print-mallocs", print_mallocs, True) {}
  else if VG_XACT_CLO(arg, "--start-off", running, False) {}
  else if VG_STR_CLO(arg, "--outfile", outfile_path) {}
  else if VG_XACT_CLO(arg, "--print-expr-updates", print_expr_updates, True) {}
  else if VG_XACT_CLO(arg, "--report-ops", report_exprs, False) {}
  else return False;
  return True;
}

static void hg_print_usage(void){
  VG_(printf)(" --precision=<number> the precision of the shadow vales [1000]\n"
              " --error-threshold=<number> the threshold over which to report an erroneous operation (bits) [1]\n"
              " --human output in human-readable form (default)\n"
              " --machine output in machine-readable form\n"
              " --start-off turn off instrumentation until HERBGRIND_BEGIN() is called\n"
              " --outfile=<filepath> the output file path.\n"
              " --report-ops report individual operations instead of expressions\n"
              " --max-print-depth the maximum depth at which to print floating point expressions extracted from the binary [10]\n"
              " --max-expr-string-size the maximum size of an expression string. Expressions that need a string longer than this will be truncated [256]"
              );
}
static void hg_print_debug_usage(void){
  VG_(printf)(" --longprint-len=<number> the length of long-printed shadow values [15]\n"
              " --print-in-blocks print the incoming VEX blocks\n"
              " --print-out-blocks print the instrumented VEX blocks\n"
              " --print-inputs print the inputs to operations alongside the errors\n"
              " --print-errors print the errors of operations as they're executed\n"
              " --print-errors-long print the errors of the operations as they're executed, in long form. This means that we'll print the shadow values to <longprint-len> decimal digits, instead of rounding them\n"
              " --print-moves print every move of a shadow value/location\n"
              " --print-mallocs print every memory allocation\n"
              );
}

// This is called after the program exits, for cleanup and such.
static void hg_fini(Int exitcode){
  cleanup_runtime();

  // Write out the report
  HChar filename[100];
  if (outfile_path == NULL){
    VG_(snprintf)(filename, 100, "%s-errors.gh", VG_(args_the_exename));
    outfile_path = filename;
  }
  writeReport(outfile_path);
}
// This does any initialization that needs to be done after command
// line processing.
static void hg_post_clo_init(void){
   init_instrumentation();
   // Set up the data structures we'll need to keep track of our MPFR
   // shadow values.
   init_runtime();
}

// This is where we initialize everything
static void hg_pre_clo_init(void)
{
   VG_(details_name)            ("HerbGrind");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("a valgrind tool for Herbie");
   VG_(details_copyright_author)("");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );

   VG_(basic_tool_funcs)        (hg_post_clo_init,
                                 hg_instrument,
                                 hg_fini);

   VG_(needs_client_requests) (hg_handle_client_request);
   VG_(needs_command_line_options)(hg_process_cmd_line_option,
                                   hg_print_usage,
                                   hg_print_debug_usage);

   // Tell the gmp stuff to use valgrind c library instead of the
   // standard one for memory allocation and the like.
   mp_set_memory_functions(gmp_alloc, gmp_realloc, gmp_free);
   mpfr_set_strlen_function(mpfr_strlen);
   mpfr_set_strcpy_function(VG_(strcpy));
   mpfr_set_strtol_function(mpfr_strtol);
   mpfr_set_isspace_function(mpfr_isspace);
   mpfr_set_memmove_function(mpfr_memmove);
   mpfr_set_memcmp_function(mpfr_memcmp);
   mpfr_set_memset_function(mpfr_memset);
}

static void printSuperBlock(IRSB* superblock){
  for(int i = 0; i < superblock->stmts_used; i++){
    IRStmt* st = superblock->stmts[i];
    ppIRStmt(st);
    VG_(printf)("\n");
  }
  VG_(printf)("\n");
}

VG_DETERMINE_INTERFACE_VERSION(hg_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
