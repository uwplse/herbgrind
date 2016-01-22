#ifndef _HG_HELPER
#define _HG_HELPER

#include "pub_tool_basics.h"

const char* IRExprTagString(IRExprTag tag);
const char* IRExprTagString(IRExprTag tag){
  switch(tag){
  case Iex_Binder:
    return "Iex_Binder";
  case Iex_Get:
    return "Iex_Get";
  case Iex_GetI:
    return "Iex_GetI";
  case Iex_RdTmp:
    return "Iex_RdTmp";
  case Iex_Qop:
    return "Iex_Qop";
  case Iex_Triop:
    return "Iex_Triop";
  case Iex_Binop:
    return "Iex_Binop";
  case Iex_Unop:
    return "Iex_Unop";
  case Iex_Load:
    return "Iex_Load";
  case Iex_Const:
    return "Iex_Const";
  case Iex_ITE:
    return "Iex_ITE";
  case Iex_CCall:
    return "Iex_CCall";
  case Iex_VECRET:
    return "Iex_VECRET";
  case Iex_BBPTR:
    return "Iex_BBPTR";
  }
  return "****";
}

#endif
