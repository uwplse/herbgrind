
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie        hg_instrument.c ---*/
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

#include "hg_instrument.h"
#include "include/hg_helper.h"
#include "include/hg_macros.h"

void instrumentStatement(IRStmt* st, IRSB* sbOut, Addr stAddr){
  IRExpr* expr;
  IRDirty* copyShadowLocation;
  LoadG_Info* loadGInfo;

  switch (st->tag) {
    // If it's a no op, or just metadata, we'll just pass it into
    // the result IR.
  case Ist_NoOp:
  case Ist_IMark:
    // If it's a memory bus event or an exit, we shouldn't have to
    // do much with it either.
  case Ist_MBE:
  case Ist_Exit:
    addStmtToIRSB(sbOut, st);
    break;
    // So, ABI hints can mean that we're about to enter a wrapped
    // function. They don't always mean that, but they (seem to)
    // always happen before we enter a wrapped function, so they can
    // give us a hint as to where this wrapping is
    // happening. Unfortunately, valgrind doesn't provide us with a
    // mechanism to detect where wrapped functions are being called,
    // but this hack should do the trick.
  case Ist_AbiHint:
    // Add the hint into the output.
    addStmtToIRSB(sbOut, st);
    // Now, store the current address in the field (in our memory, in
    // hg_runtime.c) for the address of the last executed
    // ABIHint. We'll pull this out whenever we're in a wrapped
    // library function, and it should give us the address of the
    // call.
    addStmtToIRSB(sbOut,
                  IRStmt_Store(ENDIAN,
                               mkU64((uintptr_t)&last_abi_addr),
                               mkU64((uintptr_t)stAddr)));
    break;
  case Ist_Put:
    // Here we'll want to instrument moving Shadow values into
    // thread state. In flattened IR, these shadow values should
    // always come from temporaries.
    addStmtToIRSB(sbOut, st);

    expr = st->Ist.Put.data;
    switch (expr->tag) {
    case Iex_RdTmp:
      if (isFloat(sbOut->tyenv, expr->Iex.RdTmp.tmp)){
        // Okay, in this one we're reading from a temp instead of the
        // thread state, but otherwise it's pretty much like above.
        copyShadowLocation =
          unsafeIRDirty_0_N(2,
                            "copyShadowTmptoTS",
                            VG_(fnptr_to_fnentry)(&copyShadowTmptoTS),
                            mkIRExprVec_2(// The number of the temporary
                                          mkU64(expr->Iex.RdTmp.tmp),
                                          // The thread state offset,
                                          // as above.
                                          mkU64(st->Ist.Put.offset)));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      }
      break;
    case Iex_Const:
      break;
    default:
      // This shouldn't happen in flattened IR.
      VG_(dmsg)("\
A non-constant or temp is being placed into thread state in a single IR statement! \
That doesn't seem flattened...\n");
      break;
    }
    break;
  case Ist_PutI:
    // This will look a lot like above, but we have to deal with not
    // knowing at compile time which piece of thread state we're
    // putting into. This will probably involve putting more burden
    // on the runtime c function which we'll insert after the put to
    // process it.
    addStmtToIRSB(sbOut, st);

    expr = st->Ist.PutI.details->data;
    switch (expr->tag) {
    case Iex_RdTmp:
      if (isFloat(sbOut->tyenv, expr->Iex.RdTmp.tmp)){
        copyShadowLocation =
          unsafeIRDirty_0_N(
                            2,
                            "copyShadowTmptoTS",
                            VG_(fnptr_to_fnentry)(&copyShadowTmptoTS),
                            mkIRExprVec_2(
                                          mkU64(expr->Iex.RdTmp.tmp),
                                          // Calculate array_base + (ix + bias) %
                                          // array_len at run time. This will give us
                                          // the offset into the thread state at which
                                          // the actual get is happening, so we can
                                          // use that same offset for the shadow get.
                                          mkArrayLookupExpr(st->Ist.PutI.details->descr->base,
                                                            st->Ist.PutI.details->ix,
                                                            st->Ist.PutI.details->bias,
                                                            st->Ist.PutI.details->descr->nElems,
                                                            sbOut)));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      }
      break;
    case Iex_Const:
      break;
    default:
      // This shouldn't happen in flattened IR.
      VG_(dmsg)("\
A non-constant or temp is being placed into thread state in a single IR statement! \
That doesn't seem flattened...\n");
      break;
    }
    break;
  case Ist_WrTmp:
    // Here we'll instrument moving Shadow values into temps. See
    // above.
    addStmtToIRSB(sbOut, st);
    if (!isFloat(sbOut->tyenv, st->Ist.WrTmp.tmp)) break;
    expr = st->Ist.WrTmp.data;
    switch(expr->tag) {
    case Iex_Get:
      copyShadowLocation =
        unsafeIRDirty_0_N(3,
                          "copyShadowTStoTmp",
                          VG_(fnptr_to_fnentry)(&copyShadowTStoTmp),
                          mkIRExprVec_3(mkU64(expr->Iex.Get.offset),
                                        mkU64(expr->Iex.Get.ty),
                                        mkU64(st->Ist.WrTmp.tmp)));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      break;
    case Iex_GetI:
      // See comments above on PutI to make sense of this thing.
      copyShadowLocation =
        unsafeIRDirty_0_N(3,
                          "copyShadowTStoTmp",
                          VG_(fnptr_to_fnentry)(&copyShadowTStoTmp),
                          mkIRExprVec_3(mkArrayLookupExpr(expr->Iex.GetI.descr->base,
                                                          expr->Iex.GetI.ix,
                                                          expr->Iex.GetI.bias,
                                                          expr->Iex.GetI.descr->nElems,
                                                          sbOut),
                                        mkU64(expr->Iex.Get.ty),
                                        mkU64(st->Ist.WrTmp.tmp)));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      break;
    case Iex_RdTmp:
      copyShadowLocation =
        unsafeIRDirty_0_N(2,
                          "copyShadowTmptoTmp",
                          VG_(fnptr_to_fnentry)(&copyShadowTmptoTmp),
                          mkIRExprVec_2(mkU64(expr->Iex.RdTmp.tmp),
                                        mkU64(st->Ist.WrTmp.tmp)));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      break;
    case Iex_ITE:
      {
        // When we have an ITE, we want to transfer across the shadow
        // value for one temp if the cond is true, and the other if
        // the cond is false. We do that by branching on the cond, and
        // assigning the temp number of the temp we want to transfer
        // from to condTmp. This means that condTmp is a temp that
        // stores a temp. Or, at least, a reference to one.
        IRTemp condTmp = newIRTemp(sbOut->tyenv, Ity_I64);
        IRStmt* pickTmp = IRStmt_WrTmp(condTmp,
                                       IRExpr_ITE(expr->Iex.ITE.cond,
                                                  // The branches of
                                                  // the "if" should
                                                  // be temps, since
                                                  // the IR is
                                                  // flattened at
                                                  // instrumentation
                                                  // time. If they
                                                  // aren't, we're in
                                                  // trouble.
                                                  mkU64(expr->Iex.ITE.iftrue->Iex.RdTmp.tmp),
                                                  mkU64(expr->Iex.ITE.iffalse->Iex.RdTmp.tmp)));
        addStmtToIRSB(sbOut, pickTmp);

        // Once we have the temp we want to transfer from in condTmp,
        // we can call our shadow value transfering function for temps
        // like normal.
        copyShadowLocation =
          unsafeIRDirty_0_N(2,
                            "copyShadowTmptoTmp",
                            VG_(fnptr_to_fnentry)(&copyShadowTmptoTmp),
                            mkIRExprVec_2(mkU64(condTmp),
                                          mkU64(st->Ist.WrTmp.tmp)));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      }
      break;
    case Iex_Load:
      if (isFloatType(expr->Iex.Load.ty)){
        copyShadowLocation =
          unsafeIRDirty_0_N(3,
                            "copyShadowMemtoTmp",
                            VG_(fnptr_to_fnentry)(&copyShadowMemtoTmp),
                            mkIRExprVec_3(expr->Iex.Load.addr,
                                          mkU64(expr->Iex.Load.ty),
                                          mkU64(st->Ist.WrTmp.tmp)));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      }
      break;
    case Iex_Qop:
    case Iex_Triop:
    case Iex_Binop:
    case Iex_Unop:
      instrumentOp(sbOut, st->Ist.WrTmp.tmp, expr, stAddr);
      break;
      // We don't have to do anything for constants, since a
      // constant isn't considered a float yet.
    case Iex_Const:
      // There's really nothing we can do about opaque pure C calls,
      // so we'll skip them.
    case Iex_CCall:
      break;
    default:
      // This shouldn't happen in flattened IR.
      VG_(dmsg)("We don't recognize this expression! It's type is %s.\n", IRExprTagString(expr->tag));
      break;
    }
    break;
  case Ist_Store:
    // Here we'll instrument moving Shadow values into memory,
    // unconditionally.
    addStmtToIRSB(sbOut, st);
    expr = st->Ist.Store.data;
    switch (expr->tag) {
    case Iex_RdTmp:
      if (isFloat(sbOut->tyenv, expr->Iex.RdTmp.tmp)){
        copyShadowLocation =
          unsafeIRDirty_0_N(2,
                            "copyShadowTmptoMem",
                            VG_(fnptr_to_fnentry)(&copyShadowTmptoMem),
                            mkIRExprVec_2(mkU64(expr->Iex.RdTmp.tmp),
                                          st->Ist.Store.addr));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      }
      break;
    case Iex_Const:
      break;
    default:
      // This shouldn't happen in flattened IR.
      VG_(dmsg)("\
A non-constant or temp is being placed into memory in a single IR statement! \
That doesn't seem flattened...\n");
      break;
    }
    break;
  case Ist_StoreG:
    // Same as above, but only assigns the value to memory if a
    // guard returns true.
    addStmtToIRSB(sbOut, st);
    expr = st->Ist.Store.data;
    switch(expr->tag) {
    case Iex_RdTmp:
      if (isFloat(sbOut->tyenv, expr->Iex.RdTmp.tmp)){
        copyShadowLocation =
          unsafeIRDirty_0_N(3,
                            "copyShadowTmptoMemG",
                            VG_(fnptr_to_fnentry)(&copyShadowTmptoMemG),
                            mkIRExprVec_3(st->Ist.StoreG.details->guard,
                                          mkU64(expr->Iex.RdTmp.tmp),
                                          st->Ist.StoreG.details->addr));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      }
      break;
    case Iex_Const:
      break;
    default:
      // This shouldn't happen in flattened IR.
      VG_(dmsg)("\
A non-constant or temp is being placed into memory in a single IR statement! \
That doesn't seem flattened...\n");
      break;
    }
    break;
  case Ist_LoadG:
    // Guarded load. This will load a value from memory, and write
    // it to a temp, but only if a condition returns true.
    addStmtToIRSB(sbOut, st);
    if (isFloat(sbOut->tyenv, st->Ist.LoadG.details->dst)){
      ALLOC(loadGInfo, "hg.loadGmalloc.1", 1, sizeof(LoadG_Info));
      // These are the lines we'd like to write. Unfortunately, we
      // can't, because these values in theory are not known until the
      // block is run. So, we're going to do the same thing, but at
      // runtime, by inserting store instructions.

      /* loadGInfo->cond = st->Ist.LoadG.details->guard; */
      /* loadGInfo->src_mem = st->Ist.LoadG.details->addr; */
      /* loadGInfo->alt_tmp = st->Ist.LoadG.details->alt; */

      addStmtToIRSB(sbOut, IRStmt_Store(ENDIAN, mkU64((uintptr_t)&(loadGInfo->cond)),
                                        st->Ist.LoadG.details->guard));
      addStmtToIRSB(sbOut, IRStmt_Store(ENDIAN, mkU64((uintptr_t)&(loadGInfo->src_mem)),
                                        st->Ist.LoadG.details->addr));
      addStmtToIRSB(sbOut, IRStmt_Store(ENDIAN, mkU64((uintptr_t)&(loadGInfo->alt_tmp)),
                                        st->Ist.LoadG.details->alt));
      loadGInfo->dest_tmp = st->Ist.LoadG.details->dst;
      loadGInfo->dest_type = typeOfIRTemp(sbOut->tyenv, st->Ist.LoadG.details->dst);

      copyShadowLocation =
        unsafeIRDirty_0_N(1,
                          "copyShadowMemtoTmpIf",
                          VG_(fnptr_to_fnentry)(&copyShadowMemtoTmpIf),
                          mkIRExprVec_1(mkU64((uintptr_t)loadGInfo)));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
    }
    break;
  case Ist_CAS:
    // This is an atomic compare and swap operation. Basically, has
    // three parts: a destination, a value address, an expected
    // value, and a result value. If the value at the value address
    // is equal to the expected value, then the result value is
    // stored in the destination temp.

    // TODO: Add something here if we ever want to support multithreading.

    addStmtToIRSB(sbOut, st);
    VG_(dmsg)("\
Warning! Herbgrind does not currently support the Compare and Swap instruction, \
because we don't support multithreaded programs.\n");
    break;
  case Ist_LLSC:
    // I honestly have no goddamn idea what this does. See: libvex_ir.h:2816

    // TODO: Add something here if we ever want to support multithreading.

    addStmtToIRSB(sbOut, st);
    VG_(dmsg)("\
Warning! Herbgrind does not currently support the Load Linked / Store Conditional \
set of instructions, because we don't support multithreaded programs.\n");
    break;
  case Ist_Dirty:
    // Call a C function, possibly with side affects. The possible
    // side effects should be denoted in the attributes of this
    // instruction.
    addStmtToIRSB(sbOut, st);
    break;
  }
}

