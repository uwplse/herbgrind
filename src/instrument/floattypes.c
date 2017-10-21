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

Stack* tsTypeEntries = NULL;

ValueType tempTypes[MAX_TEMPS];
TSTypeEntry* tsTypes[MAX_REGISTERS];
ShadowStatus tempShadowStatus[MAX_TEMPS];
ShadowStatus tsShadowStatus[MAX_TEMPS];

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
ValueType tempType(int idx){
  return tempTypes[idx];
}
Bool refineTempType(int tempIdx, ValueType type){
  if (tempTypes[tempIdx] == type){
    return False;
  } else {
    tempTypes[tempIdx] = typeMeet(type, tempTypes[tempIdx]);
    return True;
  }
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
    if (tempType(idx) == Vt_Single){
      return sizeof(float);
    } else if (tempType(idx) == Vt_Double){
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
    if (tempType(idx) == Vt_Single){
      return 4;
    } else if (tempType(idx) == Vt_Double){
      return 2;
    }
  case Ity_V256:
    if (tempType(idx) == Vt_Single){
      return 8;
    } else if (tempType(idx) == Vt_Double){
      return 4;
    }
  default:
    VG_(printf)("%d\n", tempType(idx));
    ppIRType(typeOfIRTemp(sbOut->tyenv, idx));
    tl_assert(0);
    return 0;
  }
}
Bool staticallyFloat(IRExpr* expr){
  switch(expr->tag){
  case Iex_Const:
    return False;
  case Iex_RdTmp:
    return typeMeet(tempType(expr->Iex.RdTmp.tmp), Vt_UnknownFloat) == Vt_UnknownFloat;
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
  return staticallyFloat(expr) &&
    tempShadowStatus[expr->Iex.RdTmp.tmp] == Ss_Shadowed;
}
Bool canBeFloat(IRTypeEnv* typeEnv, IRExpr* expr){
  if (!isFloatIRType(typeOfIRExpr(typeEnv, expr))){
    return False;
  } else if (expr->tag == Iex_Const){
    return True;
  } else if (tempType(expr->Iex.RdTmp.tmp) == Vt_NonFloat){
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
  } else if (tempType(expr->Iex.RdTmp.tmp) == Vt_NonFloat){
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
  return tsType(tsAddr, instrIdx) == Vt_NonFloat;
}
Bool tsHasStaticShadow(Int tsAddr, int instrIdx){
  switch(tsType(tsAddr, instrIdx)){
  case Vt_Single:
  case Vt_Double:
    return True;
  default:
    return False;
  }
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
  while(*nextTSEntry != NULL && (*nextTSEntry)->instrIndexSet < instrIdx){
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
  return True;
}
Bool refineTSType(int idx, int instrIdx, ValueType type){
  if (tsTypes[idx] == NULL || tsTypes[idx]->instrIndexSet > instrIdx){
    setTSType(idx, 0, type);
    return True;
  }
  TSTypeEntry* nextTSEntry = tsTypes[idx];
  while(nextTSEntry->next != NULL && nextTSEntry->next->instrIndexSet < instrIdx){
    nextTSEntry = nextTSEntry->next;
  }
  if (nextTSEntry->type == type){
    return False;
  } else {
    nextTSEntry->type = typeMeet(nextTSEntry->type, type);
    return True;
  }
}
ValueType tsType(Int tsAddr, int instrIdx){
  if (tsTypes[tsAddr] == NULL || tsTypes[tsAddr]->instrIndexSet > instrIdx){
    return Vt_Unknown;
  }
  TSTypeEntry* nextTSEntry = tsTypes[tsAddr];
  while(nextTSEntry->next != NULL && nextTSEntry->next->instrIndexSet < instrIdx){
    nextTSEntry = nextTSEntry->next;
  }
  return nextTSEntry->type;
}
ValueType inferTSType64(Int tsAddr, int instrIdx){
  tl_assert2(tsType(tsAddr + sizeof(float), instrIdx) != Vt_Double,
             "Mismatched float at TS(%d)!", tsAddr);
  tl_assert2(tsType(tsAddr, instrIdx) != Vt_Double ||
             tsType(tsAddr + sizeof(float), instrIdx) == Vt_NonFloat,
             "Mismatched float at TS(%d)!", tsAddr);
  if (tsType(tsAddr, instrIdx) == Vt_Unknown){
    return tsType(tsAddr + sizeof(float), instrIdx);
  } else {
    return tsType(tsAddr, instrIdx);
  }
}

ValueType argPrecision(IROp op_code){
  switch((int)op_code){
    // Non-semantic ops have no need for this, since they will never
    // be constructing new shadow values, so we can just return
    // Vt_Unknown for them.
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
    return Vt_Single;
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
    return Vt_Double;
  case Iop_SetV128lo64:
    return Vt_Unknown;
  default:
    return Vt_NonFloat;
  }
}
ValueType resultPrecision(IROp op_code){
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
    return Vt_Double;
  case Iop_SetV128lo64:
  case Iop_64UtoV128:
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
  while(dirty){
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
    for(int instrIdx = 0; instrIdx < sbIn->stmts_used; ++instrIdx){
      IRStmt* stmt = sbIn->stmts[instrIdx];
      VG_(printf)("Looking at instruction: ");
      ppIRStmt(stmt);
      VG_(printf)("\n");
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
              dirty |= setTSType(destLocation, instrIdx, srcType);
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
              IRTemp srcTemp = sourceData->Iex.RdTmp.tmp;
              dirty |= setTSType(destLocation, instrIdx, tempType(srcTemp));
              dirty |= refineTempType(srcTemp, tsType(destLocation, instrIdx));
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
                ++i){
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
                ++i){
              int destLocation =
                stmt->Ist.PutI.details->descr->base + i;
              ValueType srcType = tempType(sourceData->Iex.RdTmp.tmp);
              if (srcType == Vt_Double && i % 2 == 1){
                srcType = Vt_NonFloat;
              }
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
              dirty |= refineTempType(destTemp, tsType(sourceOffset, instrIdx));
              dirty |= refineTSType(sourceOffset, instrIdx, tempType(destTemp));
            }
            break;
          case Iex_GetI:
            // Ugh lets not even try to get this one right for now,
            // these are pretty rare.
            break;
          case Iex_RdTmp:
            {
              int sourceTemp = expr->Iex.RdTmp.tmp;
              dirty |= refineTempType(destTemp, tempType(sourceTemp));
              dirty |= refineTempType(sourceTemp, tempType(destTemp));
            }
            break;
          case Iex_ITE:
            {
              IRExpr* source1 = expr->Iex.ITE.iftrue;
              IRExpr* source2 = expr->Iex.ITE.iffalse;
              int source1Temp, source2Temp;
              ValueType source1Type, source2Type;
              switch(source1->tag){
              case Iex_Const:
                source1Temp = -1;
                source1Type = constType(source1->Iex.Const.con);
                break;
              case Iex_RdTmp:
                source1Temp = source1->Iex.RdTmp.tmp;
                source1Type = tempType(source1Temp);
                break;
              default:
                tl_assert(0);
                return;
              }
              switch(source2->tag){
              case Iex_Const:
                source2Temp = -1;
                source2Type = constType(source1->Iex.Const.con);
                break;
              case Iex_RdTmp:
                source2Temp = source2->Iex.RdTmp.tmp;
                source2Type = tempType(source2Temp);
                break;
              default:
                tl_assert(0);
                return;
              }
              dirty |= refineTempType(destTemp, typeJoin(source1Type, source2Type));
              // These two branches take care of the case where we
              // already know something more specific about the output
              // temp than we know about either input. In this case
              // the above statement will be a noop.
              if (source1Temp != -1){
                dirty |= refineTempType(source1Temp, tempType(destTemp));
              }
              if (source2Temp != -1){
                dirty |= refineTempType(source2Temp, tempType(destTemp));
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
              ValueType argType = argPrecision(details->op);
              if (details->arg1->tag == Iex_RdTmp){
                dirty |= refineTempType(details->arg1->Iex.RdTmp.tmp, argType);
              }
              if (details->arg2->tag == Iex_RdTmp){
                dirty |= refineTempType(details->arg2->Iex.RdTmp.tmp, argType);
              }
              if (details->arg3->tag == Iex_RdTmp){
                dirty |= refineTempType(details->arg3->Iex.RdTmp.tmp, argType);
              }
              if (details->arg4->tag == Iex_RdTmp){
                dirty |= refineTempType(details->arg4->Iex.RdTmp.tmp, argType);
              }
              dirty |= refineTempType(destTemp, resultPrecision(details->op));
            }
            break;
          case Iex_Triop:
            {
              IRTriop* details = expr->Iex.Triop.details;
              ValueType argType = argPrecision(details->op);
              if (details->arg1->tag == Iex_RdTmp){
                dirty |= refineTempType(details->arg1->Iex.RdTmp.tmp, argType);
              }
              if (details->arg2->tag == Iex_RdTmp){
                dirty |= refineTempType(details->arg2->Iex.RdTmp.tmp, argType);
              }
              if (details->arg3->tag == Iex_RdTmp){
                dirty |= refineTempType(details->arg3->Iex.RdTmp.tmp, argType);
              }
              dirty |= refineTempType(destTemp, resultPrecision(details->op));
            }
            break;
          case Iex_Binop:
            {
              ValueType argType = argPrecision(expr->Iex.Binop.op);
              IRExpr* arg1 = expr->Iex.Binop.arg1;
              IRExpr* arg2 = expr->Iex.Binop.arg2;
              if (arg1->tag == Iex_RdTmp){
                dirty |= refineTempType(arg1->Iex.RdTmp.tmp, argType);
              }
              if (arg2->tag == Iex_RdTmp){
                dirty |= refineTempType(arg2->Iex.RdTmp.tmp, argType);
              }
              dirty |= refineTempType(destTemp, resultPrecision(expr->Iex.Binop.op));
            }
            break;
          case Iex_Unop:
            {
              IRExpr* arg = expr->Iex.Unop.arg;
              if (arg->tag == Iex_RdTmp){
                dirty |= refineTempType(arg->Iex.RdTmp.tmp,
                                        argPrecision(expr->Iex.Unop.op));
              }
              dirty |= refineTempType(destTemp, resultPrecision(expr->Iex.Unop.op));
            }
            break;
          case Iex_Const:
            dirty |= refineTempType(destTemp, constType(expr->Iex.Const.con));
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
    return Vt_Unknown;
  case Ity_V128:
  case Ity_V256:
    return Vt_UnknownFloat;
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
