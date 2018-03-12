/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie           floattypes.c ---*/
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

#include "floattypes.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "../helper/stack.h"
#include "../helper/ir-info.h"
#include "../options.h"

Stack* tsTypeEntries = NULL;

ValueType tempTypes[MAX_TEMPS][MAX_TEMP_SHADOWS];
TSTypeEntry* tsTypes[MAX_REGISTERS];
ShadowStatus tempShadowStatus[MAX_TEMPS];
ShadowStatus tsShadowStatus[MAX_REGISTERS];

void initTypeState(void){
  tsTypeEntries = mkStack();
}
void resetTypeState(void){
  VG_(memset)(tempTypes, 0, sizeof tempTypes);
  VG_(memset)(tempShadowStatus, 0, sizeof tempShadowStatus);
  VG_(memset)(tsShadowStatus, 0, sizeof tsShadowStatus);
  for(int i = 0; i < MAX_REGISTERS; ++i){
    while (tsTypes[i] != NULL){
      TSTypeEntry* nextEntry = tsTypes[i]->next;
      stack_push(tsTypeEntries, (StackNode*)tsTypes[i]);
      tsTypes[i] = nextEntry;
    }
  }
}
void cleanupTypeState(void){
}
ValueType* tempTypeArray(int idx){
  return tempTypes[idx];
}
ValueType tempBlockType(int idx, int blockIdx){
  return tempTypes[idx][blockIdx];
}
ValueType* exprTypeArray(IRExpr* expr){
  static ValueType typeArrays[5][8] = {
    {Vt_Unknown, Vt_Unknown, Vt_Unknown, Vt_Unknown,
     Vt_Unknown, Vt_Unknown, Vt_Unknown, Vt_Unknown},
    {Vt_NonFloat, Vt_NonFloat, Vt_NonFloat, Vt_NonFloat,
     Vt_NonFloat, Vt_NonFloat, Vt_NonFloat, Vt_NonFloat},
    {Vt_UnknownFloat, Vt_UnknownFloat, Vt_UnknownFloat, Vt_UnknownFloat,
     Vt_UnknownFloat, Vt_UnknownFloat, Vt_UnknownFloat, Vt_UnknownFloat},
    {Vt_Double, Vt_Double, Vt_Double, Vt_Double,
     Vt_Double, Vt_Double, Vt_Double, Vt_Double},
    {Vt_Single, Vt_Single, Vt_Single, Vt_Single,
     Vt_Single, Vt_Single, Vt_Single, Vt_Single}};
  switch(expr->tag){
  case Iex_RdTmp:
    return tempTypeArray(expr->Iex.RdTmp.tmp);
  case Iex_Const:
    return typeArrays[constType(expr->Iex.Const.con)];
  default:
    tl_assert(0);
    return Vt_Unknown;
  }
}
ValueType exprBlockType(IRExpr* expr, int blockIdx){
  switch(expr->tag){
  case Iex_RdTmp:
    return tempBlockType(expr->Iex.RdTmp.tmp, blockIdx);
  case Iex_Const:
    return constType(expr->Iex.Const.con);
  default:
    tl_assert(0);
    return Vt_Unknown;
  }
}
Bool refineTempBlockType(int tempIdx, int blockIdx, ValueType type){
  /* VG_(printf)("Refining type of t%d from %s with %s\n", */
  /*             tempIdx, typeName(tempTypes[tempIdx]), typeName(type)); */
  ValueType refinedType = typeMeet(type, tempBlockType(tempIdx, blockIdx));
  if (tempTypes[tempIdx][blockIdx] == refinedType){
    return False;
  } else {
    if (print_type_inference){
      VG_(printf)("Refining type of t%d from %s to %s\n",
                  tempIdx, typeName(tempTypes[tempIdx][blockIdx]), typeName(refinedType));
    }
    tempTypes[tempIdx][blockIdx] = refinedType;
    return True;
  }
}
Bool refineExprBlockType(IRExpr* expr, int blockIdx, ValueType type){
  if (expr->tag == Iex_RdTmp){
    return refineTempBlockType(expr->Iex.RdTmp.tmp, blockIdx, type);
  } else {
    return False;
  }
}
Bool someStaticallyFloat(IRTypeEnv* tyenv, IRExpr* expr){
  for(int i = 0; i < INT(exprSize(tyenv, expr)); ++i){
    if (staticallyFloat(expr, i)){
      return True;
    }
  }
  return False;
}
Bool staticallyFloatType(ValueType type){
  return typeJoin(type, Vt_UnknownFloat) == Vt_UnknownFloat;
}
Bool staticallyFloat(IRExpr* expr, int blockIdx){
  switch(expr->tag){
  case Iex_Const:
    return False;
  case Iex_RdTmp:
    return staticallyFloatType(tempBlockType(expr->Iex.RdTmp.tmp, blockIdx));
  default:
    VG_(printf)("Hey, what are you trying to pull here, man? "
                "You can't check the shadow of a non-trivial "
                "expression. It's got to be either a RdTmp or "
                "a Const, not %p ", expr);
    ppIRExpr(expr);
    tl_assert(0);
  }
}
Bool staticallyShadowed(IRExpr* expr){
  return expr->tag == Iex_RdTmp &&
    tempShadowStatus[expr->Iex.RdTmp.tmp] == Ss_Shadowed;
}
Bool someCanBeFloat(IRTypeEnv* typeEnv, IRExpr* expr){
  for(int i = 0; i < INT(exprSize(typeEnv, expr)); ++i){
    if (canBeFloat(typeEnv, expr, i)){
      return True;
    }
  }
  return False;
}
Bool canBeFloat(IRTypeEnv* typeEnv, IRExpr* expr, int blockIdx){
  if (!isFloatIRType(typeOfIRExpr(typeEnv, expr))){
    return False;
  } else if (expr->tag == Iex_Const){
    return True;
  } else if (tempBlockType(expr->Iex.RdTmp.tmp, blockIdx) == Vt_NonFloat){
    return False;
  } else {
    return True;
  }
}
Bool canStoreShadow(IRTypeEnv* typeEnv, IRExpr* expr){
  if (expr->tag == Iex_Const){
    return False;
  } else if (!isFloatIRType(typeOfIRExpr(typeEnv, expr))){
    return False;
  } else {
    return someCanBeFloat(typeEnv, expr);
  }
}

