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

FloatTypeEntry tempContext[MAX_TEMPS];
TSFloatTypeEntry* tsContext[MAX_REGISTERS];
VgHashTable* memContext = NULL;
Stack* memTypeEntries = NULL;
Stack* tsTypeEntries = NULL;

void initTypeState(void){
  memContext = VG_(HT_construct)("mem context");
  tsTypeEntries = mkStack();
}
void resetTypeState(void){
  VG_(memset)(tempContext, 0, sizeof tempContext);
  for(int i = 0; i < MAX_REGISTERS; ++i){
    while (tsContext[i] != NULL){
      stack_push(tsTypeEntries, (StackNode*)tsContext[i]);
      tsContext[i] = tsContext[i]->next;
    }
  }
  addClearMemTypes();
}
Bool tempIsTyped(int idx, int instrIdx){
  if (tempContext[idx].instrIndexSet > instrIdx){
    return False;
  }
  return tempContext[idx].type == Ft_Single ||
    tempContext[idx].type == Ft_Double;
}
FloatType tempType(int idx, int instrIdx){
  if (tempContext[idx].instrIndexSet > instrIdx){
    return Ft_Unknown;
  }
  return tempContext[idx].type;
}
FloatType tempEventualType(int idx){
  return tempContext[idx].type;
}
void setTempType(int tempIdx, int instrIdx, FloatType type){
  tl_assert(tempContext[tempIdx].type == Ft_Unknown);
  tl_assert(tempContext[tempIdx].instrIndexSet == 0);
  tempContext[tempIdx].type = type;
  tempContext[tempIdx].instrIndexSet = instrIdx;
}
int valueSize(IRSB* sbOut, int idx){
  switch(typeOfIRTemp(sbOut->tyenv, idx)){
  case Ity_I64:
  case Ity_F64:
    return sizeof(double);
  case Ity_F32:
  case Ity_I32:
    return sizeof(float);
  case Ity_V128:
  case Ity_V256:
    if (tempEventualType(idx) == Ft_Single){
      return sizeof(float);
    } else if (tempEventualType(idx) == Ft_Double){
      return sizeof(double);
    }
  default:
    tl_assert(0);
    return 0;
  }
}
int numTempValues(IRSB* sbOut, int idx){
  switch(typeOfIRTemp(sbOut->tyenv, idx)){
  case Ity_I64:
  case Ity_F64:
  case Ity_I32:
  case Ity_F32:
    return 1;
  case Ity_V128:
    if (tempEventualType(idx) == Ft_Single){
      return 4;
    } else if (tempEventualType(idx) == Ft_Double){
      return 2;
    }
  case Ity_V256:
    if (tempEventualType(idx) == Ft_Single){
      return 8;
    } else if (tempEventualType(idx) == Ft_Double){
      return 4;
    }
  default:
    VG_(printf)("%d\n", tempEventualType(idx));
    ppIRType(typeOfIRTemp(sbOut->tyenv, idx));
    tl_assert(0);
    return 0;
  }
}
Bool hasStaticShadow(IRExpr* expr, int instrIdx){
  switch(expr->tag){
  case Iex_Const:
    return False;
  case Iex_RdTmp:
    return tempIsTyped(expr->Iex.RdTmp.tmp, instrIdx);
  default:
    VG_(printf)("Hey, what are you trying to pull here, man? "
                "You can't check the shadow of a non-trivial "
                "expression. It's got to be either a RdTmp or "
                "a Const, not %p ", expr);
    ppIRExpr(expr);
    tl_assert(0);
  }
}
Bool hasStaticShadowEventually(IRExpr* expr){
  switch(expr->tag){
  case Iex_Const:
    return False;
  case Iex_RdTmp:{
      FloatType type = tempEventualType(expr->Iex.RdTmp.tmp);
      return type == Ft_Single || type == Ft_Double;
  }
  default:
    VG_(printf)("Hey, what are you trying to pull here, man? "
                "You can't check the shadow of a non-trivial "
                "expression. It's got to be either a RdTmp or "
                "a Const, not %p ", expr);
    ppIRExpr(expr);
    tl_assert(0);
  }
}
Bool canHaveShadow(IRTypeEnv* typeEnv, IRExpr* expr){
  if (expr->tag == Iex_Const){
    return False;
  } else if (!isFloatType(typeOfIRExpr(typeEnv, expr))){
    return False;
  } else if (tempEventualType(expr->Iex.RdTmp.tmp) == Ft_NonFloat ||
             tempEventualType(expr->Iex.RdTmp.tmp) == Ft_Unshadowed) {
    return False;
  } else {
    return True;
  }
}
Bool canBeFloat(IRTypeEnv* typeEnv, IRExpr* expr){
  if (!isFloatType(typeOfIRExpr(typeEnv, expr))){
    return False;
  } else if (expr->tag == Iex_Const){
    return True;
  } else if (tempEventualType(expr->Iex.RdTmp.tmp) == Ft_NonFloat){
    return False;
  } else {
    return True;
  }
}
Bool canStoreShadow(IRTypeEnv* typeEnv, IRExpr* expr){
  if (expr->tag == Iex_Const){
    return False;
  } else if (!isFloatType(typeOfIRExpr(typeEnv, expr))){
    return False;
  } else if (tempEventualType(expr->Iex.RdTmp.tmp) == Ft_NonFloat){
    tl_assert2(0, "Why are you even asking this?");
    return False;
  } else {
    return True;
  }
}

