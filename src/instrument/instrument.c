/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie           instrument.c ---*/
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

#include "instrument.h"
#include "options.h"

#include "instrument-storage.h"
#include "instrument-op.h"

// Pull in this header file so that we can call the valgrind version
// of printf.
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_mallocfree.h"

#include "../runtime/value-shadowstate/shadowval.h"
#include "../runtime/value-shadowstate/value-shadowstate.h"

#include "../helper/instrument-util.h"
#include "../helper/debug.h"
#include "intercept-block.h"

// This is where the magic happens. This function gets called to
// instrument every superblock.
IRSB* hg_instrument (VgCallbackClosure* closure,
                     IRSB* sbIn,
                     const VexGuestLayout* layout,
                     const VexGuestExtents* vge,
                     const VexArchInfo* archinfo_host,
                     IRType gWordTy, IRType hWordTy) {
  IRSB* sbOut = deepCopyIRSBExceptStmts(sbIn);

  if (PRINT_IN_BLOCKS){
    VG_(printf)("Instrumenting block at %p:\n", (void*)closure->readdr);
    printSuperBlock(sbIn);
  }
  inferTypes(sbIn);
  if (PRINT_RUN_BLOCKS){
    char* blockMessage = VG_(perm_malloc)(35, 1);
    VG_(snprintf)(blockMessage, 35,
                  "Running block at %p\n", (void*)closure->readdr);
    addPrint(blockMessage);
  }
  IRExpr* blockStateDirtyExpr = runLoad64C(sbOut, &blockStateDirty);
  addAssertEQ(sbOut, "Uncleaned block!\n", blockStateDirtyExpr, mkU64(0));
  addStoreC(sbOut, mkU64(1), &blockStateDirty);

  Addr curAddr = 0;
  Addr prevAddr = -1;
  for(int i = 0; i < sbIn->stmts_used; ++i){
    IRStmt* stmt = sbIn->stmts[i];
    if (stmt->tag == Ist_IMark){
      prevAddr = curAddr;
      curAddr = stmt->Ist.IMark.addr;
      if (print_run_instrs){
        addPrint2("Running instruction at %lX\n", mkU64(curAddr));
      }
    }
    if (print_run_stmts){
      addPrint2("Running statement %d\n", mkU64(i));
    }
    if (curAddr){
      preInstrumentStatement(sbOut, stmt, curAddr, prevAddr);
    }
    addStmtToIRSB(sbOut, stmt);
    if (curAddr)
      instrumentStatement(sbOut, stmt,
                          curAddr, closure->readdr,
                          i, sbIn->stmts_used);
    if (print_run_stmts){
      addPrint2("Finished running statement %d\n", mkU64(i));
    }
  }
  finishInstrumentingBlock(sbOut);
  if (PRINT_BLOCK_BOUNDRIES){
    addPrint("\n+++++\n");
  }
  if (PRINT_OUT_BLOCKS){
    VG_(printf)("Printing out block:\n");
    printSuperBlock(sbOut);
  }
  return sbOut;
}

void init_instrumentation(void){
  initInstrumentationState();
}

void finish_instrumentation(void){
  cleanupTypeState();
}
void preInstrumentStatement(IRSB* sbOut, IRStmt* stmt, Addr stAddr, Addr prevAddr){
  switch(stmt->tag){
  case Ist_Exit:
    addBlockCleanupG(sbOut, stmt->Ist.Exit.guard);
    break;
  case Ist_AbiHint:
    if (stmt->Ist.AbiHint.nia->tag == Iex_Const &&
        stmt->Ist.AbiHint.nia->Iex.Const.con->tag == Ico_U64){

      maybeInterceptBlock(sbOut,
                          (void*)(uintptr_t)stmt->Ist.AbiHint.nia->Iex.Const.con->Ico.U64,
                          (void*)prevAddr);
    }
  default:
    break;
  }
}