Bool canBeShadowed(IRTypeEnv* typeEnv, IRExpr* expr){
  return canStoreShadow(typeEnv, expr) &&
    tempShadowStatus[expr->Iex.RdTmp.tmp] != Ss_Unshadowed;
}

FloatBlocks tempSize(IRTypeEnv* tyenv, IRTemp tmp){
  return typeSize(typeOfIRTemp(tyenv, tmp));
}

FloatBlocks exprSize(IRTypeEnv* tyenv, IRExpr* expr){
  return typeSize(typeOfIRExpr(tyenv, expr));
}

FloatBlocks typeSize(IRType type){
  switch (type){
  case Ity_I32:
  case Ity_F32:
    return FB(1);
  case Ity_I64:
  case Ity_F64:
    return FB(2);
  case Ity_V128:
    return FB(4);
  case Ity_V256:
    return FB(8);
    // We're also going to include things that are too small to be
    // floats as size 1, so that we can set their single type slot to
    // be non-float easily.
  case Ity_I1:
  case Ity_I8:
    return FB(1);
  default:
    ppIRType(type);
    tl_assert(0);
    return FB(0);
  }
}
FloatBlocks loadConversionSize(IRLoadGOp conversion){
  switch(conversion){
  case ILGop_IdentV128:
    return FB(4);
  case ILGop_Ident64:
    return FB(2);
  case ILGop_Ident32:
    return FB(1);
  default:
    tl_assert(0);
    return FB(0);
  }
}

Bool tsAddrCanHaveShadow(Int tsAddr, int instrIdx){
  return tsType(tsAddr, instrIdx) != Vt_NonFloat;
}
Bool tsHasStaticShadow(Int tsAddr, int instrIdx){
  return tsShadowStatus[tsAddr] == Ss_Shadowed;
}

// The behavior of this function is this: if no type has been set for
// the thread state at this instrIdx, then we create a new entry for
// this instrIdx, which is active until the entry with the smallest
// instrIdx greater than this one. If the type HAS been set for this
// thread state location at this instrIdx, then we refine that
// type. If there is no way to meet the existing type set at this idx
// and the type given as a parameter, this will fail an assert.
Bool setTSType(int idx, int instrIdx, ValueType type){
  TSTypeEntry** nextTSEntry = &(tsTypes[idx]);
  while(*nextTSEntry != NULL && (*nextTSEntry)->instrIndexSet <= instrIdx){
    if ((*nextTSEntry)->instrIndexSet == instrIdx){
      ValueType newType = typeMeet(type, (*nextTSEntry)->type);
      if (newType == (*nextTSEntry)->type){
        return False;
      } else {
        (*nextTSEntry)->type = newType;
        return True;
      }
    }
    nextTSEntry = &((*nextTSEntry)->next);
  }
  TSTypeEntry* newTSEntry;
  if (stack_empty(tsTypeEntries)){
    newTSEntry = VG_(malloc)("TSTypeEntry", sizeof(TSTypeEntry));
  } else {
    newTSEntry = (void*)stack_pop(tsTypeEntries);
  }
  newTSEntry->type = type;
  newTSEntry->instrIndexSet = instrIdx;
  newTSEntry->next = *nextTSEntry;
  *nextTSEntry = newTSEntry;
  if (print_type_inference){
    VG_(printf)("Setting type of TS(%d) at instr %d to %s\n",
               idx, instrIdx, typeName(type));
  }
  return True;
}
Bool refineTSType(int idx, int instrIdx, ValueType type){
  if (tsTypes[idx] == NULL || tsTypes[idx]->instrIndexSet > instrIdx){
    if (print_type_inference){
      VG_(printf)("Setting initial type of TS(%d) to %s\n",
                  idx, typeName(type));
    }
    TSTypeEntry* newTSEntry;
    if (stack_empty(tsTypeEntries)){
      newTSEntry = VG_(malloc)("TSTypeEntry", sizeof(TSTypeEntry));
    } else {
      newTSEntry = (void*)stack_pop(tsTypeEntries);
    }
    newTSEntry->type = type;
    newTSEntry->instrIndexSet = 0;
    newTSEntry->next = NULL;
    tsTypes[idx] = newTSEntry;
    return True;
  }
  TSTypeEntry* nextTSEntry = tsTypes[idx];
  while(nextTSEntry->next != NULL && nextTSEntry->next->instrIndexSet < instrIdx){
    nextTSEntry = nextTSEntry->next;
  }
  ValueType refinedType = typeMeet(nextTSEntry->type, type);
  if (nextTSEntry->type == refinedType){
    return False;
  } else {
    if (print_type_inference){
      VG_(printf)("Refining type of TS(%d) at instr %d from %s to %s\n",
                  idx, instrIdx, typeName(nextTSEntry->type), typeName(refinedType));
    }
    nextTSEntry->type = refinedType;
    return True;
  }
}
ValueType tsType(Int tsAddr, int instrIdx){
  if (tsTypes[tsAddr] == NULL || tsTypes[tsAddr]->instrIndexSet > instrIdx){
    return Vt_Unknown;
  }
  TSTypeEntry* nextTSEntry = tsTypes[tsAddr];
  while(nextTSEntry->next != NULL && nextTSEntry->next->instrIndexSet <= instrIdx){
    nextTSEntry = nextTSEntry->next;
  }
  return nextTSEntry->type;
}
ValueType inferTSBlockType(int tsAddr, int instrIdx, FloatBlocks size){
  ValueType result = Vt_Unknown;
  for(int i = 0; i < INT(size); ++i){
    if (result == Vt_Double && i % 2 == 1) continue;
    ValueType floatSizeType = tsType(tsAddr + sizeof(float) * i, instrIdx);
    result = typeMeet(result, floatSizeType);
  }
  return result;
}