int exprSize(IRTypeEnv* tyenv, IRExpr* expr){
  return typeSize(typeOfIRExpr(tyenv, expr));
}

int typeSize(IRType type){
  switch (type){
  case Ity_I32:
  case Ity_F32:
    return 1;
  case Ity_I64:
  case Ity_F64:
    return 2;
  case Ity_V128:
    return 4;
  case Ity_V256:
    return 8;
  default:
    return 1;
  }
}
int loadConversionSize(IRLoadGOp conversion){
  switch(conversion){
  case ILGop_IdentV128:
    return 4;
  case ILGop_Ident64:
    return 2;
  case ILGop_Ident32:
    return 1;
  default:
    tl_assert(0);
    return 0;
  }
}

Bool tsAddrCanHaveShadow(Int tsAddr, int instrIdx){
  switch(tsType(tsAddr, instrIdx)){
  case Ft_NonFloat:
  case Ft_Unshadowed:
    return False;
  default:
    return True;
  }
}
Bool tsHasStaticShadow(Int tsAddr, int instrIdx){
  switch(tsType(tsAddr, instrIdx)){
  case Ft_Single:
  case Ft_Double:
    return True;
  default:
    return False;
  }
}
typedef struct _memTypeEntry {
  struct _memTypeEntry* next;
  UWord addr;

  FloatType type;
} MemTypeEntry;

void pushMemEntry(void* entry);
void pushMemEntry(void* entry){
  stack_push(memTypeEntries, entry);
}

void addClearMemTypes(void){
  VG_(HT_destruct)(memContext, pushMemEntry);
  memContext = VG_(HT_construct)("mem context");
}
void setMemType(ULong addr, FloatType type){
  MemTypeEntry* entry = VG_(HT_lookup)(memContext, (UWord)addr);
  if (entry != NULL){
    entry->type = type;
  } else {
    if (stack_empty(memTypeEntries)){
      entry = VG_(malloc)("entry", sizeof(MemTypeEntry));
    } else {
      entry = (void*)stack_pop(memTypeEntries);
    }
    entry->addr = addr;
    entry->type = type;
    VG_(HT_add_node)(memContext, entry);
  }
}
FloatType getMemType(ULong addr){
  MemTypeEntry* entry = VG_(HT_lookup)(memContext, (UWord)addr);
  if (entry != NULL){
    return entry->type;
  } else {
    return Ft_Unknown;
  }
}