void instrumentStatement(IRSB* sbOut, IRStmt* stmt,
                         Addr stAddr, Addr blockAddr,
                         int stIdx, int numStmtsIn){
  if (dummy){
    return;
  }
  switch(stmt->tag){
  case Ist_NoOp:
  case Ist_IMark:
  case Ist_MBE:
  case Ist_Exit:
  case Ist_AbiHint:
    break;
  case Ist_Put:
    instrumentPut(sbOut, stmt->Ist.Put.offset, stmt->Ist.Put.data,
                  stIdx);
    break;
  case Ist_PutI:
    instrumentPutI(sbOut,
                   stmt->Ist.PutI.details->ix,
                   stmt->Ist.PutI.details->bias,
                   stmt->Ist.PutI.details->descr->base,
                   stmt->Ist.PutI.details->descr->nElems,
                   stmt->Ist.PutI.details->descr->elemTy,
                   stmt->Ist.PutI.details->data,
                   stIdx);
    break;
  case Ist_WrTmp:
    {
      IRExpr* expr = stmt->Ist.WrTmp.data;
      switch(expr->tag){
      case Iex_Get:
        instrumentGet(sbOut,
                      stmt->Ist.WrTmp.tmp,
                      expr->Iex.Get.offset,
                      expr->Iex.Get.ty,
                      stIdx);
        break;
      case Iex_GetI:
        instrumentGetI(sbOut,
                       stmt->Ist.WrTmp.tmp,
                       expr->Iex.GetI.ix,
                       expr->Iex.GetI.bias,
                       expr->Iex.GetI.descr->base,
                       expr->Iex.GetI.descr->nElems,
                       expr->Iex.GetI.descr->elemTy,
                       stIdx);
        break;
      case Iex_RdTmp:
        instrumentRdTmp(sbOut,
                        stmt->Ist.WrTmp.tmp,
                        expr->Iex.RdTmp.tmp);
        break;
      case Iex_ITE:
        instrumentITE(sbOut,
                      stmt->Ist.WrTmp.tmp,
                      expr->Iex.ITE.cond,
                      expr->Iex.ITE.iftrue,
                      expr->Iex.ITE.iffalse);
        break;
      case Iex_Load:
        if (numStmtsIn < LOAD_FALLBACK_THRESHOLD){
          instrumentLoad(sbOut,
                         stmt->Ist.WrTmp.tmp,
                         expr->Iex.Load.addr,
                         expr->Iex.Load.ty);
        } else {
          instrumentLoadSmallButSlow(sbOut,
                                     stmt->Ist.WrTmp.tmp,
                                     expr->Iex.Load.addr,
                                     expr->Iex.Load.ty);
        }
        break;
      case Iex_Qop:
      case Iex_Triop:
      case Iex_Binop:
      case Iex_Unop:
        instrumentOp(sbOut,
                     stmt->Ist.WrTmp.tmp,
                     expr,
                     stAddr, blockAddr,
                     stIdx);
        break;
      case Iex_Const:
        instrumentWriteConst(sbOut,
                             stmt->Ist.WrTmp.tmp,
                             expr->Iex.Const.con);
        break;
      case Iex_CCall:
        break;
      default:
        tl_assert(0);
        break;
      }
    }
    break;
  case Ist_Store:
    instrumentStore(sbOut,
                    stmt->Ist.Store.addr,
                    stmt->Ist.Store.data);
    break;
  case Ist_StoreG:
    instrumentStoreG(sbOut,
                     stmt->Ist.StoreG.details->addr,
                     stmt->Ist.StoreG.details->guard,
                     stmt->Ist.StoreG.details->data);
    break;
  case Ist_LoadG:
    if (numStmtsIn < LOADG_FALLBACK_THRESHOLD){
      instrumentLoadG(sbOut,
                      stmt->Ist.LoadG.details->dst,
                      stmt->Ist.LoadG.details->alt,
                      stmt->Ist.LoadG.details->guard,
                      stmt->Ist.LoadG.details->addr,
                      stmt->Ist.LoadG.details->cvt);
    } else {
      instrumentLoadGSmallButSlow(sbOut,
                                  stmt->Ist.LoadG.details->dst,
                                  stmt->Ist.LoadG.details->alt,
                                  stmt->Ist.LoadG.details->guard,
                                  stmt->Ist.LoadG.details->addr,
                                  stmt->Ist.LoadG.details->cvt);
    }
    break;
  case Ist_CAS:
    instrumentCAS(sbOut,
                  stmt->Ist.CAS.details);
    break;
  case Ist_LLSC:
    VG_(dmsg)("Warning! Herbgrind does not currently support "
              "the Load Linked / Store Conditional set of "
              "instructions, because we don't support multithreaded "
              "programs.\n");
    break;
  case Ist_Dirty:
    break;
  default:
    tl_assert(0);
    break;
  }
}

void printSuperBlock(IRSB* superblock){
  for(int i = 0; i < superblock->stmts_used; i++){
    IRStmt* st = superblock->stmts[i];
    if (print_statement_numbers){
      VG_(printf)("%d: ", i);
    }
    ppIRStmt(st);
    VG_(printf)("\n");
  }
  VG_(printf)("\n");
}
