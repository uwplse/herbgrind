/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie           floattypes.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016 Alex Sanchez-Stern

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

FloatType tempContext[MAX_TEMPS];
FloatType tsContext[MAX_REGISTERS];
VgHashTable* memContext = NULL;
Stack* typeEntries = NULL;

void initTypeState(void){
  memContext = VG_(HT_construct)("mem context");
  typeEntries = mkStack();
}
void resetTypeState(void){
  VG_(memset)(tempContext, 0, sizeof tempContext);
  VG_(memset)(tsContext, 0, sizeof tsContext);
  addClearMemTypes();
}
Bool tempIsTyped(int idx){
  return tempContext[idx] == Ft_Single ||
    tempContext[idx] == Ft_Double;
}
FloatType tempType(int idx){
  /* tl_assert2(tempContext[idx] != Ft_NonFloat, */
  /*            "Tried to get the type of temp %d, " */
  /*            "but it hasn't been set yet this SB!\n", */
  /*            idx); */
  return tempContext[idx];
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
    if (tempType(idx) == Ft_Single){
      return sizeof(float);
    } else if (tempType(idx) == Ft_Double){
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
    if (tempType(idx) == Ft_Single){
      return 4;
    } else if (tempType(idx) == Ft_Double){
      return 2;
    }
  default:
    VG_(printf)("%d\n", tempType(idx));
    ppIRType(typeOfIRTemp(sbOut->tyenv, idx));
    tl_assert(0);
    return 0;
  }
}
Bool hasStaticShadow(IRExpr* expr){
  switch(expr->tag){
  case Iex_Const:
    return False;
  case Iex_RdTmp:
    return tempIsTyped(expr->Iex.RdTmp.tmp);
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
  } else if (tempContext[expr->Iex.RdTmp.tmp] == Ft_NonFloat ||
             tempContext[expr->Iex.RdTmp.tmp] == Ft_Unshadowed) {
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
  } else if (tempContext[expr->Iex.RdTmp.tmp] == Ft_NonFloat){
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
  } else if (tempContext[expr->Iex.RdTmp.tmp] == Ft_NonFloat){
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

Bool tsAddrCanHaveShadow(Int tsAddr){
  switch(tsContext[tsAddr]){
  case Ft_NonFloat:
  case Ft_Unshadowed:
    return False;
  default:
    return True;
  }
}
Bool tsHasStaticShadow(Int tsAddr){
  switch(tsContext[tsAddr]){
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

void pushEntry(void* entry);
void pushEntry(void* entry){
  stack_push(typeEntries, entry);
}

void addClearMemTypes(void){
  VG_(HT_destruct)(memContext, pushEntry);
  memContext = VG_(HT_construct)("mem context");
}
void setMemType(ULong addr, FloatType type){
  MemTypeEntry* entry = VG_(HT_lookup)(memContext, (UWord)addr);
  if (entry != NULL){
    entry->type = type;
  } else if (!stack_empty(typeEntries)){
    if (stack_empty(typeEntries)){
      entry = VG_(malloc)("entry", sizeof(MemTypeEntry));
    } else {
      entry = (void*)stack_pop(typeEntries);
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

FloatType inferTSType64(Int tsAddr){
  tl_assert2(tsContext[tsAddr + sizeof(float)] != Ft_Double,
             "Mismatched float at TS(%d)!", tsAddr);
  tl_assert2(tsContext[tsAddr] != Ft_Double ||
             tsContext[tsAddr + sizeof(float)] == Ft_NonFloat,
             "Mismatched float at TS(%d)!", tsAddr);
  /* tl_assert2(tsContext[tsAddr] != Ft_NonFloat || */
  /*            tsContext[tsAddr + sizeof(float)] == Ft_NonFloat, */
  /*            "Mismatched float at TS(%d)!", tsAddr); */
  /* tl_assert2(tsContext[tsAddr] != Ft_Single || */
  /*            tsContext[tsAddr + sizeof(float)] == Ft_Single || */
  /*            tsContext[tsAddr + sizeof(float)] == Ft_NonFloat || */
  /*            tsContext[tsAddr + sizeof(float)] == Ft_Unknown, */
  /*            "Mismatched float at TS(%d)!", tsAddr); */
  if (tsContext[tsAddr] == Ft_Unknown){
    return tsContext[tsAddr + sizeof(float)];
  } else {
    return tsContext[tsAddr];
  }
}

FloatType argPrecision(IROp op_code){
  switch(op_code){
    // Non-semantic ops have no need for this, since they will never
    // be constructing new shadow values, so we can just return
    // Ft_Unknown for them.
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
    return Ft_Single;
  case Iop_RSqrtEst5GoodF64:
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
    return Ft_Double;
  case Iop_SetV128lo64:
    return Ft_Unknown;
  default:
    tl_assert(0);
    return Ft_NonFloat;
  }
}
FloatType resultPrecision(IROp op_code){
  switch(op_code){
    // Non-semantic ops have no need for this, since they will never
    // be constructing new shadow values, so we can just return
    // Ft_NonFloat for them.
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
  case Iop_RoundF32toInt:
  case Iop_SetV128lo32:
  case Iop_RoundF64toF32:
  case Iop_TruncF64asF32:
  case Iop_F64toF32:
    return Ft_Single;
  case Iop_RSqrtEst5GoodF64:
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
    || type == Ity_V128;
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