ValueType conversionArgPrecision(IROp op_code, int argIndex){
  tl_assert(isConversionOp(op_code));
  switch(op_code){
  case Iop_F64toF32:
    switch(argIndex){
    case 0:
      return Vt_NonFloat;
    case 1:
      return Vt_Double;
    default:
      tl_assert(0);
    }
  default:
    return opArgPrecision(op_code);
  }
}

ValueType opBlockArgPrecision(IROp op_code, int blockIdx){
  ValueType globalPrecision = opArgPrecision(op_code);
  if (globalPrecision == Vt_Double){
    if (blockIdx / 2 >= numSIMDOperands(op_code)){
      return Vt_Unknown;
    }
    if (blockIdx % 2 == 1){
      return Vt_NonFloat;
    }
    return Vt_Double;
  } else if (globalPrecision == Vt_Single) {
    if (blockIdx >= numSIMDOperands(op_code)){
      return Vt_Unknown;
    }
    return Vt_Single;
  } else {
    return globalPrecision;
  }
}
ValueType opArgPrecision(IROp op_code){
  if (!isFloatOp(op_code) && !isExitFloatOp(op_code) && !isSpecialOp(op_code)){
    return Vt_NonFloat;
  }
  switch((int)op_code){
    // Non-semantic ops have no need for this, since they will never
    // be constructing new shadow values, so we can just return
    // Vt_Unknown for them.
  case Iop_CmpF32:
  case Iop_CmpEQ32Fx2:
  case Iop_CmpGT32Fx2:
  case Iop_CmpGE32Fx2:
  case Iop_CmpEQ32Fx4:
  case Iop_CmpLT32Fx4:
  case Iop_CmpLE32Fx4:
  case Iop_CmpUN32Fx4:
  case Iop_CmpEQ32F0x4:
  case Iop_CmpLT32F0x4:
  case Iop_CmpLE32F0x4:
  case Iop_CmpUN32F0x4:
  case Iop_RecipEst32Fx4:
  case Iop_RSqrtEst32Fx4:
  case Iop_Abs32Fx4:
  case Iop_Neg32Fx4:
  case Iop_RecipEst64Fx2:
  case Iop_RSqrtEst64Fx2:
  case Iop_Abs64Fx2:
  case Iop_Neg64Fx2:
  case Iop_RecipEst32F0x4:
  case Iop_Sqrt32F0x4:
  case Iop_RSqrtEst32F0x4:
  case Iop_RSqrtEst32Fx2:
  case Iop_RecipEst32Fx2:
  case Iop_NegF32:
  case IEop_Neg32F0x4:
  case Iop_AbsF32:
  case Iop_RecipStep32Fx4:
  case Iop_RSqrtStep32Fx4:
  case Iop_Add32Fx2:
  case Iop_Sub32Fx2:
  case Iop_Add32F0x4:
  case Iop_Sub32F0x4:
  case Iop_Mul32F0x4:
  case Iop_Div32F0x4:
  case Iop_Max32F0x4:
  case Iop_Max32Fx2:
  case Iop_Max32Fx4:
  case Iop_Min32F0x4:
  case Iop_Min32Fx2:
  case Iop_Min32Fx4:
  case Iop_RecipStep32Fx2:
  case Iop_RSqrtStep32Fx2:
  case Iop_RecipStep64Fx2:
  case Iop_RSqrtStep64Fx2:
  case Iop_Neg32Fx2:
  case Iop_Abs32Fx2:
  case Iop_RecpExpF32:
  case Iop_SqrtF32:
  case Iop_Add32Fx8:
  case Iop_Sub32Fx8:
  case Iop_Mul32Fx8:
  case Iop_Div32Fx8:
  case Iop_Add32Fx4:
  case Iop_Sub32Fx4:
  case Iop_Mul32Fx4:
  case Iop_Div32Fx4:
  case Iop_MAddF32:
  case Iop_MSubF32:
  case Iop_AddF32:
  case Iop_SubF32:
  case Iop_MulF32:
  case Iop_DivF32:
  case Iop_AddF64r32:
  case Iop_SubF64r32:
  case Iop_MulF64r32:
  case Iop_DivF64r32:
  case Iop_MAddF64r32:
  case Iop_MSubF64r32:
  case Iop_ZeroHI96ofV128:
  case Iop_V128to32:
  case Iop_F32toF64:
  case Iop_RoundF32toInt:
  case Iop_SetV128lo32:
  case Iop_32Uto64:
  case Iop_32UtoV128:
  case Iop_F32toI32S:
  case Iop_F32toI64S:
  case Iop_F32toI32U:
  case Iop_F32toI64U:
    return Vt_Single;
  case Iop_CmpF64:
  case Iop_CmpEQ64Fx2:
  case Iop_CmpLT64Fx2:
  case Iop_CmpLE64Fx2:
  case Iop_CmpUN64Fx2:
  case Iop_CmpEQ64F0x2:
  case Iop_CmpLT64F0x2:
  case Iop_CmpLE64F0x2:
  case Iop_CmpUN64F0x2:
  case Iop_RSqrtEst5GoodF64:
  case Iop_NegF64:
  case IEop_Neg64F0x2:
  case Iop_AbsF64:
  case Iop_Sqrt64F0x2:
  case Iop_Sqrt64Fx2:
  case Iop_RecpExpF64:
  case Iop_SinF64:
  case Iop_CosF64:
  case Iop_TanF64:
  case Iop_2xm1F64:
  case Iop_SqrtF64:
  case Iop_Mul64F0x2:
  case Iop_Div64F0x2:
  /* case Iop_XorV128: */
  case Iop_Sub64F0x2:
  case Iop_Add64F0x2:
  case Iop_Add64Fx4:
  case Iop_Sub64Fx4:
  case Iop_Mul64Fx4:
  case Iop_Div64Fx4:
  case Iop_Add64Fx2:
  case Iop_Sub64Fx2:
  case Iop_Mul64Fx2:
  case Iop_Div64Fx2:
  case Iop_AtanF64:
  case Iop_Yl2xF64:
  case Iop_Yl2xp1F64:
  case Iop_ScaleF64:
  case Iop_AddF128:
  case Iop_SubF128:
  case Iop_MulF128:
  case Iop_DivF128:
  case Iop_AddF64:
  case Iop_SubF64:
  case Iop_MulF64:
  case Iop_DivF64:
  case Iop_MAddF64:
  case Iop_MSubF64:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_RoundF64toInt:
  case Iop_F64toF32:
  case Iop_F64HLtoF128:
  case Iop_Max64F0x2:
  case Iop_Max64Fx2:
  case Iop_Min64F0x2:
  case Iop_Min64Fx2:
  case Iop_64to32:
  case Iop_F64toI32S:
  case Iop_F64toI64S:
  case Iop_F64toI32U:
  case Iop_F64toI64U:
  case Iop_ReinterpF64asI64:
    return Vt_Double;
  case Iop_SetV128lo64:
  case Iop_64UtoV128:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_ZeroHI64ofV128:
  case Iop_64HLtoV128:
  case Iop_XorV128:
  case Iop_AndV128:
  case Iop_OrV128:
  case Iop_NotV128:
  case Iop_Shr64:
  case Iop_Shl64:
  case Iop_Sar64:
    return Vt_Unknown;
  case Iop_I64StoF64:
  case Iop_I32StoF64:
    return Vt_NonFloat;
  default:
    ppIROp_Extended(op_code);
    tl_assert2(0,
               "Op %d doesn't have an arg precision entry, "
               "but it's considered a float op.\n",op_code);
    return Vt_NonFloat;
  }
}
ValueType resultBlockPrecision(IROp op_code, int blockIdx){
  ValueType globalPrecision = resultPrecision(op_code);
  if (globalPrecision == Vt_Double){
    if (blockIdx / 2 >= numSIMDOperands(op_code)){
      return Vt_Unknown;
    }
    if (blockIdx % 2 == 1){
      return Vt_NonFloat;
    }
    return Vt_Double;
  } else if (globalPrecision == Vt_Single) {
    if (blockIdx >= numSIMDOperands(op_code)){
      return Vt_Unknown;
    }
    return Vt_Single;
  } else {
    return globalPrecision;
  }
}
ValueType resultPrecision(IROp op_code){
  if (!isFloatOp(op_code) && !isSpecialOp(op_code)){
    return Vt_NonFloat;
  }
  switch((int)op_code){
    // Non-semantic ops have no need for this, since they will never
    // be constructing new shadow values, so we can just return
    // Vt_NonFloat for them.
  case Iop_64to32:
  case Iop_RecipEst32Fx4:
  case Iop_RSqrtEst32Fx4:
  case Iop_Abs32Fx4:
  case Iop_Neg32Fx4:
  case IEop_Neg32F0x4:
  case Iop_RecipEst32F0x4:
  case Iop_Sqrt32F0x4:
  case Iop_RSqrtEst32F0x4:
  case Iop_RSqrtEst32Fx2:
  case Iop_RecipEst32Fx2:
  case Iop_NegF32:
  case Iop_AbsF32:
  case Iop_RecipStep32Fx4:
  case Iop_RSqrtStep32Fx4:
  case Iop_Add32Fx2:
  case Iop_Sub32Fx2:
  case Iop_Add32F0x4:
  case Iop_Sub32F0x4:
  case Iop_Mul32F0x4:
  case Iop_Div32F0x4:
  case Iop_Max32F0x4:
  case Iop_Max32Fx2:
  case Iop_Max32Fx4:
  case Iop_Min32F0x4:
  case Iop_Min32Fx2:
  case Iop_Min32Fx4:
  case Iop_RecipStep32Fx2:
  case Iop_RSqrtStep32Fx2:
  case Iop_RecipStep64Fx2:
  case Iop_RSqrtStep64Fx2:
  case Iop_Neg32Fx2:
  case Iop_Abs32Fx2:
  case Iop_RecpExpF32:
  case Iop_SqrtF32:
  case Iop_Add32Fx8:
  case Iop_Sub32Fx8:
  case Iop_Mul32Fx8:
  case Iop_Div32Fx8:
  case Iop_Add32Fx4:
  case Iop_Sub32Fx4:
  case Iop_Mul32Fx4:
  case Iop_Div32Fx4:
  case Iop_MAddF32:
  case Iop_MSubF32:
  case Iop_AddF32:
  case Iop_SubF32:
  case Iop_MulF32:
  case Iop_DivF32:
  case Iop_AddF64r32:
  case Iop_SubF64r32:
  case Iop_MulF64r32:
  case Iop_DivF64r32:
  case Iop_MAddF64r32:
  case Iop_MSubF64r32:
  case Iop_ZeroHI96ofV128:
  case Iop_V128to32:
  case Iop_32UtoV128:
  case Iop_RoundF32toInt:
  case Iop_SetV128lo32:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_F64toF32:
    return Vt_Single;
  case Iop_32Uto64:
  case Iop_RSqrtEst5GoodF64:
  case Iop_RecipEst64Fx2:
  case Iop_RSqrtEst64Fx2:
  case Iop_Abs64Fx2:
  case Iop_Neg64Fx2:
  case IEop_Neg64F0x2:
  case Iop_NegF64:
  case Iop_AbsF64:
  case Iop_Sqrt64F0x2:
  case Iop_Sqrt64Fx2:
  case Iop_RecpExpF64:
  case Iop_SinF64:
  case Iop_CosF64:
  case Iop_TanF64:
  case Iop_2xm1F64:
  case Iop_SqrtF64:
  case Iop_Mul64F0x2:
  case Iop_Div64F0x2:
  /* case Iop_XorV128: */
  case Iop_Sub64F0x2:
  case Iop_Add64F0x2:
  case Iop_Add64Fx4:
  case Iop_Sub64Fx4:
  case Iop_Mul64Fx4:
  case Iop_Div64Fx4:
  case Iop_Add64Fx2:
  case Iop_Sub64Fx2:
  case Iop_Mul64Fx2:
  case Iop_Div64Fx2:
  case Iop_Max64F0x2:
  case Iop_Max64Fx2:
  case Iop_Min64F0x2:
  case Iop_Min64Fx2:
  case Iop_AtanF64:
  case Iop_Yl2xF64:
  case Iop_Yl2xp1F64:
  case Iop_ScaleF64:
  case Iop_AddF128:
  case Iop_SubF128:
  case Iop_MulF128:
  case Iop_DivF128:
  case Iop_AddF64:
  case Iop_SubF64:
  case Iop_MulF64:
  case Iop_DivF64:
  case Iop_MAddF64:
  case Iop_MSubF64:
  case Iop_ZeroHI64ofV128:
  case Iop_V128to64:
  case Iop_V128HIto64:
  case Iop_RoundF64toF64_NEAREST:
  case Iop_RoundF64toF64_NegINF:
  case Iop_RoundF64toF64_PosINF:
  case Iop_RoundF64toF64_ZERO:
  case Iop_F128HItoF64:
  case Iop_F128LOtoF64:
  case Iop_RoundF64toInt:
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_F32toF64:
  case Iop_I32StoF64:
  case Iop_I64StoF64:
  case Iop_ReinterpF64asI64:
    return Vt_Double;
  case Iop_SetV128lo64:
  case Iop_64UtoV128:
  case Iop_XorV128:
  case Iop_AndV128:
  case Iop_OrV128:
  case Iop_NotV128:
  case Iop_Shr64:
  case Iop_Shl64:
  case Iop_Sar64:
    return Vt_Unknown;
  default:
    tl_assert(0);
    return Vt_NonFloat;
  }
}

