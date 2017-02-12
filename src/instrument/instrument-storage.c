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
#include "../runtime/shadowop/shadowop.h"
#include "../helper/instrument-util.h"
#include "../helper/debug.h"
#include "../options.h"

#include "pub_tool_libcprint.h"
#include "pub_tool_machine.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_xarray.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_threadstate.h"

XArray* tempDebt;
FloatType tempContext[MAX_TEMPS];
FloatType tsContext[MAX_REGISTERS];

void initInstrumentationState(void){
  tempDebt = VG_(newXA)(VG_(malloc), "temp debt array",
                        VG_(free), sizeof(IRTemp));
  initValueShadowState();
}

void instrumentRdTmp(IRSB* sbOut, IRTemp dest, IRTemp src){
  tl_assert2(typeOfIRTemp(sbOut->tyenv, dest) ==
             typeOfIRTemp(sbOut->tyenv, src),
             "Source of temp move doesn't match dest!");
  if (!canHaveShadow(sbOut->tyenv, IRExpr_RdTmp(src))){
    tempContext[dest] = tempContext[src];
    return;
  } else if (hasStaticShadow(IRExpr_RdTmp(src))){
    tempContext[dest] = tempType(src);
  } else {
    tempContext[dest] = Ft_Unknown;
  }
  // Load the new temp into memory.
  IRExpr* newShadowTemp = runLoadTemp(sbOut, src);

  // Copy across the new temp and increment it's ref count.
  // Increment the ref count of the new temp
  IRTemp newShadowTempCopy = newIRTemp(sbOut->tyenv, Ity_I64);
  IRDirty* copyShadowTempDirty =
    unsafeIRDirty_1_N(newShadowTempCopy,
                      1, "copyShadowTemp",
                      VG_(fnptr_to_fnentry)(&copyShadowTemp),
                      mkIRExprVec_1(newShadowTemp));
  copyShadowTempDirty->mFx = Ifx_Read;
  copyShadowTempDirty->mAddr = newShadowTemp;
  copyShadowTempDirty->mSize = sizeof(ShadowTemp);

  IRExpr* tempNonNull = runNonZeroCheck64(sbOut, newShadowTemp);
  copyShadowTempDirty->guard = tempNonNull;
  addStmtToIRSB(sbOut, IRStmt_Dirty(copyShadowTempDirty));
  addStoreTempG(sbOut, tempNonNull,
                IRExpr_RdTmp(newShadowTempCopy),
                tempContext[src], dest);
  if (print_temp_moves){
    addPrintG3(tempNonNull, "Copying shadow temp %p in %d ", newShadowTemp, mkU64(src));
    addPrintG3(tempNonNull, "to %p in %d\n",
               IRExpr_RdTmp(newShadowTempCopy), mkU64(dest));
  }
}
void instrumentWriteConst(IRSB* sbOut, IRTemp dest,
                          IRConst* con){
  tempContext[dest] = Ft_Unshadowed;
}
void instrumentITE(IRSB* sbOut, IRTemp dest,
                   IRExpr* trueExpr, IRExpr* falseExpr){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
  tempContext[dest] = Ft_Unshadowed;
}
void instrumentPut(IRSB* sbOut, Int tsDest, IRExpr* data){
  // This procedure adds instrumentation to sbOut which shadows the
  // putting of a value from a temporary into thread state.

  // To handle dealing with shadow thread state at runtime more
  // efficiently, we maintain a static record for each superblock of
  // possible states of thread state shadows. For each byte location
  // in thread state, we store whether at this point in the block,
  // it's definitely a float (single or double), it's definitely not a
  // float, or we don't know. This way at runtime we don't have to go
  // through the computation of clearing something which can't have
  // anything in it anyway. We're not going to presume to know
  // anything about thread state coming into this block, since block
  // entries might happen from a bunch of different contexts, and we
  // want to keep our analysis fairly simple. So all thread state
  // starts statically at the "havoc" value, Ft_Unknown.

  // The first thing we need to do is clear any existing shadow value
  // references from the threadstate we'll be overwriting.

  // Figure out how many thread state 4-byte units are being
  // overwritten. Note: because floats are always either 4 or 8 bytes,
  // and are always aligned to 4-byte boundries in thread state, we
  // can assume that all shadow values are 4-byte aligned in thread
  // state, and not touch the non-aligned bytes for anything.
  if (!canBeFloat(sbOut->tyenv, data)) return;
  int dest_size = exprSize(sbOut->tyenv, data);

  // Now, we'll overwrite those bytes.
  for(int i = 0; i < dest_size; ++i){
    Int dest_addr = tsDest + (i * sizeof(float));
    // If we know statically that the thread state cannot be a float
    // (meaning it's been overwritten by a non-float this block), then
    // we don't need to bother trying to clear it or change it's
    // static info here
    if (tsAddrCanHaveShadow(dest_addr)){
      IRExpr* oldVal = runGetTSVal(sbOut, dest_addr);
      // If we don't know whether or not it's a shadowed float at
      // runtime, we'll do a runtime check to see if there is a shadow
      // value there, and disown it if there is.
      if (tsHasStaticShadow(dest_addr)){
        if (print_value_moves){
          IRExpr* oldValRCount =
            runArrow(sbOut, oldVal, ShadowValue, ref_count);
          addPrint3("Disowning %p (old rc %d) "
                    "from thread state overwrite.\n",
                    oldVal, oldValRCount);
        }
        addSVDisownNonNull(sbOut, oldVal);
      } else {
        if (print_value_moves){
          IRExpr* oldValNonNull =
            runNonZeroCheck64(sbOut, oldVal);
          IRExpr* oldValRCount =
            runArrowG(sbOut, oldValNonNull, oldVal,
                      ShadowValue, ref_count);
          addPrintG3(oldValNonNull,
                     "Disowning %p (old rc %d) "
                     "from thread state overwrite.\n",
                     oldVal, oldValRCount);
        }
        addSVDisown(sbOut, oldVal);
      }
    }
  }
  if (!canHaveShadow(sbOut->tyenv, data)){
    for(int i = 0; i < dest_size; ++i){
      Int dest_addr = tsDest + (i * sizeof(float));
      if (canBeFloat(sbOut->tyenv, data)){
        addSetTSValUnshadowed(sbOut, dest_addr);
      } else {
        if (print_types){
          VG_(printf)("Setting TS(%d) to non-float, "
                      "because %d can't contain a float.\n",
                      dest_addr, data->Iex.RdTmp.tmp);
        }
        addSetTSValNonFloat(sbOut, dest_addr);
      }
    }
  } else {
    int idx = data->Iex.RdTmp.tmp;
    IRExpr* st = runLoadTemp(sbOut, idx);
    if (hasStaticShadow(data)){
      IRExpr* values =
        runArrow(sbOut, st, ShadowTemp, values);
      for(int i = 0; i < dest_size; ++i){
        Int dest_addr = tsDest + (i * sizeof(float));
        if (tempType(idx) == Ft_Double){
          if (i % 2 == 1){
            addSetTSValNonFloat(sbOut, dest_addr);
            if (print_types){
              VG_(printf)("Setting TS(%d) to non-float, "
                          "because we wrote a double "
                          "to the position before.\n",
                          dest_addr);
            }
          } else {
            IRExpr* value;
            if (i == 0){
              value = runLoad64(sbOut, values);
            } else {
              value =
                runIndex(sbOut, values, ShadowValue*, i / 2);
            }
            addSetTSValNonNull(sbOut, dest_addr, value, Ft_Double);

            if (print_value_moves){
              IRExpr* valRCount = runArrow(sbOut, value, ShadowValue,
                                           ref_count);
              addPrint3("Owning %p (new rc %lu) "
                        "as part of thread state put.\n",
                        value, valRCount);
            }
          }
        } else {
          tl_assert(tempType(idx) == Ft_Single);
          IRExpr* value =
            runIndex(sbOut, values, ShadowValue*, i);
          addSetTSValNonNull(sbOut, dest_addr, value, Ft_Single);

          if (print_value_moves){
            IRExpr* valRCount = runArrow(sbOut, value, ShadowValue,
                                         ref_count);
            addPrint3("Owning %p (new rc %lu) "
                      "as part of thread state put.\n",
                      value, valRCount);
          }
        }
      }
    } else {
      // Otherwise, we don't know whether or not there is a shadow
      // temp to be stored.
      IRExpr* stExists = runNonZeroCheck64(sbOut, st);
      // If the size of the value is 32-bits, then we know what type
      // of thing it is statically, so we can just pull out the values
      // much like above, except conditional on the whole thing not
      // being null.
      if (dest_size == 1) {
        IRExpr* values =
          runArrowG(sbOut, stExists, st, ShadowTemp, values);
        /* IRExpr* value = runIndexG(sbOut, stExists, values, ShadowValue*, 0); */
        IRExpr* value = runLoadG64(sbOut, values, stExists);

        addSVOwnNonNullG(sbOut, stExists, value);
        addSetTSVal(sbOut, tsDest, value);
        tsContext[tsDest] = Ft_Unknown;
      } else {
        // If it's 128-bits, and we don't have static info about it,
        // then it could either be two doubles or four singles, so
        // we're going to have to delay figuring out how to pull out
        // the individual values until runtime. Hopefully this case
        // should be pretty rare.
        for(int i = 0; i < 4; ++i){
          Addr dest_addr = tsDest + (i * sizeof(float));
          tsContext[dest_addr] = Ft_Unknown;
        }
        IRDirty* putDirty =
          unsafeIRDirty_0_N(2, "dynamicPut",
                            VG_(fnptr_to_fnentry)(dynamicPut),
                            mkIRExprVec_2(mkU64(tsDest), st));
        // We don't have to bother going into C if the value is null
        // at runtime.
        putDirty->guard = stExists;
        putDirty->mFx = Ifx_Modify;
        putDirty->mAddr = mkU64((uintptr_t)&(shadowThreadState
                                             [VG_(get_running_tid)()]
                                             [tsDest]));
        putDirty->mSize = sizeof(ShadowValue*) * 4;
        addStmtToIRSB(sbOut, IRStmt_Dirty(putDirty));
      }
    }
  }
}
void instrumentPutI(IRSB* sbOut,
                    IRExpr* varOffset, Int constOffset,
                    Int arrayBase, Int numElems, IRType elemType,
                    IRExpr* data){
  if (!canBeFloat(sbOut->tyenv, data)) return;
  int dest_size = exprSize(sbOut->tyenv, data);
  IRExpr* dest_addrs[4];
  // Because we don't know where in the fixed region of the array this
  // put will affect, we have to mark the whole array as unknown
  // statically. Well, except we know they are making well-aligned
  // rights because of how putI is calculated, so if we know they are
  // writing doubles, then we know there are no new floats in the odd
  // offsets.
  for(int i = 0; i < (numElems * dest_size); ++i){
    Int dest = arrayBase + i * 4;
    if (hasStaticShadow(data) &&
        tempType(data->Iex.RdTmp.tmp) == Ft_Double &&
        i % 2 == 1 &&
        tsContext[dest] == Ft_NonFloat){
      continue;
    }
    tsContext[dest] = Ft_Unknown;
  }

  for(int i = 0; i < dest_size; ++i){
    dest_addrs[i] =
      mkArrayLookupExpr(sbOut, arrayBase, varOffset,
                        constOffset + i, numElems, elemType);
    IRExpr* oldVal = runGetTSValDynamic(sbOut, dest_addrs[i]);
    addSVDisown(sbOut, oldVal);
  }
  if (!canHaveShadow(sbOut->tyenv, data)){
    for(int i = 0; i < dest_size; ++i){
      addSetTSValDynamic(sbOut, dest_addrs[i], mkU64(0));
    }
  } else {
    int tempIdx = data->Iex.RdTmp.tmp;
    IRExpr* st = runLoadTemp(sbOut, tempIdx);
    if (hasStaticShadow(data)){
      IRExpr* values =
        runArrow(sbOut, st, ShadowTemp, values);
      for(int i = 0; i < dest_size; ++i){
        if (tempType(tempIdx) == Ft_Double){
          if (i % 2 == 1){
            addSetTSValDynamic(sbOut, dest_addrs[i], mkU64(0));
          } else {
            IRExpr* value =
              runIndex(sbOut, values, ShadowValue*, i / 2);
            addSetTSValDynamic(sbOut, dest_addrs[i], value);
          }
        } else {
          IRExpr* value =
            runIndex(sbOut, values, ShadowValue*, i);
          addSetTSValDynamic(sbOut, dest_addrs[i], value);
        }
      }
    } else {
      IRExpr* stExists = runNonZeroCheck64(sbOut, st);
      if (dest_size == 1) {
        IRExpr* values =
          runArrowG(sbOut, stExists, st, ShadowTemp, values);
        IRExpr* value = runLoadG64(sbOut, values, stExists);
        addSVOwnNonNullG(sbOut, stExists, value);
        addSetTSValDynamic(sbOut, dest_addrs[0], value);
      } else {
        IRDirty* putDirty =
          unsafeIRDirty_0_N(2, "dynamicPut",
                            VG_(fnptr_to_fnentry)(dynamicPut),
                            mkIRExprVec_2(dest_addrs[0], st));
        putDirty->guard = stExists;
        putDirty->mFx = Ifx_Modify;
        putDirty->mAddr = mkU64((uintptr_t)&(shadowThreadState
                                             [VG_(get_running_tid)()]
                                             [arrayBase]));
        putDirty->mSize = numElems * sizeofIRType(elemType);
        addStmtToIRSB(sbOut, IRStmt_Dirty(putDirty));
      }
    }
  }
}
void instrumentGet(IRSB* sbOut, IRTemp dest,
                   Int tsSrc, IRType type){
  if (!canStoreShadow(sbOut->tyenv, IRExpr_RdTmp(dest))){
    return;
  }
  int src_size = typeSize(type);
  if (src_size == 1){
    FloatType valType = tsContext[tsSrc];
    // Getting the first half of a double is undefined.
    tl_assert(valType != Ft_Double);
    // If it's not a float propagate that information.
    if (valType == Ft_NonFloat){
      if (print_types){
        VG_(printf)("Marking %d as nonfloat because TS(%d) is nonfloat.\n",
                    dest, tsSrc);
      }
      addStoreTempNonFloat(sbOut, dest);
    } else if (valType == Ft_Unshadowed){
      addStoreTempUnshadowed(sbOut, dest);
    } else if (valType == Ft_Single) {
      // If we know it's a non-null single, then we can load it
      // unconditionally.
      IRExpr* val = runGetTSVal(sbOut, tsSrc);
      IRExpr* temp = runMkShadowTempValues(sbOut, 1, &val);
      addStoreTemp(sbOut, temp, Ft_Single, dest);
      if (print_temp_moves){
        addPrint3("1. Making %p in %d ", temp, mkU64(dest));
        addPrint2("for value from TS(%d)\n", mkU64(tsSrc));
      }
    } else if (valType == Ft_Unknown){
      IRExpr* loadedVal = runGetTSVal(sbOut, tsSrc);
      IRExpr* loadedValNonNull = runNonZeroCheck64(sbOut, loadedVal);
      IRExpr* temp = runMkShadowTempValuesG(sbOut, loadedValNonNull, 1,
                                            &loadedVal);
      addStoreTempUnknown(sbOut, temp, dest);
      if (print_temp_moves){
        IRExpr* loadedNonNull = runNonZeroCheck64(sbOut, temp);
        addPrintG3(loadedNonNull,
                   "2. Making %p in %d ",
                   temp, mkU64(dest));
        addPrintG2(loadedNonNull, "for value from TS(%d)\n",
                  mkU64(tsSrc));
      }
    }
  } else if (src_size == 2){
    FloatType valType = inferTSType64(tsSrc);
    if (valType == Ft_NonFloat){
      if (print_types){
        VG_(printf)("Marking %d as nonfloat because TS(%d) is nonfloat.\n",
                    dest, tsSrc);
      }
      addStoreTempNonFloat(sbOut, dest);
    } else if (valType == Ft_Unshadowed){
      addStoreTempUnshadowed(sbOut, dest);
    } else if (valType == Ft_Single){
      IRExpr* vals[2];
      for(int i = 0; i < 2; ++i){
        Int src_addr = tsSrc + (i * sizeof(float));
        if (tsContext[src_addr] == Ft_Single){
          vals[i] = runGetTSVal(sbOut, src_addr);
        } else if (tsContext[src_addr] == Ft_Unknown){
          IRExpr* loaded = runGetTSVal(sbOut, src_addr);
          IRExpr* loadedNull = runZeroCheck64(sbOut, loaded);
          IRExpr* valExpr = runF32toF64(sbOut, runGet64C(sbOut, src_addr));
          IRExpr* freshSV = runMkShadowValG(sbOut, loadedNull,
                                            Ft_Single, valExpr);
          vals[i] = runITE(sbOut, loadedNull, freshSV, loaded);
        } else {
          IRExpr* valExpr =
            runF32toF64(sbOut, runGet64C(sbOut, src_addr));
          IRExpr* freshSV =
            runMkShadowVal(sbOut, Ft_Single, valExpr);
          vals[i] = freshSV;
        }
      }
      IRExpr* temp = runMkShadowTempValues(sbOut, 2, vals);
      addStoreTemp(sbOut, temp, Ft_Single, dest);
      if (print_temp_moves){
        addPrint3("3. Making %p in %d ", temp, mkU64(dest));
        addPrint2("for value from TS(%d)\n", mkU64(tsSrc));
      }
    } else if (valType == Ft_Double){
      IRExpr* val = runGetTSVal(sbOut, tsSrc);
      IRExpr* temp = runMkShadowTempValues(sbOut, 1, &val);
      addStoreTemp(sbOut, temp, Ft_Double, dest);

      if (print_temp_moves){
        addPrint3("4. Making %p in %d ", temp, mkU64(dest));
        addPrint2("for value from TS(%d)\n", mkU64(tsSrc));
      }
    } else if (valType == Ft_Unknown){
      IRExpr* temp = runPureCCall64_2(sbOut, dynamicGet64,
                                      mkU64(tsSrc),
                                      runGet64C(sbOut, tsSrc));
      addStoreTempUnknown(sbOut, temp, dest);
      if (print_temp_moves){
        IRExpr* loadedNonNull = runNonZeroCheck64(sbOut, temp);
        addPrintG3(loadedNonNull,
                   "5. Making %p in %d ",
                   temp, mkU64(dest));
        addPrintG2(loadedNonNull, "for value from TS(%d)\n", mkU64(tsSrc));
      }
    }
  } else if (src_size == 4){
    FloatType valTypes[2];
    for(int i = 0; i < 2; ++i){
      valTypes[i] = inferTSType64(tsSrc + (i * sizeof(double)));
    }
    FloatType valType;
    if (valTypes[0] == Ft_Unknown){
      valType = valTypes[1];
    } else {
      valType = valTypes[0];
    }
    if (valType == Ft_NonFloat){
      addStoreTempNonFloat(sbOut, dest);
    } else if (valType == Ft_Unshadowed){
      addStoreTempUnshadowed(sbOut, dest);
    } else
    // For mismatched V128's, we're going to assume that the program
    // is only going to use the bottom half, so that's all we'll
    // load. This might not be sound, say if the user stores the
    // result back to memory/threadstate with the values copied as
    // part of the operation, so keep an eye out for this.
    if (valType == Ft_Single){
      IRExpr* vals[4];
      for(int i = 0; i < 4; ++i){
        Int src_addr = tsSrc + (i * sizeof(float));
        if (tsContext[src_addr] == Ft_Single){
          vals[i] = runGetTSVal(sbOut, src_addr);
        } else if (tsContext[src_addr] == Ft_Unknown){
          IRExpr* loaded = runGetTSVal(sbOut, src_addr);
          IRExpr* loadedNull = runZeroCheck64(sbOut, loaded);
          IRExpr* valExpr = runF32toF64(sbOut, runGet32C(sbOut, src_addr));
          IRExpr* freshSV = runMkShadowValG(sbOut, loadedNull,
                                            Ft_Single, valExpr);
          vals[i] = runITE(sbOut, loadedNull, freshSV, loaded);
        } else {
          IRExpr* valExpr =
            runF32toF64(sbOut, runGet32C(sbOut, src_addr));
          vals[i] = runMkShadowVal(sbOut, Ft_Single, valExpr);
        }
      }
      IRExpr* temp = runMkShadowTempValues(sbOut, 4, vals);
      addStoreTemp(sbOut, temp, Ft_Single, dest);
      if (print_temp_moves){
        addPrint3("6. Making %p in %d ", temp, mkU64(dest));
        addPrint2("for value from TS(%d)\n", mkU64(tsSrc));
      }
    } else if (valType == Ft_Double){
      IRExpr* vals[2];
      for(int i = 0; i < 2; ++i){
        Int src_addr = tsSrc + (i * sizeof(double));
        if (tsContext[src_addr] == Ft_Double){
          vals[i] = runGetTSVal(sbOut, src_addr);
        } else if (tsContext[src_addr] == Ft_Unknown){
          IRExpr* loaded = runGetTSVal(sbOut, src_addr);
          IRExpr* loadedNull = runZeroCheck64(sbOut, loaded);
          IRExpr* valExpr = runGet64C(sbOut, src_addr);
          IRExpr* freshSV = runMkShadowValG(sbOut, loadedNull,
                                            Ft_Double, valExpr);
          vals[i] = runITE(sbOut, loadedNull, freshSV, loaded);
        } else {
          IRExpr* valExpr = runGet64C(sbOut, src_addr);
          vals[i] = runMkShadowVal(sbOut, Ft_Double, valExpr);
        }
      }
      IRExpr* temp = runMkShadowTempValues(sbOut, 2, vals);
      addStoreTemp(sbOut, temp, Ft_Double, dest);
      if (print_temp_moves){
        addPrint3("7. Making %p in %d ", temp, mkU64(dest));
        addPrint2("for value from TS(%d)\n", mkU64(tsSrc));
      }
    } else if (valType == Ft_Unknown) {
      IRExpr* loaded = runPureCCall64_3(sbOut, dynamicGet128,
                                        mkU64(tsSrc),
                                        runGet64C(sbOut, tsSrc),
                                        runGet64C(sbOut, tsSrc + sizeof(double)));
      addStoreTempUnknown(sbOut, loaded, dest);
      if (print_temp_moves){
        IRExpr* loadedNonNull = runNonZeroCheck64(sbOut, loaded);
        addPrintG3(loadedNonNull,
                   "8. Making %p in %d ",
                   loaded, mkU64(dest));
        addPrintG2(loadedNonNull, "for value from TS(%d)\n", mkU64(tsSrc));
      }
    }
  }
}
void instrumentGetI(IRSB* sbOut, IRTemp dest,
                    IRExpr* varoffset, int constoffset,
                    Int arrayBase, Int numElems, IRType elemType){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
  tempContext[dest] = Ft_Unshadowed;
}
void instrumentLoad(IRSB* sbOut, IRTemp dest,
                    IRExpr* addr, IRType type){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
  tempContext[dest] = Ft_Unshadowed;
}
void instrumentLoadG(IRSB* sbOut, IRTemp dest,
                     IRExpr* altValue, IRExpr* guard,
                     IRExpr* addr, IRLoadGOp conversion){
  if (!isFloat(sbOut->tyenv, dest)){
    return;
  }
  tempContext[dest] = Ft_Unshadowed;
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
void finishInstrumentingBlock(IRSB* sbOut){
  VG_(memset)(tempContext, 0, sizeof tempContext);
  VG_(memset)(tsContext, 0, sizeof tsContext);
  if (VG_(sizeXA)(tempDebt) == 0){
    return;
  }
  IRTemp* curDebtContents =
    VG_(perm_malloc)(sizeof(IRTemp) * VG_(sizeXA)(tempDebt),
                     vg_alignof(IRTemp));
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    curDebtContents[i] = *(IRTemp*)VG_(indexXA)(tempDebt, i);
  }
  IRDirty* dynCleanupDirty =
    unsafeIRDirty_0_N(2, "dynamicCleanup",
                      VG_(fnptr_to_fnentry)(dynamicCleanup),
                      mkIRExprVec_2(mkU64(VG_(sizeXA)(tempDebt)),
                                    mkU64((uintptr_t)curDebtContents)));
  dynCleanupDirty->mFx = Ifx_Modify;
  dynCleanupDirty->mAddr = mkU64((uintptr_t)shadowTemps);
  dynCleanupDirty->mSize = sizeof(ShadowTemp) * MAX_TEMPS;
  addStmtToIRSB(sbOut, IRStmt_Dirty(dynCleanupDirty));
  VG_(deleteXA)(tempDebt);
  tempDebt = VG_(newXA)(VG_(malloc), "temp debt array", VG_(free),
                        sizeof(IRTemp));
}
void addBlockCleanupG(IRSB* sbOut, IRExpr* guard){
  IRTemp* curDebtContents =
    VG_(perm_malloc)(sizeof(IRTemp) * VG_(sizeXA)(tempDebt),
                     vg_alignof(IRTemp));
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    curDebtContents[i] = *(IRTemp*)VG_(indexXA)(tempDebt, i);
  }
  IRDirty* dynCleanupDirty =
    unsafeIRDirty_0_N(2, "dynamicCleanup",
                      VG_(fnptr_to_fnentry)(dynamicCleanup),
                      mkIRExprVec_2(mkU64(VG_(sizeXA)(tempDebt)),
                                    mkU64((uintptr_t)curDebtContents)));
  dynCleanupDirty->guard = guard;
  dynCleanupDirty->mFx = Ifx_Modify;
  dynCleanupDirty->mAddr = mkU64((uintptr_t)shadowTemps);
  dynCleanupDirty->mSize = sizeof(ShadowTemp) * MAX_TEMPS;
  addStmtToIRSB(sbOut, IRStmt_Dirty(dynCleanupDirty));
}
void cleanupAtEndOfBlock(IRSB* sbOut, IRTemp shadowed_temp){
  for(int i = 0; i < VG_(sizeXA)(tempDebt); ++i){
    IRTemp* prevEntry = VG_(indexXA)(tempDebt, i);
    if (*prevEntry == shadowed_temp){
      return;
    }
  }
  VG_(addToXA)(tempDebt, &shadowed_temp);
}
void addDynamicDisown(IRSB* sbOut, IRTemp idx){
  IRDirty* disownDirty =
    unsafeIRDirty_0_N(1, "disownShadowTempDynamic",
                      VG_(fnptr_to_fnentry)(disownShadowTempDynamic),
                      mkIRExprVec_1(mkU64(idx)));
  disownDirty->mFx = Ifx_Modify;
  disownDirty->mAddr = mkU64((uintptr_t)&(shadowTemps[idx]));
  disownDirty->mSize = sizeof(ShadowTemp*);
  addStmtToIRSB(sbOut, IRStmt_Dirty(disownDirty));
}
void addDynamicDisownNonNull(IRSB* sbOut, IRTemp idx){
  IRDirty* disownDirty =
    unsafeIRDirty_0_N(1, "disownShadowTempNonNullDynamic",
                      VG_(fnptr_to_fnentry)(disownShadowTempNonNullDynamic),
                      mkIRExprVec_1(mkU64(idx)));
  disownDirty->mFx = Ifx_Modify;
  disownDirty->mAddr = mkU64((uintptr_t)&(shadowTemps[idx]));
  disownDirty->mSize = sizeof(ShadowTemp*);
  addStmtToIRSB(sbOut, IRStmt_Dirty(disownDirty));
}
void addDynamicDisownNonNullDetached(IRSB* sbOut, IRExpr* st){
  runPureCCall(sbOut,
               mkIRCallee(1, "disownShadowTemp",
                          VG_(fnptr_to_fnentry)(disownShadowTemp)),
               Ity_I64,
               mkIRExprVec_1(st));
}
void addDisownNonNull(IRSB* sbOut, IRExpr* shadow_temp, int num_vals){
  IRExpr* valuesAddr = runArrow(sbOut, shadow_temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    IRExpr* value = runIndex(sbOut, valuesAddr, ShadowValue*, i);
    addSVDisown(sbOut, value);
  }
  addStackPush(sbOut, freedTemps[num_vals - 1], shadow_temp);
}
void addDisown(IRSB* sbOut, IRExpr* shadow_temp, int num_vals){
  IRExpr* tempNonNull = runNonZeroCheck64(sbOut, shadow_temp);
  IRExpr* valuesAddr =
    runArrowG(sbOut, tempNonNull, shadow_temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    IRExpr* value = runIndexG(sbOut, tempNonNull,
                              valuesAddr,
                              ShadowValue*, i);
    addSVDisownG(sbOut, tempNonNull, value);
  }
  addStackPushG(sbOut, tempNonNull, freedTemps[num_vals - 1],
                shadow_temp);
}
void addDisownG(IRSB* sbOut, IRExpr* guard, IRExpr* shadow_temp, int num_vals){
  IRExpr* valuesAddr =
    runArrowG(sbOut, guard, shadow_temp, ShadowTemp, values);
  for(int i = 0; i < num_vals; ++i){
    IRExpr* value = runIndexG(sbOut, guard,
                              valuesAddr,
                              ShadowValue*, i);
    addSVDisownG(sbOut, guard, value);
  }
  addStackPushG(sbOut, guard, freedTemps[num_vals - 1],
                shadow_temp);
}
void addSVOwn(IRSB* sbOut, IRExpr* sv){
  IRExpr* valueNonNull = runNonZeroCheck64(sbOut, sv);
  addSVOwnNonNullG(sbOut, valueNonNull, sv);
}
void addSVOwnNonNullG(IRSB* sbOut, IRExpr* guard, IRExpr* sv){
  IRExpr* prevRefCount =
    runArrowG(sbOut, guard, sv, ShadowValue, ref_count);
  IRExpr* newRefCount =
    runBinop(sbOut, Iop_Add64, prevRefCount, mkU64(1));
  addStoreArrowG(sbOut, guard, sv, ShadowValue, ref_count, newRefCount);
  if (print_value_moves){
    addPrintG3(guard, "[2] Owning %p, new ref_count %d\n", sv, newRefCount);
  }
}
void addSVOwnNonNull(IRSB* sbOut, IRExpr* sv){
  IRExpr* prevRefCount =
    runArrow(sbOut, sv, ShadowValue, ref_count);
  IRExpr* newRefCount =
    runBinop(sbOut, Iop_Add64, prevRefCount, mkU64(1));
  addStoreArrow(sbOut, sv, ShadowValue, ref_count, newRefCount);
}
void addSVDisown(IRSB* sbOut, IRExpr* sv){
  IRExpr* valueNonNull = runNonZeroCheck64(sbOut, sv);
  IRExpr* prevRefCount =
    runArrowG(sbOut, valueNonNull, sv, ShadowValue, ref_count);
  IRExpr* lastRef = runBinop(sbOut, Iop_CmpEQ64, prevRefCount, mkU64(1));
  // If value is null, then preRefCount will be zero, so lastRef will be false.
  addStackPushG(sbOut, lastRef, freedVals, sv);
  if (print_value_moves){
    addPrintG2(lastRef,
               "Disowned last reference to %p! Freeing...\n", sv);
  }

  IRExpr* newRefCount =
    runBinop(sbOut, Iop_Sub64, prevRefCount, mkU64(1));
  IRExpr* shouldUpdateRefCount =
    runBinop(sbOut, Iop_CmpLT64U, mkU64(1), prevRefCount);
  addStoreArrowG(sbOut, shouldUpdateRefCount, sv, ShadowValue,
                 ref_count, newRefCount);
}
void addSVDisownNonNull(IRSB* sbOut, IRExpr* sv){
  IRExpr* prevRefCount =
    runArrow(sbOut, sv, ShadowValue, ref_count);
  IRExpr* lastRef = runBinop(sbOut, Iop_CmpEQ64, prevRefCount, mkU64(1));
  addStackPushG(sbOut, lastRef, freedVals, sv);
  if (print_value_moves){
    addPrintG2(lastRef,
               "Disowned last reference to %p! Freeing...\n", sv);
  }

  IRExpr* newRefCount =
    runBinop(sbOut, Iop_Sub64, prevRefCount, mkU64(1));
  IRExpr* shouldUpdateRefCount = runUnop(sbOut, Iop_Not1, lastRef);
  if (print_value_moves){
    addPrintG3(shouldUpdateRefCount,
               "[2] Disowning %p, new ref_count %d\n", sv, newRefCount);
  }
  addStoreArrowG(sbOut, shouldUpdateRefCount, sv, ShadowValue,
                 ref_count, newRefCount);
}
void addSVDisownG(IRSB* sbOut, IRExpr* guard, IRExpr* sv){
  IRExpr* valueNonNull = runNonZeroCheck64(sbOut, sv);
  IRExpr* shouldDoAnythingAtAll = runAnd(sbOut, valueNonNull, guard);
  IRExpr* prevRefCount =
    runArrowG(sbOut, shouldDoAnythingAtAll, sv, ShadowValue, ref_count);
  IRExpr* lastRef = runBinop(sbOut, Iop_CmpEQ64, prevRefCount, mkU64(1));
  IRExpr* shouldPush = runAnd(sbOut, shouldDoAnythingAtAll, lastRef);
  if (print_value_moves){
    addPrintG2(shouldPush,
               "Disowned last reference to %p! Freeing...\n", sv);
  }
  addStackPushG(sbOut, shouldPush, freedVals, sv);

  IRExpr* newRefCount =
    runBinop(sbOut, Iop_Sub64, prevRefCount, mkU64(1));
  IRExpr* shouldUpdateRefCount =
    runAnd(sbOut, shouldDoAnythingAtAll,
           runUnop(sbOut, Iop_Not1, lastRef));
  if (print_value_moves){
    addPrintG3(shouldUpdateRefCount,
               "[3] Disowning %p, new ref_count %d\n", sv, newRefCount);
  }
  addStoreArrowG(sbOut, shouldUpdateRefCount, sv, ShadowValue,
                 ref_count, newRefCount);
}
void addClear(IRSB* sbOut, IRTemp dest, int num_vals){
  IRExpr* oldShadowTemp = runLoad64C(sbOut, &(shadowTemps[dest]));
  addPrint3("Freeing value %p in %d early\n",
            oldShadowTemp, mkU64(dest));
  addDisownNonNull(sbOut, oldShadowTemp, num_vals);
  addStoreC(sbOut, mkU64(0), &(shadowTemps[dest]));
}
IRExpr* runMkShadowTempValuesG(IRSB* sbOut, IRExpr* guard,
                               int num_values,
                               IRExpr** values){
  IRExpr* stackEmpty = runStackEmpty(sbOut, freedTemps[num_values-1]);
  IRExpr* shouldMake = runAnd(sbOut, guard, stackEmpty);
  IRExpr* freshTemp = runDirtyG_1_1(sbOut, shouldMake, newShadowTemp,
                                    mkU64(num_values));
  IRExpr* shouldPop = runAnd(sbOut, guard,
                             runUnop(sbOut, Iop_Not1, stackEmpty));
  IRExpr* poppedTemp = runStackPopG(sbOut,
                                    shouldPop,
                                    freedTemps[num_values-1]);
  IRExpr* temp = runITE(sbOut, stackEmpty, freshTemp, poppedTemp);
  IRExpr* tempValues = runArrowG(sbOut, guard, temp, ShadowTemp, values);
  for(int i = 0; i < num_values; ++i){
    addSVOwnNonNullG(sbOut, guard, values[i]);
    addStoreIndexG(sbOut, guard, tempValues, ShadowValue*, i, values[i]);
  }
  IRExpr* result = runITE(sbOut, guard, temp, mkU64(0));
  return result;
}
IRExpr* runMkShadowTempValues(IRSB* sbOut, int num_values,
                              IRExpr** values){
  IRExpr* stackEmpty = runStackEmpty(sbOut, freedTemps[num_values-1]);
  IRExpr* freshTemp = runDirtyG_1_1(sbOut, stackEmpty, newShadowTemp,
                                    mkU64(num_values));
  IRExpr* poppedTemp = runStackPopG(sbOut,
                                    runUnop(sbOut, Iop_Not1, stackEmpty),
                                    freedTemps[num_values-1]);
  IRExpr* temp = runITE(sbOut, stackEmpty, freshTemp, poppedTemp);
  IRExpr* tempValues = runArrow(sbOut, temp, ShadowTemp, values);
  for(int i = 0; i < num_values; ++i){
    addSVOwnNonNull(sbOut, values[i]);
    addStoreIndex(sbOut, tempValues, ShadowValue*, i, values[i]);
  }
  return temp;
}
IRExpr* runMkShadowVal(IRSB* sbOut, FloatType type, IRExpr* valExpr){
  return runPureCCall64_2(sbOut, mkShadowValue, mkU64(type), valExpr);
}
IRExpr* runMkShadowValG(IRSB* sbOut, IRExpr* guard,
                        FloatType type, IRExpr* valExpr){
  return runDirtyG_1_2(sbOut, guard, mkShadowValue,
                       mkU64(type), valExpr);
}
IRExpr* runMakeInput(IRSB* sbOut, IRExpr* argExpr,
                     FloatType valType, int num_vals){
  IRExpr* result;
  IRType bytesType = typeOfIRExpr(sbOut->tyenv, argExpr);
  if (num_vals == 1){
    IRExpr* argI64 = toDoubleBytes(sbOut, argExpr);;
    if (valType == Ft_Single){
      result = runPureCCall64(sbOut, mkShadowTempOneSingle, argI64);
    } else {
      result = runPureCCall64(sbOut, mkShadowTempOneDouble, argI64);
    }
  } else if (num_vals == 2 && valType == Ft_Double) {
    tl_assert(bytesType == Ity_V128);
    addStoreC(sbOut, argExpr, computedArgs.argValues[0]); 
    result = runPureCCall64(sbOut, mkShadowTempTwoDoubles,
                          mkU64((uintptr_t)computedArgs.argValues[0]));
  } else if (num_vals == 2 && valType == Ft_Single) {
    tl_assert(bytesType == Ity_I64);
    result = runPureCCall64(sbOut, mkShadowTempTwoSingles, argExpr);
  } else if (num_vals == 4) {
    tl_assert(valType == Ft_Single);
    tl_assert(bytesType == Ity_V128);
    addStoreC(sbOut, argExpr, computedArgs.argValues[0]);
    result = runPureCCall64(sbOut, mkShadowTempFourSingles,
                          mkU64((uintptr_t)computedArgs.argValues[0]));
  } else {
    tl_assert2(0, "Hey, you can't have %d vals!\n", num_vals);
  }
  if (canStoreShadow(sbOut->tyenv, argExpr)){
    addStoreTemp(sbOut, result, valType,
                 argExpr->Iex.RdTmp.tmp);
  }
  return result;
}
IRExpr* runMakeInputG(IRSB* sbOut, IRExpr* guard, 
                      IRExpr* argExpr,
                      FloatType valType, int num_vals){
  IRExpr* result;
  IRType bytesType = typeOfIRExpr(sbOut->tyenv, argExpr);
  if (num_vals == 1){
    if (valType == Ft_Single){
      tl_assert(bytesType == Ity_I32);
    } else {
      tl_assert(bytesType == Ity_I64 || bytesType == Ity_F64);
    }
    IRExpr* argI64 = toDoubleBytes(sbOut, argExpr);
    result = runDirtyG_1_1(sbOut, guard,
                           valType == Ft_Single ?
                           (void*)mkShadowTempOneSingle :
                           (void*)mkShadowTempOneDouble,
                           argI64);
  } else if (num_vals == 2 && valType == Ft_Single){
    tl_assert(bytesType == Ity_I64);
    result = runDirtyG_1_1(sbOut, guard, mkShadowTempTwoSingles, argExpr);
  } else if (num_vals == 2 && valType == Ft_Double){ 
    tl_assert(bytesType == Ity_V128);
    addStoreGC(sbOut, guard, argExpr, computedArgs.argValues[0]);
    result = runDirtyG_1_1(sbOut, guard,
                           mkShadowTempTwoDoubles,
                           mkU64((uintptr_t)computedArgs.argValues[0]));
  } else if (num_vals == 4){
    tl_assert(valType == Ft_Single);
    tl_assert(bytesType == Ity_V128);
    addStoreGC(sbOut, guard, argExpr, computedArgs.argValues[0]);
    result = runDirtyG_1_1(sbOut, guard,
                           mkShadowTempFourSingles,
                           mkU64((uintptr_t)computedArgs.argValues[0]));
  } else {
    tl_assert2(0, "Hey, you can't have %d vals!\n", num_vals);
  }
  if (canStoreShadow(sbOut->tyenv, argExpr)){
    addStoreTempG(sbOut, guard, result, valType,
                  argExpr->Iex.RdTmp.tmp);
  }
  return result;
}
IRExpr* runLoadTemp(IRSB* sbOut, int idx){
  return runLoad64C(sbOut, &(shadowTemps[idx]));
}
IRExpr* runGetTSVal(IRSB* sbOut, Int tsSrc){
  return runLoad64C(sbOut,
                    &(shadowThreadState[VG_(get_running_tid)()][tsSrc]));
}
IRExpr* runGetTSValDynamic(IRSB* sbOut, IRExpr* tsSrc){
  return runLoad64(sbOut,
                   runBinop(sbOut,
                            Iop_Add64,
                            mkU64((uintptr_t)shadowThreadState
                                  [VG_(get_running_tid)()]),
                            tsSrc));
}
void addSetTSValNonNull(IRSB* sbOut, Int tsDest,
                        IRExpr* newVal, FloatType floatType){
  tl_assert(floatType == Ft_Single ||
            floatType == Ft_Double);
  tsContext[tsDest] = floatType;
  addSVOwnNonNull(sbOut, newVal);
  addSetTSVal(sbOut, tsDest, newVal);
}
void addSetTSValNonFloat(IRSB* sbOut, Int tsDest){
  addSetTSVal(sbOut, tsDest, mkU64(0));
  tsContext[tsDest] = Ft_NonFloat;
}
void addSetTSValUnshadowed(IRSB* sbOut, Int tsDest){
  addSetTSVal(sbOut, tsDest, mkU64(0));
  tsContext[tsDest] = Ft_Unshadowed;
}
void addSetTSValUnknown(IRSB* sbOut, Int tsDest, IRExpr* newVal){
  addSetTSVal(sbOut, tsDest, newVal);
  tsContext[tsDest] = Ft_Unknown;
}
void addSetTSVal(IRSB* sbOut, Int tsDest, IRExpr* newVal){
  if (print_value_moves){
    IRExpr* existing = runGetTSVal(sbOut, tsDest);
    IRExpr* overwriting = runNonZeroCheck64(sbOut, existing);
    IRExpr* valueNonNull = runNonZeroCheck64(sbOut, newVal);
    IRExpr* shouldPrintAtAll = runOr(sbOut, overwriting, valueNonNull);
    addPrintG3(shouldPrintAtAll,
               "Setting thread state %d to %p",
               mkU64(tsDest), newVal);
    IRExpr* newValType = runArrowG(sbOut, valueNonNull,
                                   newVal, ShadowValue, type);
    addPrintG2(valueNonNull, " (type %d)\n", newValType);
    addPrintG(runAnd(sbOut, shouldPrintAtAll,
                     runUnop(sbOut, Iop_Not1, valueNonNull)),
              "\n");
  }
  addStoreC(sbOut,
            newVal,
            &(shadowThreadState[VG_(get_running_tid)()][tsDest]));
}
void addSetTSValDynamic(IRSB* sbOut, IRExpr* tsDest, IRExpr* newVal){
  addStore(sbOut, newVal,
           runBinop(sbOut,
                    Iop_Add64,
                    mkU64((uintptr_t)shadowThreadState
                          [VG_(get_running_tid)()]),
                    tsDest));
}
void addStoreTemp(IRSB* sbOut, IRExpr* shadow_temp,
                  FloatType type,
                  int idx){
  tl_assert2(tempContext[idx] == Ft_Unknown ||
             tempContext[idx] == Ft_Unshadowed,
             "Tried to set an already set temp %d!\n",
             idx);
  tempContext[idx] = type;
  addStoreC(sbOut, shadow_temp, &(shadowTemps[idx]));
  cleanupAtEndOfBlock(sbOut, idx);
}
void addStoreTempG(IRSB* sbOut, IRExpr* guard,
                   IRExpr* shadow_temp,
                   FloatType type,
                   int idx){
  tl_assert2(tempContext[idx] == Ft_Unknown ||
             tempContext[idx] == Ft_NonFloat ||
             tempContext[idx] == Ft_Unshadowed ||
             tempContext[idx] == type,
             "Tried to conditionally set a"
             " temp (%d) to type %d already set with a different"
             " type temp %d!\n", idx, type, tempContext[idx]);
  addStoreGC(sbOut, guard, shadow_temp, &(shadowTemps[idx]));
  tempContext[idx] = Ft_Unknown;
  cleanupAtEndOfBlock(sbOut, idx);
}
void addStoreTempNonFloat(IRSB* sbOut, int idx){
  tempContext[idx] = Ft_NonFloat;
}
void addStoreTempUnknown(IRSB* sbOut, IRExpr* shadow_temp_maybe, int idx){
  /* IRExpr* tempNonNull = runNonZeroCheck64(sbOut, shadow_temp_maybe); */
  addStoreTemp(sbOut/* , tempNonNull */, shadow_temp_maybe, Ft_Unknown, idx);
}
void addStoreTempUnshadowed(IRSB* sbOut, int idx){
  tempContext[idx] = Ft_Unshadowed;
}
Bool tempIsTyped(int idx){
  return tempContext[idx] == Ft_Single ||
    tempContext[idx] == Ft_Double;
}
FloatType tempType(int idx){
  tl_assert2(tempContext[idx] != Ft_NonFloat,
             "Tried to get the type of temp %d, "
             "but it hasn't been set yet this SB!\n",
             idx);
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
IRExpr* toDoubleBytes(IRSB* sbOut, IRExpr* floatExpr){
  IRExpr* result;
  IRType bytesType = typeOfIRExpr(sbOut->tyenv, floatExpr);
  switch(bytesType){
  case Ity_F32:
    result = runUnop(sbOut, Iop_ReinterpF64asI64,
                     runUnop(sbOut, Iop_F32toF64, floatExpr));
    break;
  case Ity_I32:
    result = runUnop(sbOut, Iop_ReinterpF64asI64,
                     runUnop(sbOut, Iop_F32toF64,
                             runUnop(sbOut, Iop_ReinterpI32asF32,
                                     floatExpr)));
    break;
  case Ity_F64:
    result = runUnop(sbOut, Iop_ReinterpF64asI64, floatExpr);
    break;
  case Ity_I64:
    result = floatExpr;
    break;
  default:
    tl_assert(0);
  }
  return result;
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
    ppIRType(type);
    tl_assert(0);
    return 0;
  }
}

