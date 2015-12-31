
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie              hg_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for Herbie, which
   is mostly for experimenting with the valgrind interface with
   respect to measuring the accuracy of binary floating point
   programs.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
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

#include "hg_include.h"

// Whether or not the tool is currently "turned on".
static int running = 0;

// Here are the data structures we set up to hold shadow values. They
// take three forms:
//
// * Values that are being worked on currently are held in temps by
//   VEX, so we have an array of shadow values to match each temp
//   register, up to a limit set in the .h file.
//
// * Values that persist between blocks (I think this is how it
//   works), are held in a per thread data structure by VEX, so we set
//   up another array for every thread to hold those, also up to a
//   limit set in the .h file.
//
// * Finally, values might be written to memory, and then read out
//   later at some arbitrary point. For these, we'll maintain a hash
//   table that maps addresses to shadow values, so we don't have to
//   maintain a vast array of shadow values for all of memory.
static ShadowValue* localTemps[MAX_TEMPS];
static ShadowValue* threadRegisters[VG_N_THREADS][MAX_REGISTERS];
static VgHashTable globalMemory = NULL;

// This is where the magic happens. This function gets called to
// instrument every superblock.
static
IRSB* hg_instrument ( VgCallbackClosure* closure,
                      IRSB* bb,
                      const VexGuestLayout* layout, 
                      const VexGuestExtents* vge,
                      const VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy )
{
  // For right now, just print out the VEX representation as we
  // process it.
  if (running == 0) return bb;
  VG_(printf)("Instrumenting block:\n");
  for(int i = 0; i < bb->stmts_used; i++){
    IRStmt* st = bb->stmts[i];
    ppIRStmt(st);
    VG_(printf)("\n");
  }
  VG_(printf)("\n");

  // Let's do some light instrumentation!

  // First, we'll set up a data structure to hold our instrumented IR.
  // We'll copy the typing environment, and the next block to jump to,
  // as well as some info about the exit jump, from the old superblock.
  IRSB* sbOut = deepCopyIRSBExceptStmts(bb);

  // Now, let's loop through these statements, and instrument them to
  // add our shadow values.
  for (int i = 0; i < bb->stmts_used; i++){
    IRStmt* st = bb->stmts[i];

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
      // thread state. These can either be direct copies from other
      // shadow value locations, or they can involve doing an MPFR
      // calculation, depending on what sort of expression we're
      // getting from.
      IRExpr* expr = st->Put.data;
      addStmtToIRSB(sbOut, st);
      switch (expr->tag) {
      case Iex_Binder:
        // As far as I can tell, we're never supposed to see this
        // here, and it's not really documented. So, if we get it,
        // let's just throw an error and then pass it through.
        VG_(dmsg)("We hit a binder while processing VEX. This should never happen!\n");
        break;
      case Iex_Get:
        // This case is simply getting a value from one part of the
        // guest state, and putting it into another. Transferring
        // shadow values in this case is about as simple as it gets.
        IRDirty* copyShadowValue =
          unsafeIRDirty_0_N(// The number of arguments to
                            // copyShadowValue.
                            2, 
                            // The name of copyShadowValue, used for
                            // printing.
                            "copyShadowTStoTS",
                            // The actual function. This helper
                            // function is usually a noop, but can do
                            // something on weird architectures.
                            VG_(fnptr_to_fnentry)(&copyShadowTStoTS),
                            // Finally, the two arguments. Since both
                            // our source and destination are at
                            // constant offsets in the thread state in
                            // this case, we just pass source offset
                            // and destination offset.
                            mkIRExprVec_2(mkU64(expr->Get.offset),
                                          mkU64(st->Put.offset)));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowValue));
        break;
      case Iex_GetI:
        // Mostly like the above, but the location that we're getting
        // from is calculated at run time.
        IRDirty* copyShadowValue =
          unsafeIRDirty_0_N(2,
                            "copyShadowTStoTS",
                            VG_(fnptr_to_fnentry)(&copyShadowTStoTS),
                            mkIRExprVec_2(// Calculate array_base +
                                          // (ix + bias) % array_len
                                          // at run time. This will
                                          // give us the offset into
                                          // the thread state at which
                                          // the actual get is
                                          // happening, so we can use
                                          // that same offset for the
                                          // shadow get.
                                          IRExpr_Binop( // +
                                                       Iop_Add64,
                                                       // array_base
                                                       mkU64(expr->GetI.descr->base),
                                                       // These two ops together are %
                                                       IRExpr_Unop(Iop_64HIto32,
                                                                   IRExpr_Binop(IOp_DivModU64to32,
                                                                                // +
                                                                                IRExpr_Binop(IOp_Add64,
                                                                                             // ix
                                                                                             //
                                                                                             // This
                                                                                             // is
                                                                                             // the
                                                                                             // only
                                                                                             // part
                                                                                             // that's
                                                                                             // not
                                                                                             // constant.
                                                                                             expr->GetI.ix,
                                                                                             // bias
                                                                                             mkU64(expr->GetI.bias)),
                                                                                // array_len
                                                                                mkU64(expr->GetI.descr->nElems)))),
                                          mkU64(st->Put.offset)));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowValue));
        break;
      case Iex_RdTmp:
        // Okay, in this one we're reading from a temp instead of the
        // thread state, but otherwise it's pretty much like above.
        IRDirty* copyShadowValue =
          unsafeIRDirty_0_N(2,
                            "copyShadowTmptoTS",
                            VG_(fnptr_to_fnentry)(&copyShadowTmptoTS),
                            mkIRExprVec_2(// The number of the temporary
                                          mkU64(expr->RdTmp.tmp),
                                          // The thread state offset,
                                          // as above.
                                          mkU64(st->Put.offset)));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowValue));
        break;
      case Iex_Qop:
      case Iex_Triop:
      case Iex_Binop:
      case Iex_Unop:
        instrumentOpPut(sbOut, offset, expr);
        break;
      case Iex_Load:
        IRDirty* copyShadowValue =
          unsafeIRDirty_0_N(2,
                            "copyShadowMemtoTS",
                            VG_(fnptr_to_fnentry)(&copyShadowMemtoTS),
                            mkIRExprVec_2(addr,
                                          mkU64(st->Put.offset)));
        addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowValue));
        break;
        // Pure function calls shouldn't show up in the input, from
        // what I know, so we don't have to do anything for them. If
        // they did show up in the input, there isn't really anything
        // we can do with them anyway, since they're so opaque about
        // what they're doing.
      case Iex_CCall:
        VG_(dmsg)("There's a pure c call in our input! I wonder why...\n");
        // We don't need to instrument constants being loaded in,
        // because we're not going to start treating a value like a
        // float until a float operation happens to it. We follow
        // FpDebug in this regard.
      case Iex_Const:
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
      break;
    case Ist_WrTmp:
      // Here we'll instrument moving Shadow values into temps. See
      // above.
      addStmtToIRSB(sbOut, st);
      break;
    case Ist_Store:
      // Here we'll instrument moving Shadow values into memory,
      // unconditionally.
      addStmtToIRSB(sbOut, st);
      break;
    case Ist_StoreG:
      // Same as above, but only assigns the value to memory if a
      // guard returns true.
      addStmtToIRSB(sbOut, st);
      break;
    case Ist_LoadG:
      // Guarded load. This will load a value from memory, and write
      // it to a temp, but only if a condition returns true.
      addStmtToIRSB(sbOut, st);
      break;
    case Ist_CAS:
      // This is an atomic compare and swap operation. Basically, has
      // three parts: a destination, a value address, an expected
      // value, and a result value. If the value at the value address
      // is equal to the expected value, then the result value is
      // stored in the destination temp.
      addStmtToIRSB(sbOut, st);
      break;
    case Ist_LLSC:
      // I honestly have no goddamn idea what this does. See: libvex_ir.h:2816
      addStmtToIRSB(sbOut, st);
      break;
    case Ist_Dirty:
      // Call a C function, possibly with side affects. The possible
      // side effects should be denoted in the attributes of this
      // instruction.
      addStmtToIRSB(sbOut, st);
      break;
    }
  }

  return sbOut;
}