int isFloatIRType(IRType type){
  return type == Ity_I32 || type == Ity_I64
    || type == Ity_F32 || type == Ity_F64
    || type == Ity_V128 || type == Ity_V256;
}

int isFloat(IRTypeEnv* env, IRTemp temp){
  IRType type = typeOfIRTemp(env, temp);
  return isFloatIRType(type);
}

void ppValueType(ValueType type){
  switch(type){
  case Vt_Unknown:
    VG_(printf)("Vt_Unknown");
    break;
  case Vt_NonFloat:
    VG_(printf)("Vt_NonFloat");
    break;
  case Vt_UnknownFloat:
    VG_(printf)("Vt_UnknownFloat");
    break;
  case Vt_Single:
    VG_(printf)("Vt_Single");
    break;
  case Vt_Double:
    VG_(printf)("Vt_Double");
    break;
  }
}
// This function does type inference for the super block. The new type
// inference system infers both forwards and backwards.
void inferTypes(IRSB* sbIn){
  // To calculate a fixpoint on forward and backwards type inference,
  // we'll use this dirty flag. It is set to zero at the beginning of
  // every iteration, and only set to one if something
  // changes. Therefore, the while loop will terminate when an
  // iteration has completed which doesn't change anything.
  int dirty = 1;
  int pass_num = 0;
  int direction = 1;
  while(dirty){
    pass_num++;
    if (print_type_inference){
      VG_(printf)("Making type pass %d\n"
                  "==================\n"
                  "\n",
                  pass_num);
    }
    dirty = 0;
    // We make forward passes through the instructions, building type
    // information.
    //
    // Temporary type information is simple: every temporary has
    // exactly one type throughout the lifetime of the
    // superblock.
    //
    // Thread state type information is slightly more complicated,
    // because thread state locations don't always have a single type
    // throughout the lifetime of the superblock. A particular
    // location could have integers in it at one point, and floating
    // point numbers in it at another. So instead of storing a single
    // type for each thread state locations, we're going to store a
    // time-series of types. This is represented as a linked list of
    // entries where the type changes, due to an assignment. All the
    // thread state type accessing and setting functions will
    // therefore take the instruction index, and will use it to update
    // this data structure.
    for(int instrIdx = direction == 1 ? 0 : sbIn->stmts_used - 1;
        direction == 1 ? instrIdx < sbIn->stmts_used : instrIdx >= 0;
        instrIdx += direction){
      IRStmt* stmt = sbIn->stmts[instrIdx];
      switch(stmt->tag){
        // These statements don't really do much, so we can ignore
        // them for type inference, although we're keeping the cases
        // here to be exhaustive.
      case Ist_NoOp:
      case Ist_IMark:
      case Ist_MBE:
      case Ist_Exit:
      case Ist_AbiHint:
        break;
        // The first non-trivial instruction for inference. PUTs break
        // down into two major cases: either they are putting a
        // constant into thread state, or they are moving between a
        // temporary and thread state.
      case Ist_Put:
        {
          IRExpr* sourceData = stmt->Ist.Put.data;
          int destLocation = stmt->Ist.Put.offset;
          switch(sourceData->tag){
          case Iex_Const:
            {
              ValueType srcType = constType(sourceData->Iex.Const.con);
              FloatBlocks numBlocks = exprSize(sbIn->tyenv, sourceData);
              for(int i = 0; i < INT(numBlocks); ++i){
                if (srcType == Vt_Double && i % 2 == 1){
                  dirty |= setTSType(destLocation + i * sizeof(float),
                                     instrIdx, Vt_NonFloat);
                } else {
                  dirty |= setTSType(destLocation + i * sizeof(float),
                                     instrIdx, srcType);
                }
              }
            }
            break;
            // The temporary case gets a lot more interesting. We'll
            // want to propagate information both ways: if we know
            // something about the temporary, but not the thread
            // state, we'll want to propagate that information FORWARD
            // to the thread state; if we know something about the
            // thread state, but not the temporary, we want to
            // propagate that information BACKWARD to the
            // temporary. We also might not know anything useful right
            // now, but we could figure out more later as we look at
            // more of the block and propagate information around.
          case Iex_RdTmp:
            {
              FloatBlocks numBlocks = exprSize(sbIn->tyenv, sourceData);
              IRTemp srcTemp = sourceData->Iex.RdTmp.tmp;
              for(int i = 0; i < INT(numBlocks); ++i){
                int tsDest = destLocation + i * sizeof(float);
                dirty |= setTSType(tsDest, instrIdx, tempBlockType(srcTemp, i));
                dirty |= refineTempBlockType(srcTemp, i, tsType(tsDest, instrIdx));
              }
            }
            break;
          default:
            tl_assert(0);
            return;
          }
        }
        break;
      case Ist_PutI:
        // Because we don't know where in the fixed region of the array this
        // put will affect, we have to mark the whole array as unknown
        // statically. Well, except we know they are making well-aligned
        // rights because of how putI is calculated, so if we know they are
        // writing doubles, then we know there are no new floats in the odd
        // offsets.
        //
        // We'll skip backwards propagation for this one, because it's
        // pretty uncommon, and you'd need to be pretty conservative,
        // so it's not clear that it'd be a win.
        {
          IRExpr* sourceData = stmt->Ist.PutI.details->data;
          switch(sourceData->tag){
          case Iex_Const:
            for(int i = 0;
                i < stmt->Ist.PutI.details->descr->nElems *
                  sizeofIRType(stmt->Ist.PutI.details->descr->elemTy);
                i+=sizeof(float)){
              int destLocation =
                stmt->Ist.PutI.details->descr->base + i;
              dirty |= setTSType(destLocation, instrIdx,
                                 typeJoin(constType(sourceData->Iex.Const.con),
                                          tsType(destLocation, instrIdx)));
            }
            break;
          case Iex_RdTmp:
            for(int i = 0;
                i < stmt->Ist.PutI.details->descr->nElems *
                  sizeofIRType(stmt->Ist.PutI.details->descr->elemTy);
                i+=sizeof(float)){
              int destLocation =
                stmt->Ist.PutI.details->descr->base + i;
              ValueType srcType = tempBlockType(sourceData->Iex.RdTmp.tmp,
                                                i / sizeof(float));
              dirty |= setTSType(destLocation, instrIdx,
                                 typeJoin(srcType, tsType(destLocation, instrIdx)));
            }
            break;
          default:
            tl_assert(0);
            break;
          }
        }
        break;
      case Ist_WrTmp:
        {
          IRExpr* expr = stmt->Ist.WrTmp.data;
          int destTemp = stmt->Ist.WrTmp.tmp;
          switch(expr->tag){
          case Iex_Get:
            {
              int sourceOffset = expr->Iex.Get.offset;
              switch(expr->Iex.Get.ty){
              case Ity_F32:
                tl_assert(INT(tempSize(sbIn->tyenv, destTemp)) == 1);
                dirty |= refineTSType(sourceOffset, instrIdx, Vt_Single);
                dirty |= refineTempBlockType(destTemp, 0, Vt_Single);
                break;
              case Ity_F64:
                tl_assert(INT(tempSize(sbIn->tyenv, destTemp)) == 2);
                dirty |= refineTSType(sourceOffset, instrIdx, Vt_Double);
                dirty |= refineTSType(sourceOffset + sizeof(float),
                                      instrIdx, Vt_NonFloat);
                dirty |= refineTempBlockType(destTemp, 0, Vt_Double);
                dirty |= refineTempBlockType(destTemp, 1, Vt_NonFloat);
                break;
              case Ity_I32:
              case Ity_I64:
              case Ity_V128:
              case Ity_V256:
                for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                  int tsSrc = sourceOffset + i * sizeof(float);
                  dirty |= refineTSType(tsSrc, instrIdx, tempBlockType(destTemp, i));
                  dirty |= refineTempBlockType(destTemp, i, tsType(tsSrc, instrIdx));
                }
                break;
              case Ity_I1:
              case Ity_I8:
              case Ity_I16:
                dirty |= refineTSType(sourceOffset, instrIdx, Vt_NonFloat);
                dirty |= refineTempBlockType(destTemp, 0, Vt_NonFloat);
                break;
              default:
                tl_assert(0);
                break;
              }
            }
            break;
          case Iex_GetI:
            // Ugh lets not even try to get this one right for now,
            // these are pretty rare.
            break;
          case Iex_RdTmp:
            {
              int sourceTemp = expr->Iex.RdTmp.tmp;
              tl_assert(INT(tempSize(sbIn->tyenv, destTemp)) ==
                        INT(tempSize(sbIn->tyenv, sourceTemp)));
              for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                dirty |= refineTempBlockType(sourceTemp, i, tempBlockType(destTemp, i));
                dirty |= refineTempBlockType(destTemp, i, tempBlockType(sourceTemp, i));
              }
            }
            break;
          case Iex_ITE:
            {
              IRExpr* source1 = expr->Iex.ITE.iftrue;
              IRExpr* source2 = expr->Iex.ITE.iffalse;
              int source1Temp, source2Temp;
              switch(source1->tag){
              case Iex_Const:
                source1Temp = -1;
                break;
              case Iex_RdTmp:
                source1Temp = source1->Iex.RdTmp.tmp;
                break;
              default:
                tl_assert(0);
                return;
              }
              switch(source2->tag){
              case Iex_Const:
                source2Temp = -1;
                break;
              case Iex_RdTmp:
                source2Temp = source2->Iex.RdTmp.tmp;
                break;
              default:
                tl_assert(0);
                return;
              }
              ValueType resultTypes[4];
              typeJoins(exprTypeArray(source1), exprTypeArray(source2),
                        tempSize(sbIn->tyenv, destTemp), resultTypes);
              for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                dirty |= refineTempBlockType(destTemp, i, resultTypes[i]);
                dirty |= refineTempBlockType(source1Temp, i, tempBlockType(destTemp, i));
                dirty |= refineTempBlockType(source2Temp, i, tempBlockType(destTemp, i));
              }
            }
            break;
          case Iex_Load:
            // We can just do nothing for these, since we very rarely
            // have any info about their source.
            break;
          case Iex_Qop:
            {
              IRQop* details = expr->Iex.Qop.details;
              for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                ValueType argType = opBlockArgPrecision(details->op, i);
                dirty |= refineExprBlockType(details->arg1, i, argType);
                dirty |= refineExprBlockType(details->arg2, i, argType);
                dirty |= refineExprBlockType(details->arg3, i, argType);
                dirty |= refineExprBlockType(details->arg4, i, argType);
                dirty |= refineTempBlockType(destTemp, i,
                                             resultBlockPrecision(details->op, i));
              }
            }
            break;
          case Iex_Triop:
            {
              IRTriop* details = expr->Iex.Triop.details;
              for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                ValueType argType = opBlockArgPrecision(details->op, i);
                dirty |= refineExprBlockType(details->arg1, i, argType);
                dirty |= refineExprBlockType(details->arg2, i, argType);
                dirty |= refineExprBlockType(details->arg3, i, argType);
                dirty |= refineTempBlockType(destTemp, i,
                                             resultBlockPrecision(details->op, i));
              }
            }
            break;
          case Iex_Binop:
            {
              IROp op = expr->Iex.Binop.op;
              // Most of this code is for handling conversions, which
              // can be tricky to infer properly because they are
              // often polymorphic.
              if (isConversionOp(op)){
                ValueType arg1Type = conversionArgPrecision(op, 0);
                if (arg1Type == Vt_Unknown && tempBlockType(destTemp, 0) == Vt_NonFloat){
                  arg1Type = Vt_NonFloat;
                }

                ValueType arg2Type = conversionArgPrecision(op, 1);
                if (arg2Type == Vt_Unknown && tempBlockType(destTemp, 0) == Vt_NonFloat){
                  arg2Type = Vt_NonFloat;
                }

                for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                  dirty |= refineExprBlockType(expr->Iex.Binop.arg1, i, arg1Type);
                  dirty |= refineExprBlockType(expr->Iex.Binop.arg2, i, arg2Type);
                }
                if (!staticallyFloatType(arg1Type) &&
                    !staticallyFloatType(arg2Type)){
                  for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                    refineTempBlockType(destTemp, i, typeMeet(arg1Type, arg2Type));
                  }
                } else {
                  for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                    refineTempBlockType(destTemp, i, resultPrecision(op));
                  }
                }
              } else {
                for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                  ValueType argType = opBlockArgPrecision(op, i);
                  dirty |= refineExprBlockType(expr->Iex.Binop.arg1, i, argType);
                  dirty |= refineExprBlockType(expr->Iex.Binop.arg2, i, argType);
                  dirty |= refineTempBlockType(destTemp, i, resultBlockPrecision(op, i));
                }
              }
            }
            break;
          case Iex_Unop:
            {
              // Most of this code is for handling conversions, which
              // can be tricky to infer properly because they are
              // often polymorphic.
              IRExpr* arg = expr->Iex.Unop.arg;
              IROp op = expr->Iex.Unop.op;
              if (isConversionOp(op)){
                ValueType srcType = conversionArgPrecision(op, 0);
                if (srcType == Vt_Unknown && tempBlockType(destTemp, 0) == Vt_NonFloat){
                  srcType = Vt_NonFloat;
                }
                for(int i = 0; i < INT(exprSize(sbIn->tyenv, arg)); ++i){
                  dirty |= refineExprBlockType(arg, i, srcType);
                }
                for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                  if (!staticallyFloat(arg, i)){
                    refineTempBlockType(destTemp, i, exprBlockType(arg, i));
                  } else {
                    refineTempBlockType(destTemp, i, resultPrecision(op));
                  }
                }
              } else {
                for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
                  ValueType argType = opBlockArgPrecision(op, i);
                  dirty |= refineExprBlockType(arg, i, argType);
                  dirty |= refineTempBlockType(destTemp, i, resultBlockPrecision(op, i));
                }
              }
            }
            break;
          case Iex_Const:
            for(int i = 0; i < INT(tempSize(sbIn->tyenv, destTemp)); ++i){
              dirty |= refineTempBlockType(destTemp, i, constType(expr->Iex.Const.con));
            }
            break;
          case Iex_CCall:
            break;
          default:
            ppIRExpr(expr);
            VG_(printf)("\n");
            tl_assert(0);
            return;
          }
        }
        break;
      case Ist_Store:
        break;
      case Ist_StoreG:
        break;
      case Ist_LoadG:
        break;
      case Ist_CAS:
        break;
      case Ist_Dirty:
        break;
      case Ist_LLSC:
      default:
        tl_assert(0);
        break;
      }
    }
    direction = -direction;
  }
  if (print_inferred_types){
    printTypeState(sbIn->tyenv);
  }
}