Bool tsAddrCanStoreShadow(Int tsAddr){
  switch(tsContext[tsAddr]){
  case Ft_NonFloat:
    tl_assert2(0, "why are you even asking this?");
    return False;
  default:
    return True;
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

FloatType inferTSType64(Int tsAddr){
  tl_assert2(tsContext[tsAddr + sizeof(float)] != Ft_Double,
             "Mismatched float at TS(%d)!", tsAddr);
  tl_assert2(tsContext[tsAddr] != Ft_NonFloat ||
             tsContext[tsAddr + sizeof(float)] == Ft_NonFloat,
             "Mismatched float at TS(%d)!", tsAddr);
  tl_assert2(tsContext[tsAddr] != Ft_Double ||
             tsContext[tsAddr + sizeof(float)] == Ft_NonFloat,
             "Mismatched float at TS(%d)!", tsAddr);
  tl_assert2(tsContext[tsAddr] != Ft_Single ||
             tsContext[tsAddr + sizeof(float)] == Ft_Single ||
             tsContext[tsAddr + sizeof(float)] == Ft_NonFloat ||
             tsContext[tsAddr + sizeof(float)] == Ft_Unknown,
             "Mismatched float at TS(%d)!", tsAddr);
  if (tsContext[tsAddr] == Ft_Unknown){
    return tsContext[tsAddr + sizeof(float)];
  } else {
    return tsContext[tsAddr];
  }
}
// Produce an expression to calculate (base + ((idx + bias) % len)),
// where base, bias, and len are fixed, and idx can vary at runtime.
IRExpr* mkArrayLookupExpr(IRSB* sbOut,
                          Int base, IRExpr* idx,
                          Int bias, Int len,
                          IRType elemSize){
  // Set op the temps to hold all the different intermediary values.
  IRExpr* added = runBinop(sbOut, Iop_Add64,
                           runUnop(sbOut, Iop_32Uto64, idx),
                           mkU64(bias < 0 ? bias + len : bias));
  IRExpr* divmod = runBinop(sbOut,
                            Iop_DivModU64to32,
                            added,
                            mkU32(len));
  IRExpr* index =
    runUnop(sbOut,
            Iop_32Uto64,
            runUnop(sbOut,
                    Iop_64HIto32,
                    divmod));
  IRExpr* ex1 =
    runBinop(sbOut,
             Iop_Mul64,
             mkU64(sizeofIRType(elemSize)),
             index);
  IRExpr* lookupExpr =
    runBinop(sbOut,
             Iop_Add64,
             mkU64(base),
             ex1);
  return lookupExpr;
}
