#include "hg_instrument.h"

void instrumentStatement(IRStmt* st, IRSB* sbOut){
  IRExpr* expr;
  IRDirty* copyShadowLocation;

  switch (st->tag) {
    // If it's a no op, or just metadata, we'll just pass it into
    // the result IR.
  case Ist_NoOp:
  case Ist_IMark:
  case Ist_AbiHint:
    // If it's a memory bus event or an exit, we shouldn't have to
    // do much with it either.
  case Ist_MBE:
  case Ist_Exit:
    addStmtToIRSB(sbOut, st);
    break;
  case Ist_Put:
    // Here we'll want to instrument moving Shadow values into
    // thread state. In flattened IR, these shadow values should
    // always come from temporaries.
    expr = st->Ist.Put.data;
    addStmtToIRSB(sbOut, st);
    switch (expr->tag) {
    case Iex_RdTmp:
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
    expr = st->Ist.PutI.details->data;
    addStmtToIRSB(sbOut, st);
    switch (expr->tag) {
    case Iex_RdTmp:
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
                                                          st->Ist.PutI.details->descr->nElems)));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
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
    expr = st->Ist.WrTmp.data;
    switch(expr->tag) {
    case Iex_Get:
      copyShadowLocation =
        unsafeIRDirty_0_N(2,
                          "copyShadowTStoTmp",
                          VG_(fnptr_to_fnentry)(&copyShadowTStoTmp),
                          mkIRExprVec_2(mkU64(expr->Iex.Get.offset),
                                        mkU64(st->Ist.WrTmp.tmp)));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
      break;
    case Iex_GetI:
      // See comments above on PutI to make sense of this thing.
      copyShadowLocation =
        unsafeIRDirty_0_N(2,
                          "copyShadowTStoTmp",
                          VG_(fnptr_to_fnentry)(&copyShadowTStoTmp),
                          mkIRExprVec_2(mkArrayLookupExpr(expr->Iex.GetI.descr->base,
                                                          expr->Iex.GetI.ix,
                                                          expr->Iex.GetI.bias,
                                                          expr->Iex.GetI.descr->nElems),
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
      copyShadowLocation =
        unsafeIRDirty_0_N(2,
                          "copyShadowTmptoTmp",
                          VG_(fnptr_to_fnentry)(&copyShadowTmptoTmp),
                          mkIRExprVec_2(IRExpr_ITE(expr->Iex.ITE.cond,
                                                   // The branches
                                                   // of the "if"
                                                   // should be
                                                   // temps, since
                                                   // the IR is
                                                   // flattened at
                                                   // instrumentation
                                                   // time. If they
                                                   // aren't, we're
                                                   // in trouble.
                                                   mkU64(expr->Iex.ITE.iftrue->Iex.RdTmp.tmp),
                                                   mkU64(expr->Iex.ITE.iffalse->Iex.RdTmp.tmp)),
                                        mkU64(st->Ist.WrTmp.tmp)));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
    case Iex_Load:
      copyShadowLocation =
        unsafeIRDirty_0_N(2,
                          "copyShadowMemtoTmp",
                          VG_(fnptr_to_fnentry)(&copyShadowMemtoTmp),
                          mkIRExprVec_2(expr->Iex.Load.addr,
                                        mkU64(st->Ist.WrTmp.tmp)));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
    case Iex_Qop:
    case Iex_Triop:
    case Iex_Binop:
    case Iex_Unop:
      instrumentOp(sbOut, st->Ist.WrTmp.tmp, expr);
      break;
      // We don't have to do anything for constants, since a
      // constant isn't considered a float yet.
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
  case Ist_Store:
    // Here we'll instrument moving Shadow values into memory,
    // unconditionally.
    addStmtToIRSB(sbOut, st);
    expr = st->Ist.Store.data;
    switch (expr->tag) {
    case Iex_RdTmp:
      copyShadowLocation =
        unsafeIRDirty_0_N(2,
                          "copyShadowTmptoMem",
                          VG_(fnptr_to_fnentry)(&copyShadowTmptoMem),
                          mkIRExprVec_2(mkU64(expr->Iex.RdTmp.tmp),
                                        st->Ist.Store.addr));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
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
  case Ist_StoreG:
    // Same as above, but only assigns the value to memory if a
    // guard returns true.
    addStmtToIRSB(sbOut, st);
    expr = st->Ist.Store.data;
    switch(expr->tag) {
    case Iex_RdTmp:
      copyShadowLocation =
        unsafeIRDirty_0_N(3,
                          "copyShadowTmptoMemG",
                          VG_(fnptr_to_fnentry)(&copyShadowTmptoMemG),
                          mkIRExprVec_3(st->Ist.StoreG.details->guard,
                                        mkU64(expr->Iex.RdTmp.tmp),
                                        st->Ist.Store.addr));
      addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
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
  case Ist_LoadG:
    // Guarded load. This will load a value from memory, and write
    // it to a temp, but only if a condition returns true.
    addStmtToIRSB(sbOut, st);
    copyShadowLocation =
      unsafeIRDirty_0_N(4,
                        "copyShadowMemtoTmpIf",
                        VG_(fnptr_to_fnentry)(&copyShadowMemtoTmpIf),
                        mkIRExprVec_4(st->Ist.LoadG.details->guard,
                                      st->Ist.LoadG.details->addr,
                                      st->Ist.LoadG.details->alt,
                                      mkU64(st->Ist.LoadG.details->dst)));
    addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowLocation));
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
because we don't support multithreaded programs.");
    break;
  case Ist_LLSC:
    // I honestly have no goddamn idea what this does. See: libvex_ir.h:2816

    // TODO: Add something here if we ever want to support multithreading.

    addStmtToIRSB(sbOut, st);
    VG_(dmsg)("\
Warning! Herbgrind does not currently support the Load Linked / Store Conditional \
set of instructions, because we don't support multithreaded programs.");
    break;
  case Ist_Dirty:
    // Call a C function, possibly with side affects. The possible
    // side effects should be denoted in the attributes of this
    // instruction.
    addStmtToIRSB(sbOut, st);
    VG_(dmsg)("\
There's a dirty function call in the tool input! That can't be right...");
    break;
  }
}
// Add instrumenting expressions to sb for an operation, storing the
// result in the temporary at offset.
void instrumentOp(IRSB* sb, Int offset, IRExpr* expr){
  IRDirty* executeShadowOp;

  switch (expr->tag){
  case Iex_Unop:
    executeShadowOp =
      unsafeIRDirty_0_N(3,
                        "executeUnaryShadowOp",
                        VG_(fnptr_to_fnentry)(&executeUnaryShadowOp),
                        mkIRExprVec_3(mkU64(expr->Iex.Unop.op),
                                      mkU64(expr->Iex.Unop.arg->Iex.RdTmp.tmp),
                                      mkU64(offset)));
    addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp));
    break;
  case Iex_Binop:
    executeShadowOp =
      unsafeIRDirty_0_N(4,
                        "executeBinaryShadowOp",
                        VG_(fnptr_to_fnentry)(&executeBinaryShadowOp),
                        mkIRExprVec_4(mkU64(expr->Iex.Binop.op),
                                      mkU64(expr->Iex.Binop.arg1->Iex.RdTmp.tmp),
                                      mkU64(expr->Iex.Binop.arg2->Iex.RdTmp.tmp),
                                      mkU64(offset)));
    addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp));
    break;
  case Iex_Triop:
    executeShadowOp =
      unsafeIRDirty_0_N(5,
                        "executeTriShadowOp",
                        VG_(fnptr_to_fnentry)(&executeTriShadowOp),
                        mkIRExprVec_5(mkU64(expr->Iex.Triop.details->op),
                                      mkU64(expr->Iex.Triop.details->arg1->Iex.RdTmp.tmp),
                                      mkU64(expr->Iex.Triop.details->arg2->Iex.RdTmp.tmp),
                                      mkU64(expr->Iex.Triop.details->arg3->Iex.RdTmp.tmp),
                                      mkU64(offset)));
    addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp));
    break;
  case Iex_Qop:
    executeShadowOp =
      unsafeIRDirty_0_N(6,
                        "executeQuadShadowOp",
                        VG_(fnptr_to_fnentry)(&executeQuadShadowOp),
                        mkIRExprVec_6(mkU64(expr->Iex.Qop.details->op),
                                      mkU64(expr->Iex.Qop.details->arg1->Iex.RdTmp.tmp),
                                      mkU64(expr->Iex.Qop.details->arg2->Iex.RdTmp.tmp),
                                      mkU64(expr->Iex.Qop.details->arg3->Iex.RdTmp.tmp),
                                      mkU64(expr->Iex.Qop.details->arg4->Iex.RdTmp.tmp),
                                      mkU64(offset)));
    addStmtToIRSB(sb, IRStmt_Dirty(executeShadowOp));
    break;
  default:
    VG_(dmsg)("BAD THINGS!!!!\n");
  }
}

// Produce an expression to calculate (base + ((idx + bias) % len)),
// where base, bias, and len are fixed, and idx can vary at runtime.
IRExpr* mkArrayLookupExpr(Int base, IRExpr* idx, Int bias, Int len){
  return IRExpr_Binop(// +
                      Iop_Add64,
                      // base
                      mkU64(base),
                      // These two ops together are %
                      IRExpr_Unop(Iop_64HIto32,
                      IRExpr_Binop(Iop_DivModU64to32,
                                   IRExpr_Binop(// +
                                                Iop_Add64,
                                                idx,
                                                mkU64(bias)),
                                   mkU64(len))));
}