void typeJoins(ValueType* types1, ValueType* types2, FloatBlocks numTypes, ValueType* out){
  for(int i = 0; i < INT(numTypes); ++i){
    out[i] = typeJoin(types1[i], types2[i]);
  }
}

ValueType typeJoin(ValueType type1, ValueType type2){
  switch(type1){
  case Vt_Unknown:
    return Vt_Unknown;
  case Vt_NonFloat:
    if (type2 == Vt_NonFloat){
      return Vt_NonFloat;
    } else {
      return Vt_Unknown;
    }
  case Vt_UnknownFloat:
    if (type2 == Vt_UnknownFloat ||
        type2 == Vt_Double ||
        type2 == Vt_Single){
      return Vt_UnknownFloat;
    } else {
      return Vt_Unknown;
    }
  case Vt_Double:
    switch(type2){
    case Vt_Double:
      return Vt_Double;
    case Vt_UnknownFloat:
    case Vt_Single:
      return Vt_UnknownFloat;
    case Vt_Unknown:
    case Vt_NonFloat:
      return Vt_Unknown;
    }
  case Vt_Single:
    switch(type2){
    case Vt_Single:
      return Vt_Single;
    case Vt_UnknownFloat:
    case Vt_Double:
      return Vt_UnknownFloat;
    case Vt_Unknown:
    case Vt_NonFloat:
      return Vt_Unknown;
    }
  default:
    tl_assert(0);
    return Vt_Unknown;
  }
}
ValueType typeMeet(ValueType type1, ValueType type2){
  switch(type1){
  case Vt_Unknown:
    return type2;
  case Vt_NonFloat:
    tl_assert2(type2 == Vt_Unknown || type2 == Vt_NonFloat,
               "Cannot meet types %s and %s!\n", typeName(type1), typeName(type2));
    return Vt_NonFloat;
  case Vt_UnknownFloat:
    tl_assert2(type2 != Vt_NonFloat,
               "Cannot meet types %s and %s!\n", typeName(type1), typeName(type2));
    if (type2 == Vt_Unknown){
      return Vt_UnknownFloat;
    } else {
      return type2;
    }
  case Vt_Double:
    tl_assert2(type2 != Vt_NonFloat && type2 != Vt_Single,
               "Cannot meet types %s and %s!\n", typeName(type1), typeName(type2));
    return Vt_Double;
  case Vt_Single:
    tl_assert2(type2 != Vt_NonFloat && type2 != Vt_Double,
               "Cannot meet types %s and %s!\n", typeName(type1), typeName(type2));
    return Vt_Single;
  default:
    tl_assert(0);
    return Vt_Unknown;
  }
}

