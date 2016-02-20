#include "hg_opinfo.h"
#include "../include/hg_macros.h"
#include "pub_tool_debuginfo.h"

void getOpDebug_Info(Addr op_addr,
                     const HChar* plain_opname,
                     const HChar* symbol,
                     OpDebug_Info* result){
  result->op_addr = op_addr;
  result->plain_opname = plain_opname;
  result->symbol = symbol;
  VG_(get_filename_linenum)(op_addr,
                            &(result->src_filename),
                            NULL,
                            &(result->src_line));
  VG_(get_fnname)(op_addr, &(result->fnname));
}
Op_Info* mkOp_Info(SizeT arity, IROp op, Addr opAddr,
                   const HChar* name, const HChar* symbol){
  Op_Info* result;
  ALLOC(result, "hg.op_info.1", 1, sizeof(Op_Info));
  result->nargs = arity;
  result->op = op;
  getOpDebug_Info(opAddr, name, symbol, &(result->debuginfo));
  return result;
}
