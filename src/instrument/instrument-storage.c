/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie   instrument-storage.c ---*/
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

#include "instrument-storage.h"
#include "../runtime/value-shadowstate/value-shadowstate.h"
#include "helpers.h"

#include "pub_tool_libcprint.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcassert.h"

void instrumentRdTmp(IRSB* sbOut, IRTemp dest, IRTemp src){
  tl_assert2(typeOfIRTemp(sbOut->tyenv, dest) ==
             typeOfIRTemp(sbOut->tyenv, src),
             "Source of temp move doesn't match dest!");
  if (!isFloat(sbOut->tyenv, dest)) return;
  // Load both the temps into memory.
  IRTemp newShadowTemp = newIRTemp(sbOut->tyenv, Ity_I64);
  addLoad64C(sbOut, newShadowTemp, &(shadowTemps[src]));
  IRTemp oldShadowTemp = newIRTemp(sbOut->tyenv, Ity_I64);
  addLoad64C(sbOut, oldShadowTemp, &(shadowTemps[dest]));

  // Disown the old temp
  addDisownShadowTempCall(sbOut, oldShadowTemp);

  // Copy across the new temp and increment it's ref count.
  // Increment the ref count of the new temp
  IRTemp newTempNonNull = newIRTemp(sbOut->tyenv, Ity_I1);
  addNonZeroCheck64(sbOut, newTempNonNull, newShadowTemp);

  IRTemp newShadowTempCopy = newIRTemp(sbOut->tyenv, Ity_I64);
  IRDirty* copyShadowTempDirty =
    unsafeIRDirty_1_N(newShadowTempCopy,
                      1, "copyShadowTemp",
                      VG_(fnptr_to_fnentry)(&copyShadowTemp),
                      mkIRExprVec_1(IRExpr_RdTmp(newShadowTemp)));
  copyShadowTempDirty->mFx = Ifx_Read;
  copyShadowTempDirty->mAddr = IRExpr_RdTmp(newShadowTemp);
  copyShadowTempDirty->mSize = sizeof(ShadowTemp);
  copyShadowTempDirty->guard = IRExpr_RdTmp(newTempNonNull);
  addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowTempDirty));

  addStoreGC(sbOut, newShadowTempCopy,
             &(shadowTemps[dest]), newTempNonNull);
}
void instrumentWriteConst(IRSB* sbOut, IRTemp dest,
                          IRConst* con){
  if (!isFloat(sbOut->tyenv, dest)) return;
  
  IRTemp oldShadowTemp = newIRTemp(sbOut->tyenv, Ity_I64);
  addLoad64C(sbOut, oldShadowTemp, &(shadowTemps[dest]));

  // Disown the old temp
  addDisownShadowTempCall(sbOut, oldShadowTemp);
}
void instrumentITE(IRSB* sbOut, IRTemp dest,
                   IRExpr* trueExpr, IRExpr* falseExpr){
}
void instrumentPut(IRSB* sbOut, Int tsDest, IRExpr* data){
}
void instrumentPutI(IRSB* sbOut,
                    IRExpr* varOffset, Int constOffset,
                    Int arrayBase, Int numElems, IRType elemType,
                    IRExpr* data){
}
void instrumentGet(IRSB* sbOut, IRTemp dest,
                   Int tsSrc, IRType type){
}
void instrumentGetI(IRSB* sbOut, IRTemp dest,
                    IRExpr* varOffset, Int constOffset,
                    Int arrayBase, Int numElems, IRType elemType){
}
void instrumentLoad(IRSB* sbOut, IRTemp dest,
                    IRExpr* addr, IRType type){
}
void instrumentLoadG(IRSB* sbOut, IRTemp dest,
                     IRExpr* altValue, IRExpr* guard,
                     IRExpr* addr, IRLoadGOp conversion){
}
void instrumentStore(IRSB* sbOut, IRExpr* addr,
                     IRExpr* data){
}
void instrumentStoreG(IRSB* sbOut, IRExpr* addr,
                      IRExpr* guard, IRExpr* data){
}
void instrumentCAS(IRSB* sbOut,
                   IRCAS* details){
}
int isFloat(IRTypeEnv* env, IRTemp temp){
  IRType type = typeOfIRTemp(env, temp);
  return isFloatType(type);
}
int isFloatType(IRType type){
  return type == Ity_F32 || type == Ity_F64
    || type == Ity_V128;
}

void addDisownShadowTempCall(IRSB* sbOut, IRTemp shadowTemp){
  IRTemp tempNonNull = newIRTemp(sbOut->tyenv, Ity_I1);
  addNonZeroCheck64(sbOut, tempNonNull, shadowTemp);

  IRDirty* disownShadowTempDirty =
    unsafeIRDirty_0_N(1, "disownShadowTemp",
                      VG_(fnptr_to_fnentry)(&disownShadowTemp),
                      mkIRExprVec_1(IRExpr_RdTmp(shadowTemp)));

  // This could potentially be a little dubious, because we don't just
  // modify the shadow temp, we also potentially modify any shadow
  // values it contains (by freeing them), and any expr values those
  // contain, and any expr values those exprs reference. As long as we
  // don't ever have inline VEX touch values or exprs, and are very
  // careful, we can probably get away with this. Worst case, we might
  // end up sticking in a memory fence or something.
  disownShadowTempDirty->mFx = Ifx_Modify;
  disownShadowTempDirty->mAddr = IRExpr_RdTmp(shadowTemp);
  disownShadowTempDirty->mSize = sizeof(ShadowTemp);
  disownShadowTempDirty->guard = IRExpr_RdTmp(tempNonNull);

  addStmtToIRSB(sbOut, IRStmt_Dirty(disownShadowTempDirty));
}
