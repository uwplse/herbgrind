#include "hg_ast.h"
#include "hg_shadowvals.h"
#include "hg_opinfo.h"
#include "../include/hg_macros.h"
#include "../include/hg_options.h"

#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"

#define MAX_AST_STR_LEN 256

void initValueAST(ValueASTNode* out, Op_Info* opinfo, SizeT nargs){
  out->op = opinfo;
  out->nargs = nargs;
  ALLOC(out->args, "hg.val_ast_args", nargs, sizeof(ShadowValue));
}

void initOpAST(OpASTNode* out, Op_Info* holder, SizeT nargs){
  out->op = holder;
  out->nargs = nargs;
  ALLOC(out->args, "hg.val_ast_args", nargs, sizeof(OpASTNode));
}

void updateAST(Op_Info* op, ValueASTNode* trace_ast){
  if (op->ast == NULL){
    // The first time we see a value, the most specific AST that fits it
    // is exactly it's ast.
    op->ast = convertValASTtoOpAST(trace_ast);
  } else {
    // If we've already seen values, we'll want to generalize the AST
    // we have sufficiently to match the new value.
    generalizeAST(op->ast, trace_ast);
  }
  if (print_expr_updates){
    char* opASTString = opASTtoString(op->ast);
    VG_(printf)("Updating op ast to: %s\n", opASTString);
    VG_(free)(opASTString);
  }
}

void generalizeAST(OpASTNode* opast, ValueASTNode* valast){
  // If the op's ast stops here, we're done no matter what.
  if (opast->op == NULL) return;

  if (valast->op == NULL || valast->op != opast->op){
    // If the valast is a leaf node, or it continues but it doesn't
    // match the opast, cut off the opast here.
    opast->op = NULL;
    opast->nargs = 0;
    freeOpASTargs(opast);
    opast->args = NULL;
  } else if (opast->op != NULL) {
    // Otherwise, if they both continue and match, recurse on children.
    for(int i = 0; i < valast->nargs; ++i){
      generalizeAST(opast->args[i], valast->args[i]->ast);
    }
  }
}

OpASTNode* convertValASTtoOpAST(ValueASTNode* valAST){
  OpASTNode* result;
  ALLOC(result, "hg.op_ast", 1, sizeof(OpASTNode));
  result->op = valAST->op;
  result->nargs = valAST->nargs;
  // In the base case, valAST->nargs is zero.
  if (valAST->nargs != 0){
    ALLOC(result->args, "hg.op_ast_args", valAST->nargs, sizeof(OpASTNode*));
    for(int i = 0; i < valAST->nargs; ++i){
      result->args[i] = convertValASTtoOpAST(valAST->args[i]->ast);
    }
  }
  return result;
}

void freeOpASTargs(OpASTNode* opAST){
  for(int i = 0; i < opAST->nargs; ++i){
    freeOpASTargs(opAST->args[i]);
  }
  VG_(free)(opAST->args);
}

// This is a crude and wasteful function, but hopefully no one will
// notice.
char* opASTtoString(OpASTNode* opAST){
  char* buf;
  SizeT bufpos = 0;
  ALLOC(buf, "hg.ast_string", 1, sizeof(char)*MAX_AST_STR_LEN);

  if (opAST->op == NULL){
    VG_(snprintf)(buf, 2, "x");
  } else {
    bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, "(%s",
                            opAST->op->debuginfo.symbol);
    for (SizeT i = 0; i < opAST->nargs; ++i){
      char* subexpr = opASTtoString(opAST->args[i]);
      bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, " %s",
                              subexpr);
      VG_(free)(subexpr);
    }
    bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, ")");
  }
  return buf;
}
