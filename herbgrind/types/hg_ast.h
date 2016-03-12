#ifndef _HG_AST
#define _HG_AST

#include "hg_ast.hh"
#include "hg_opinfo.hh"
#include "hg_shadowvals.hh"

#include "pub_tool_basics.h"

struct _ValueASTNode {
  // A circular reference the the value that this AST belongs to.
  ShadowValue* val;
  // The op which turned the args into the represented shadow value,
  // or NULL if this came from an input/constant.
  Op_Info* op;
  // Number of arguments (zero if this came from a constant).
  SizeT nargs;
  // The arguments that created this AST, or NULL if this came from an
  // input/constant.
  ValueASTNode** args;
};

typedef enum {
  Node_Leaf,
  Node_Branch,
} NodeType;

struct _OpASTNode {
  NodeType tag;
  union {
    struct {
      // The value which this leaf node was initialized with, or NULL
      // if it has been initialized with multiple different values.
      ShadowValue* val;
    } Leaf;
    struct {
      // Point back to the thing that holds us, for consistencies sake
      // and so that you can pass around just the AST when you really
      // mean to be computing on the AST.
      Op_Info* op;
      // The number of arguments.
      SizeT nargs;
      // The ops that resulted in our args
      OpASTNode** args;
    } Branch;
  } nd;
};

// Initialize a branch node ast for a shadow value that has just been
// produced as the result of an operation. You must set the args that
// produced this shadow value seperately after calling this
// function. Leaf nodes (shadow values that were produced from a
// constant or variable, and not as the result of a tracked operation)
// should not be initialized this way. Instead, the default null
// values will indicate that that node is a leaf node.
void initValueBranchAST(ShadowValue* val, Op_Info* opinfo, SizeT nargs);
// Initialize a leaf node ast for a shadow value that is being
// created.
void initValueLeafAST(ShadowValue* val);
// Cleanup a values AST for destroying the value
void cleanupValueAST(ShadowValue* val);
// Deep copy a value AST.
void copyValueAST(ShadowValue* src, ShadowValue* dest);
// Initialize a branch node ast for an operation. You must set the
// operation nodes from which the arguments came explicitly after
// calling this function.
void initOpBranchAST(OpASTNode* out, Op_Info* op, SizeT nargs);
// Initialize a leaf node ast, for a shadow value that was not
// produced as the result of an operation. This doesn't actually
// coorespond to any op, except the "initialize value" op.
void initOpLeafAST(OpASTNode* out, ShadowValue* val);
// Given an operation, and the ast of a value just produced by that
// operation, update the ast of the operation so that it is the most
// specific AST which is general enough so that: every value ast that
// has been produced by this operation could be created from the
// operation ast by replacing indeterminate leaf nodes by some
// subtree. Indeterminate leaf nodes are leaf nodes for which the val
// field is null.
void updateAST(Op_Info* op, ValueASTNode* result);
// Generalize an exising AST to meet the above criteria. This function
// is nearly identical to updateAST, but does not handle the case
// where the operation doesn't already have an AST, because this is
// the first time it has been invoked.
void generalizeAST(OpASTNode* opast, ValueASTNode* valast);
// A recursive tree walk which produces from a value AST an equivalent
// op AST, linking together ops instead of concrete values.
OpASTNode* convertValASTtoOpAST(ValueASTNode* valAST);
// Print out an op AST, currently in the stupidest possible way.
char* opASTtoString(OpASTNode* opAST);
#endif
