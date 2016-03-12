#include "hg_ast.h"
#include "hg_shadowvals.h"
#include "hg_opinfo.h"
#include "../include/hg_macros.h"
#include "../include/hg_options.h"

#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"

#include <stdarg.h>

#define MAX_AST_STR_LEN 256

void initValueBranchAST(ShadowValue* val, Op_Info* opinfo,
                        SizeT nargs, ShadowValue* firstarg, ...){
  val->ast->val = val;
  val->ast->op = opinfo;

  val->ast->nargs = nargs;
  ALLOC(val->ast->args, "hg.val_ast_args", nargs, sizeof(ShadowValue*));

  // I guess this is how you handle variable arity in c. 
  va_list args;

  va_start(args, firstarg);
  for(int i = 0; i < nargs; ++i){
    val->ast->args[i] = va_arg(args, ShadowValue*)->ast;
  }
  va_end(args);

}

void initValueLeafAST(ShadowValue* val){
  val->ast->val = val;
}

void cleanupValueAST(ShadowValue* val){
  VG_(free)(val->ast->args);
  VG_(free)(val->ast);
}

void copyValueAST(ShadowValue* src, ShadowValue* dest){
  ALLOC(dest->ast, "hg.val_ast", 1, sizeof(ValueASTNode));
  dest->ast->val = dest;
  dest->ast->op = src->ast->op;
  dest->ast->nargs = src->ast->nargs;
  if (src->ast->nargs != 0){
    ALLOC(dest->ast->args, "hg.val_ast_args", src->ast->nargs, sizeof(ShadowValue*));
    for (int i = 0; i < src->ast->nargs; ++i){
      dest->ast->args[i] = src->ast->args[i];
    }
  }
}

void initOpBranchAST(OpASTNode* out, Op_Info* op, SizeT nargs){
  out->tag = Node_Branch;
  out->nd.Branch.op = op;
  out->nd.Branch.nargs = nargs;
  ALLOC(out->nd.Branch.args, "hg.val_ast_args", nargs, sizeof(OpASTNode*));
}

void initOpLeafAST(OpASTNode* out, ShadowValue* val){
  out->tag = Node_Leaf;
  out->nd.Leaf.val = val;
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
  // This doesn't (shouldn't) affect the functionality of this
  // function, but allows us to print AST's on update, mostly for
  // debugging purposes. Or like, you might just be into that, in
  // which case more power to you.
  if (print_expr_updates){
    char* opASTString = opASTtoString(op->ast);
    VG_(printf)("Updating op ast to: %s\n", opASTString);
    VG_(free)(opASTString);
  }
}

void generalizeAST(OpASTNode* opast, ValueASTNode* valast){
  if (opast->tag == Node_Leaf){
    // If we hit a value leaf, and it matches the one we've already
    // seen, then our best guess right now is that that is a constant
    // which doesn't change in this expression, so leave it in our AST.
    if (valast->val == NULL ||
        opast->nd.Leaf.val == NULL ||
        mpfr_get_d(valast->val->value, MPFR_RNDN)
        == mpfr_get_d(opast->nd.Leaf.val->value, MPFR_RNDN))
      return;
    // Otherwise, it's some sort of input that changes, so abstract it
    // into a variable by setting it's val field to null. TODO: we're
    // going to want to track somehow which inputs are the "same"
    // input.
    else { opast->nd.Leaf.val = NULL; return; }
  } else {
    // We're at a branch node.
    if (valast->op == NULL || valast->op != opast->nd.Branch.op){
      // If the valast is a leaf node, or it continues but it doesn't
      // match the opast, cut off the opast here, with a variable leaf
      // node (one where the shadow value is NULL, because we've seen
      // different values here).

      // Once we free the args array we malloc'ed, we can just
      // overwrite everything with new initial values, and things will
      // probably turn out fine.
      VG_(free)(opast->nd.Branch.args);
      initOpLeafAST(opast, NULL);
    } else if (opast->nd.Branch.op != NULL) {
      // Otherwise, if they both continue and match, recurse on children.
      for(int i = 0; i < valast->nargs; ++i){
        generalizeAST(opast->nd.Branch.args[i], valast->args[i]);
      }
    }
  }
}

OpASTNode* convertValASTtoOpAST(ValueASTNode* valAST){
  // First, check if we've already made an AST for the op that this
  // value came from. If so, just share that.
  if (valAST->op != NULL && valAST->op->ast != NULL) return valAST->op->ast;

  // Otherwise, we need to create a new AST node.
  OpASTNode* result;
  ALLOC(result, "hg.op_ast", 1, sizeof(OpASTNode));
  // If the value we're copying is a leaf...
  if (valAST->op == NULL){
    // Make a leaf node for the op ast.
    initOpLeafAST(result, valAST->val);
  } else {
    // Otherwise, start a branch node.
    initOpBranchAST(result, valAST->op, valAST->nargs);
    // Convert all of the children recursively. Usually they won't hit
    // this branch again, since we generally build subexpression AST's
    // before their parents, with the exception of leaf nodes since
    // they don't get build until a branch needs them. So, our
    // children should either already have an op ast somewhere, or
    // they are leaf nodes. This isn't necessarily an invariant I'm
    // commited to, so we should operate fine if it's not true, but
    // for clarity's sake that is what I currently expect.
    for (int i = 0; i < valAST->nargs; ++i){
      result->nd.Branch.args[i] = convertValASTtoOpAST(valAST->args[i]);
    }
  }
  return result;
}

// This is a crude and wasteful function, but hopefully no one will
// notice.
char* opASTtoString(OpASTNode* opAST){
  char* buf;
  // This is our "cursor" in the output string.
  SizeT bufpos = 0;
  ALLOC(buf, "hg.ast_string", 1, sizeof(char)*MAX_AST_STR_LEN);

  // If this is a leaf node...
  if (opAST->tag == Node_Leaf){
    // It could either be a constant or a variable. 
    if (opAST->nd.Leaf.val == NULL)
      // Variables we'll print as "x" for now, but soon we should give
      // them distinct names when we don't have a reason to believe
      // they come from the same place.
      VG_(snprintf)(buf, 2, "x");
    else
      // For constants we'll get the double value associated with the
      // constant, and print it.
      VG_(snprintf)(buf, MAX_AST_STR_LEN, "%f",
                    mpfr_get_d(opAST->nd.Leaf.val->value, MPFR_RNDN));
  } else {
    // Print the opening parenthesis and operator.
    bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, "(%s",
                            opAST->nd.Branch.op->debuginfo.symbol);
    // Recursively get the subexpression strings, and print them
    // preceded by a space.
    for (SizeT i = 0; i < opAST->nd.Branch.nargs; ++i){
      char* subexpr = opASTtoString(opAST->nd.Branch.args[i]);
      bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, " %s",
                              subexpr);
      VG_(free)(subexpr);
    }
    // Finally, print the closing parenthesis.
    bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, ")");
  }
  return buf;
}