const char* typeName(ValueType type){
  switch(type){
  case Vt_Unknown:
    return "Vt_Unknown";
  case Vt_NonFloat:
    return "Vt_NonFloat";
  case Vt_UnknownFloat:
    return "Vt_UnknownFloat";
  case Vt_Double:
    return "Vt_Double";
  case Vt_Single:
    return "Vt_Single";
  default:
    tl_assert(0);
    return "";
  }
}

ValueType constType(const IRConst* constant){
  switch(typeOfIRConst(constant)){
  case Ity_INVALID:
    tl_assert(0);
  case Ity_D32:
  case Ity_D64:
  case Ity_D128:
    tl_assert2(0, "Herbgrind doesn't support decimal floating point.\n");
  case Ity_I128:
    tl_assert2(0, "Herbgrind doesn't support 128-bit integers.\n");
  case Ity_F128:
    tl_assert2(0, "Herbgrind doesn't suport 128-bit floating point numbers.\n");
  case Ity_F16:
    tl_assert2(0, "Herbgrind doesn't support 16-bit floating point numbers.\n");
  case Ity_I1:
  case Ity_I8:
  case Ity_I16:
    return Vt_NonFloat;
  case Ity_I32:
  case Ity_I64:
  case Ity_V128:
  case Ity_V256:
    return Vt_Unknown;
  case Ity_F32:
    return Vt_Single;
  case Ity_F64:
    return Vt_Double;
  default:
    tl_assert(0);
    return Vt_Unknown;
  }
}