// This handles client requests, the macros that client programs stick
// in to send messages to the tool.
static Bool hg_handle_client_request(ThreadId tid, UWord* arg, UWord* ret) {
  switch(arg[0]) {
  case VG_USERREQ__BEGIN:
    startHerbGrind();
    break;
  case VG_USERREQ__END:
    stopHerbGrind();
    break;
  }
  return False;
}

// This is called after the program exits, for cleanup and such.
static void hg_fini(Int exitcode){}
// This does any initialization that needs to be done after command
// line processing.
static void hg_post_clo_init(void){}

// This disables the instrumentation of this tool.
static void stopHerbGrind(void){ running = 0; }

// This enables the instrumentation of this tool.
static void startHerbGrind(void){ running = 1; }

// This is where we initialize everything
static void hg_pre_clo_init(void)
{
   VG_(details_name)            ("HerbGrind");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("a valgrind tool for Herbie");
   VG_(details_copyright_author)("");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );

   VG_(basic_tool_funcs)        (hg_post_clo_init,
                                 hg_instrument,
                                 hg_fini);

   VG_(needs_client_requests) (hg_handle_client_request);

   // Tell the gmp stuff to use valgrind c library instead of the
   // standard one for memory allocation and the like.
   mp_set_memory_functions(gmp_alloc, gmp_realloc, gmp_free);
   mpfr_set_strlen_function(VG_(strlen));
   mpfr_set_strcpy_function(VG_(strcpy));
   mpfr_set_memmove_function(VG_(memmove));
   mpfr_set_memcmp_function(VG_(memcmp));
   mpfr_set_memset_function(VG_(memset));

   // Set up the data structures we'll need to keep track of our MPFR
   // shadow values.
   VG(HT_construct)
}

VG_DETERMINE_INTERFACE_VERSION(hg_pre_clo_init)

// Some memory allocation functions for gmp support
static void* gmp_alloc(size_t t){ return VG_(malloc)("hg.gmp_alloc.1", t); }
static void* gmp_realloc(void* p, size_t t1, size_t t2){ return VG_(realloc)("hg.gmp_realloc.1", p, t1); }
static void gmp_free(void* p, size_t t){ return VG_(free)(p); }

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