FloatType lookupMemType(ULong addr){
  // This cast might not be safe? Should be on 64-bit platforms, but
  // if types start colliding check this out.
  MemTypeEntry* entry = VG_(HT_lookup)(memContext, (UWord)addr);
  if (entry == NULL){
    return Ft_Unknown;
  } else {
    return entry->type;
  }
}
Bool memAddrCanHaveShadow(ULong memAddr){
  switch(lookupMemType(memAddr)){
  case Ft_NonFloat:
  case Ft_Unshadowed:
    return False;
  default:
    return True;
  }
}
Bool memAddrHasStaticShadow(ULong memAddr){
  switch(lookupMemType(memAddr)){
  case Ft_Single:
  case Ft_Double:
    return True;
  default:
    return False;
  }
}
Bool memBlockCanHaveShadow(ULong blockStart, int block_len){
  Bool someShadow = False;
  for(int i = 0; i < block_len; ++i){
    ULong chunk_addr = blockStart + (i * sizeof(float));
    someShadow = someShadow || memAddrCanHaveShadow(chunk_addr);
  }
  return someShadow;
}

FloatType inferMemType(ULong addr, int size){
  for(int i = 0; i < size; ++i){
    ULong chunk_addr = addr + (i * sizeof(float));
    FloatType chunk_type = getMemType(chunk_addr);
    if (chunk_type != Ft_Unknown){
      return chunk_type;
    }
  }
  return Ft_Unknown;
}

void setTSType(int idx, int instrIdx, FloatType type){
  TSFloatTypeEntry** nextTSEntry = &(tsContext[idx]);
  while(*nextTSEntry != NULL && (*nextTSEntry)->instrIndexSet < instrIdx){
    tl_assert2(((*nextTSEntry)->instrIndexSet != instrIdx) ||
               ((*nextTSEntry)->type == Ft_Unknown && type != Ft_Unknown),
               "Type has already been set at index %d!",
               instrIdx);
    nextTSEntry = &((*nextTSEntry)->next);
  }
  TSFloatTypeEntry* newTSEntry;
  if (stack_empty(tsTypeEntries)){
    newTSEntry = VG_(malloc)("TSTypeEntry", sizeof(TSFloatTypeEntry));
  } else {
    newTSEntry = (void*)stack_pop(tsTypeEntries);
  }
  newTSEntry->type = type;
  newTSEntry->instrIndexSet = instrIdx;
  newTSEntry->next = *nextTSEntry;
  *nextTSEntry = newTSEntry;
}
FloatType tsType(Int tsAddr, int instrIdx){
  if (tsContext[tsAddr] == NULL || tsContext[tsAddr]->instrIndexSet > instrIdx){
    return Ft_Unknown;
  }
  TSFloatTypeEntry* nextTSEntry = tsContext[tsAddr];
  while(nextTSEntry->next != NULL && nextTSEntry->next->instrIndexSet < instrIdx){
    nextTSEntry = nextTSEntry->next;
  }
  return tsContext[tsAddr]->type;
}
FloatType inferTSType64(Int tsAddr, int instrIdx){
  tl_assert2(tsType(tsAddr + sizeof(float), instrIdx) != Ft_Double,
             "Mismatched float at TS(%d)!", tsAddr);
  tl_assert2(tsType(tsAddr, instrIdx) != Ft_Double ||
             tsType(tsAddr + sizeof(float), instrIdx) == Ft_NonFloat,
             "Mismatched float at TS(%d)!", tsAddr);
  if (tsType(tsAddr, instrIdx) == Ft_Unknown){
    return tsType(tsAddr + sizeof(float), instrIdx);
  } else {
    return tsType(tsAddr, instrIdx);
  }
}

FloatType argPrecision(IROp op_code){
  switch((int)op_code){
    // Non-semantic ops have no need for this, since they will never
    // be constructing new shadow values, so we can just return
    // Ft_Unknown for them.
  case Iop_CmpF32:
  case Iop_CmpLT32F0x4:
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
    return Ft_Single;
  case Iop_CmpLT64F0x2:
  case Iop_CmpLE64F0x2:
  case Iop_CmpF64:
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
  case Iop_ZeroHI64ofV128:
  case Iop_V128to64:
  case Iop_V128HIto64:
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
  case Iop_64HLtoV128:
  case Iop_F64HLtoF128:
  case Iop_Max64F0x2:
  case Iop_Max64Fx2:
  case Iop_Min64F0x2:
  case Iop_Min64Fx2:
  case Iop_64to32:
    return Ft_Double;
  case Iop_SetV128lo64:
    return Ft_Unknown;
  default:
    ppIROp_Extended(op_code);
    tl_assert(0);
    return Ft_NonFloat;
  }
}
FloatType resultPrecision(IROp op_code){
  switch((int)op_code){
    // Non-semantic ops have no need for this, since they will never
    // be constructing new shadow values, so we can just return
    // Ft_NonFloat for them.
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
    return Ft_Single;
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
    return Ft_Double;
  case Iop_SetV128lo64:
  case Iop_64UtoV128:
    return Ft_Unknown;
  default:
    ppIROp(op_code);
    tl_assert(0);
    return Ft_NonFloat;
  }
}

