#include "hg_opinfo.h"
#include "hg_ast.h"
#include "../include/hg_macros.h"
#include "pub_tool_debuginfo.h"

void getOpDebug_Info(Addr op_addr,
                     const HChar* plain_opname,
                     const HChar* symbol,
                     OpDebug_Info* result){
  result->op_addr = op_addr;
  result->plain_opname = plain_opname;
  result->symbol = symbol;
  if (VG_(get_filename_linenum)(op_addr,
                                &(result->src_filename),
                                NULL,
                                &(result->src_line)) &&
      VG_(get_fnname)(op_addr, &(result->fnname)))
    return;
  result->src_filename = NULL;
  result->src_line = 0;
  result->fnname = NULL;
}
Op_Info* mkOp_Info(SizeT arity, IROp op, Addr opAddr,
                   const HChar* name, const HChar* symbol){
  Op_Info* result;
  ALLOC(result, "hg.op_info.1", 1, sizeof(Op_Info));
  result->tag = Op_Branch;
  result->nargs = arity;
  result->op = op;
  getOpDebug_Info(opAddr, name, symbol, &(result->debuginfo));

  // Set the evalinfo up.

  // We want to give the evalinfo an initial max error of negative
  // one, instead of zero, for the corner case where the user wants to
  // report all operations. In that case, the user should set the
  // error threshold to zero. But ops only start getting tracked when
  // they weren't previously above the error threshold, and their new
  // value is. If the max error starts at zero, there will never come
  // that point, since the op starts out in a state we think it should
  // be tracked it, and we never know when to start tracking it. So
  // instead, set it to -1, so that any update of it to a non-negative
  // number will trigger a tracking.
  result->evalinfo.max_error = -1;

  return result;
}

Op_Info* mkLeafOp_Info(ShadowValue* val){
  Op_Info* result;
  OpASTNode* ast;
  ALLOC(result, "leaf op", 1, sizeof(Op_Info));
  ALLOC(ast, "leaf op ast", 1, sizeof(OpASTNode*));
  initOpLeafAST(ast, val);
  result->tag = Op_Leaf;
  result->ast = ast;
  return result;
}