int isFloatType(ValueType type){
  return typeJoin(type, Vt_UnknownFloat) == Vt_UnknownFloat;
}

void printTypeState(IRTypeEnv* tyenv){
  for(int i = 0; i < tyenv->types_used; ++i){
    VG_(printf)("Printing state for t%d...\n", i);
    int hasKnown = 0;
    for(int j = 0; j < INT(tempSize(tyenv, i)); ++j){
      if (tempTypes[i][j] != Vt_Unknown){
        hasKnown = 1;
        break;
      }
    }
    if (hasKnown){
      VG_(printf)("t%d :", i);
      for(int j = 0; j < INT(tempSize(tyenv, i)); ++j){
        if (tempTypes[i][j] != Vt_Unknown){
          VG_(printf)(" %s", typeName(tempTypes[i][j]));
        }
      }
      VG_(printf)("\n");
    }
  }
  for(int i = 0; i < MAX_REGISTERS; ++i){
    if (tsTypes[i] != NULL){
      VG_(printf)("TS(%d) : ", i);
      TSTypeEntry* curEntry = tsTypes[i];
      while(curEntry != NULL){
        VG_(printf)("%s@%d, ", typeName(curEntry->type), curEntry->instrIndexSet);
        curEntry = curEntry->next;
      }
      VG_(printf)("\n");
    }
  }
}
