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
extern Bool print_mallocs;
extern Bool print_expr_updates;
extern Bool report_exprs;

#endif