int isFloatType(IRType type){
  return type == Ity_I32 || type == Ity_I64
    || type == Ity_F32 || type == Ity_F64
    || type == Ity_V128 || type == Ity_V256;
}

int isFloat(IRTypeEnv* env, IRTemp temp){
  IRType type = typeOfIRTemp(env, temp);
  return isFloatType(type);
}

void ppFloatType(FloatType type){
  switch(type){
  case Ft_Unknown:
    VG_(printf)("Ft_Unknown");
    break;
  case Ft_NonFloat:
    VG_(printf)("Ft_NonFloat");
    break;
  case Ft_Unshadowed:
    VG_(printf)("Ft_Unshadowed");
    break;
  case Ft_Single:
    VG_(printf)("Ft_Single");
    break;
  case Ft_Double:
    VG_(printf)("Ft_Double");
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
  while(dirty){
    dirty = 0;
    // We make forward passes through the instructions, building type
    // information.
    //
    // Temporary type information is simple: every temporary has
    // exactly one type throughout the lifetime of the
    // superblock. It's made a little more complicated by the fact
    // that we might at some point want the shadowops to be able to
    // figure out statically when they need to generate a new
    // argument. So for that, we'll also store the instruction index
    // which first actually populates the shadow value. But for the
    // majority of concerns, we just need to worry about each temp
    // having a single type, and we can use the "eventual" functions
    // to check that type.
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
    for(int instrIdx = 0; instrIdx < sbIn->stmts_used; ++instrIdx){
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
            // In the constant case, we just want to make sure the
            // thread state is initially marked as "unshadowed". This
            // means checking whether this was already done, and if
            // not setting it, and setting the dirty bit, so that we
            // know that we'll need to do more inference that might
            // depend on this result.
          case Iex_Const:
            if (tsType(destLocation, instrIdx) == Ft_Unknown){
              dirty = 1;
              setTSType(destLocation, instrIdx, Ft_Unshadowed);
            } else {
              tl_assert(tsType(destLocation, instrIdx) == Ft_Unshadowed);
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
            synchronizeThreadStateAndTemp(sourceData->Iex.RdTmp.tmp,
                                            destLocation, &dirty);
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
                ++i){
              int destLocation =
                stmt->Ist.PutI.details->descr->base + i;
              if (tsType(destLocation, instrIdx) != Ft_Unknown &&
                  tsType(destLocation, instrIdx) != Ft_NonFloat){
                dirty = 1;
                setTSType(destLocation, instrIdx, Ft_Unknown);
              }
            }
            break;
          case Iex_RdTmp:
            for(int i = 0;
                i < stmt->Ist.PutI.details->descr->nElems *
                  sizeofIRType(stmt->Ist.PutI.details->descr->elemTy);
                ++i){
              int destLocation =
                stmt->Ist.PutI.details->descr->base + i;
              FloatType srcType = tempEventualType(sourceData->Iex.RdTmp.tmp);
              FloatType destType = tsType(destLocation, instrIdx);
              if (srcType == Ft_Double && i % 2 == 1){
                srcType = Ft_NonFloat;
              }
              if (destType != Ft_Unknown &&
                  destType != srcType){
                dirty = 1;
                setTSType(destLocation, instrIdx, Ft_Unknown);
              }
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
          switch(expr->tag){
          case Iex_Get:
            {
              int sourceOffset = expr->Iex.Get.offset;
              int destLocation = stmt->Ist.WrTmp.tmp;
              synchronizeThreadStateAndTemp(destLocation, sourceOffset, &dirty);
            }
            break;
          case Iex_GetI:
            // Ugh lets not even try to get this one right for now,
            // these are pretty rare.
            break;
          case Iex_RdTmp:
            {
              int sourceTemp = expr->Iex.RdTmp.tmp;
              int destTemp = stmt->Ist.WrTmp.tmp;
              FloatType sourceType = tempEventuallyType(sourceTemp);
              FloatType destType = tempEventuallyType(destTemp);
              if (srcType != Ft_Unknown){
                if (destType == Ft_Unknown){
                  *dirty = 1;
                  setTempType(destTemp, instrIdx, sourceType);
                } else {
                  tl_assert(destType == srcType);
                }
              } else {
                if (destType != Ft_Unknown){
                  *dirty = 1;
                  setTempType(sourceTemp, instrIdx, destType);
                } else {
                  tl_assert(destType == srcType);
                }
              }
            }
            break;
          case Iex_ITE:
            {
              IRExpr* source1 = expr->Iex.ITE.iftrue;
              IRExpr* source2 = expr->Iex.ITE.iffalse;
              int source1Temp, source2Temp;
              FloatType source1Type, source2Type;
              switch(source1->tag){
              case Iex_Const:
                source1Temp = -1;
                source1Type = Ft_Unshadowed;
                break;
              case Iex_RdTmp:
                source1Temp = source1->Iex.RdTmp.tmp;
                source1Type = tempEventualType(source1Temp);
                break;
              default:
                tl_assert(0);
                return;
              }
              switch(source2->tag){
              case Iex_Const:
                source2Temp = -1;
                source2Type = Ft_Unshadowed;
                break;
              case Iex_RdTmp:
                source2Temp = source2->Iex.RdTmp.tmp;
                source2Type = tempEventualType(source2Temp);
                break;
              default:
                tl_assert(0);
                return;
              }
              if (source1Type !=
            }
            break;
          case Iex_Load:
            break;
          case Iex_Qop:
            break;
          case Iex_Triop:
            break;
          case Iex_Binop:
            break;
          case Iex_Unop:
            break;
          case Iex_Const:
            break;
          case Iex_CCall:
          default:
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
  }
}

// Synchronize the types of a thread state location and a
// temporary. This is common code between the GET instruction and the
// PUT instruction. Sets the "dirty" variable to true if anything gets
// changed
void synchronizeThreadStateAndTemp(IRTemp temp, int tsOffset, int* dirty){
  FloatType srcType = tempEventualType(temp);
  FloatType destType = tsType(tsOffset, instrIdx);
  if (srcType != Ft_Unknown){
    if (destType == Ft_Unknown){
      *dirty = 1;
      setTSType(tsOffset, instrIdx, srcType);
    } else {
      tl_assert(destType == srcType);
    }
  } else {
    if (destType != Ft_Unknown){
      *dirty = 1;
      setTempType(temp, instrIdx, destType);
    } else {
      tl_assert(destType == srcType);
    }
  }
}
            }
            break;
          case Iex_Load:
            break;
          case Iex_Qop:
            break;
          case Iex_Triop:
            break;
          case Iex_Binop:
            break;
          case Iex_Unop:
            break;
          case Iex_Const:
            break;
          case Iex_CCall:
          default:
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
  }
}

// Synchronize the types of a thread state location and a
// temporary. This is common code between the GET instruction and the
// PUT instruction. Sets the "dirty" variable to true if anything gets
// changed
void synchronizeThreadStateAndTemp(IRTemp temp, int tsOffset, int* dirty){
  FloatType srcType = tempEventualType(temp);
  FloatType destType = tsType(tsOffset, instrIdx);
  if (srcType != Ft_Unknown){
    if (destType == Ft_Unknown){
      *dirty = 1;
      setTSType(tsOffset, instrIdx, srcType);
    } else {
      tl_assert(destType == srcType);
    }
  } else {
    if (destType != Ft_Unknown){
      *dirty = 1;
      setTempType(temp, instrIdx, destType);
    } else {
      tl_assert(destType == srcType);
    }
  }
}

ValueType typeJoin(ValueType type1, ValueType type2){

}
ValueType typeMeet(ValueType type1, ValueType type2){
}