// Produce an expression to calculate (base + ((idx + bias) % len)),
// where base, bias, and len are fixed, and idx can vary at runtime.
// The type of the resulting expression is Ity_I32.
IRExpr* mkArrayLookupExpr(Int base, IRExpr* idx, Int bias, Int len, IRSB* sbOut){
  // Set op the temps to hold all the different intermediary values.
  IRTemp idxPLUSbias = newIRTemp(sbOut->tyenv, Ity_I32);
  IRTemp idxPLUSbias64 = newIRTemp(sbOut->tyenv, Ity_I64);
  IRTemp idxPLUSbiasDIVMODlen = newIRTemp(sbOut->tyenv, Ity_I64);
  IRTemp idxPLUSbiasMODlen = newIRTemp(sbOut->tyenv, Ity_I32);
  IRTemp idxPLUSbiasMODlenPLUSbase = newIRTemp(sbOut->tyenv, Ity_I32);
  IRTemp resultWidened = newIRTemp(sbOut->tyenv, Ity_I64);

  // idx + bias
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(idxPLUSbias,
                             IRExpr_Binop(Iop_Add32,
                                          idx,
                                          mkU32(bias))));
  // We need to convert to 64 bits so we can do the mod.
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(idxPLUSbias64,
                             IRExpr_Unop(Iop_32Uto64,
                                         IRExpr_RdTmp(idxPLUSbias))));
  // These two operations together are mod.
  // (idx + bias) % len
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(idxPLUSbiasDIVMODlen,
                             IRExpr_Binop(Iop_DivModU64to32,
                                          IRExpr_RdTmp(idxPLUSbias64),
                                          mkU32(len))));
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(idxPLUSbiasMODlen,
                             IRExpr_Unop(Iop_64HIto32,
                                         IRExpr_RdTmp(idxPLUSbiasDIVMODlen))));
  // base + ((idx + bias) % len)
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(idxPLUSbiasMODlenPLUSbase,
                             IRExpr_Binop(Iop_Add32,
                                          IRExpr_RdTmp(idxPLUSbiasMODlen),
                                          mkU32(base))));
  // Finally, widen the result.
  addStmtToIRSB(sbOut,
                IRStmt_WrTmp(resultWidened,
                             IRExpr_Unop(Iop_32Uto64,
                                          IRExpr_RdTmp(idxPLUSbiasMODlenPLUSbase))));

  return IRExpr_RdTmp(resultWidened);
}

void startBlock(IRSB* sbOut){
  IRDirty* initBlockCall =
    unsafeIRDirty_0_N(0, "initBlock", VG_(fnptr_to_fnentry)(&initBlock), mkIRExprVec_0());
  addStmtToIRSB(sbOut, IRStmt_Dirty(initBlockCall));
}

void finalizeBlock(IRSB* sbOut){

  // Finalize the block
  IRDirty* cleanupTemps =
    unsafeIRDirty_0_N(0, "cleanupBlock", VG_(fnptr_to_fnentry)(&cleanupBlock), mkIRExprVec_0());
  addStmtToIRSB(sbOut, IRStmt_Dirty(cleanupTemps));  
}

// A bit misleading of a name, but returns whether the temp COULD
// contain a float that we care about. It might also contain a
// vectorized integer thing.
int isFloat(IRTypeEnv* env, IRTemp temp){
  IRType type = typeOfIRTemp(env, temp);
  return isFloatType(type);
}
int isFloatType(IRType type){
  return type == Ity_F32 || type == Ity_F64
    || type == Ity_V128 || type == Ity_I32
    || type == Ity_I64;
}
