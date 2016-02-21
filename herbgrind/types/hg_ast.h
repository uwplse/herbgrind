#ifndef _HG_AST
#define _HG_AST

#include "hg_ast.hh"
#include "hg_opinfo.hh"
#include "hg_shadowvals.hh"

#include "pub_tool_basics.h"

typedef struct _ValueASTNode {
  // The op which turned the args into the represented shadow value,
  // or NULL if this came from an input/constant.
  Op_Info* op;
  // Number of arguments
  SizeT nargs;
  // The arguments that created this AST, or NULL if this came from an
  // input/constant.
  ShadowValue** args;
} ValueASTNode;

typedef struct _OpASTNode {
  // Point back to the thing that holds us, for consistencies sake and
  // so that you can pass around just the AST when you really mean to
  // be computing on the AST.
  Op_Info* op;
  // The number of arguments.
  SizeT nargs;
  // The ops that resulted in our args
  OpASTNode** args;
} OpASTNode;

void initValueAST(ValueASTNode* out, Op_Info* opinfo, SizeT nargs);
void initOpAST(OpASTNode* out, Op_Info* holder, SizeT nargs);
void updateAST(Op_Info* op, ValueASTNode* result);
void generalizeAST(OpASTNode* opast, ValueASTNode* valast);
OpASTNode* convertValASTtoOpAST(ValueASTNode* valAST);
void freeOpASTargs(OpASTNode* opAST);
char* opASTtoString(OpASTNode* opAST);
#endif
