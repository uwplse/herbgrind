/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            herbgrind.h ---*/
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
#include "hg_output.h"
#include "hg_runtime.h"
#include "../types/hg_stemtea.h"
#include "../types/hg_queue.h"
#include "pub_tool_libcassert.h"
#include "../include/hg_macros.h"
#include "hg_mathreplace.h"

#include "pub_tool_vki.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_stacktrace.h"
#include "pub_tool_threadstate.h"

XArray* marks;

// How many characters are going to be allowed in each entry.
#define ENTRY_BUFFER_SIZE 2048

#define NCALLFRAMES 5

OutputMark* mkMark(Op_Info* op, Addr curAddr){
  OutputMark* mark;
  ALLOC(mark, "output mark", 1, sizeof(OutputMark));
  mark->op = op;

  mark->addr = curAddr;
  tl_assert2(VG_(get_filename_linenum)(curAddr,
                                       &(mark->src_filename),
                                       NULL,
                                       &(mark->src_line)) &&
             VG_(get_fnname)(curAddr, &(mark->fnname)),
             "Couldn't find debug info! Please compile with debug info.");

  return mark;
}

void markValueImportant(ShadowValue* shadowVal){
  if (marks == NULL){
    marks = VG_(newXA)(VG_(malloc), "op tracker",
                       VG_(free), sizeof(OutputMark*));
  }

  // Get the current client PC
  Addr curAddr = getCallAddr();
  // See if we've already hit this mark. If so, use existing mark
  // data.
  OutputMark* curMark = NULL;
  for (int i = 0; i < VG_(sizeXA)(marks); ++i){
    if ((*(OutputMark**)VG_(indexXA)(marks, i))->addr == curAddr){
      curMark = *(OutputMark**)VG_(indexXA)(marks, i);
      break;
    }
  }
  if (curMark == NULL){
    curMark = mkMark(shadowVal->stem->branch.op, curAddr);
    VG_(addToXA)(marks, &curMark);
  }

  for (int i = 0; i < VG_(sizeXA)(shadowVal->tracked_influences); ++i){
    Op_Info* new_influence =
      *(Op_Info**)VG_(indexXA)(shadowVal->tracked_influences, i);
    dedupAdd(curMark->op->influences, new_influence);
  }
}
void trackValueExpr(ShadowValue* val){
  tl_assert2(val->stem->type == Node_Branch,
             "Tried to track a leaf value!");
  dedupAdd(val->tracked_influences, val->stem->branch.op);
  if (report_all){
    markValueImportant(val);
  }
}
void propagateInfluences(ShadowValue* dest, int nargs, ...){
  va_list args;
  va_start(args, nargs);
  for(SizeT i = 0; i < nargs; ++i){
    ShadowValue* argVal = va_arg(args, ShadowValue*);
    for (SizeT j = 0; j < VG_(sizeXA)(argVal->tracked_influences); ++j){
      Op_Info* new_influence =
        *(Op_Info**)VG_(indexXA)(argVal->tracked_influences, j);
      dedupAdd(dest->tracked_influences, new_influence);
    }
  }
  va_end(args);
}

// Assumes no duplicates. Will result in NULL's in the tracked ops
// list, does not actually remove from the list, just sets matching op
// to NULL.
void clearInfluence(Op_Info* opinfo, XArray* influences){
  for(int i = 0; i < VG_(sizeXA)(influences); ++i){
    Op_Info** entry = VG_(indexXA)(influences, i);
    if (*entry == NULL) continue;
    if (*entry == opinfo){
      *entry = NULL;
      return;
    }
  }
}
typedef struct _cEntry {
  TeaNode* node;
  SizeT depth;
} cEntry;
cEntry* mkCEntry(TeaNode* node, SizeT depth);
cEntry* mkCEntry(TeaNode* node, SizeT depth){
  cEntry* entry;
  ALLOC(entry, "clear argument entry",
        1, sizeof(cEntry));
  entry->node = node;
  entry->depth = depth;
  return entry;
}
void recursivelyClearChildren(TeaNode* _node, XArray* influences){
  Queue* clearQueue = mkQueue();
  queue_push(clearQueue, mkCEntry(_node, 0));
  while (! queue_empty(clearQueue)){
    cEntry* entry = queue_pop(clearQueue);
    TeaNode* node = entry->node;
    if (node->type == Node_Branch && entry->depth < max_tea_track_depth){
      for(int i = 0; i < node->branch.nargs; ++i){
        TeaNode* child = node->branch.args[i];
        if (child != NULL && child->type == Node_Branch){
          queue_push(clearQueue, mkCEntry(child, entry->depth + 1));
          clearInfluence(child->branch.op, influences);
        }
      }
    }
    VG_(free)(entry);
  }
  freeQueue(clearQueue);
}

Word cmp_debuginfo(const Op_Info** a, const Op_Info** b){
  if (*a == NULL && *b == NULL){
    return 0;
  } else if (*a == NULL){
    return 1;
  } else if (*b == NULL){
    return -1;
  }
  double aMax = (*a)->evalinfo.max_error;
  double bMax = (*b)->evalinfo.max_error;
  if (aMax > bMax){
    return -1;
  } else if (aMax < bMax){
    return 1;
  } else {
    return 0;
  }
}

void writeReport(const char* filename){
  VG_(printf)("Writing report...\n");
  // Try to open the filename they gave us.
  SysRes file_result = VG_(open)(filename,
                                 VKI_O_CREAT | VKI_O_TRUNC | VKI_O_WRONLY,
                                 VKI_S_IRUSR | VKI_S_IWUSR);
  if(sr_isError(file_result)){
    VG_(printf)("Couldn't open output file!\n");
    return;
  }
  Int file_d = sr_Res(file_result);

  if (marks == NULL){
    VG_(write)(file_d, "No outputs found.\n", 18);
    VG_(close)(file_d);
    VG_(printf)("Wrote report out to %s\n", filename);
    return;
  }

  for(int i = 0; i < VG_(sizeXA)(marks); ++i){
    OutputMark* mark = *(OutputMark**)VG_(indexXA)(marks, i);

    HChar buf[ENTRY_BUFFER_SIZE];
    UInt entry_len;

    if (!report_all){
      if (human_readable){
        entry_len = VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                                  "Result in %s at %s:%u (address %lX)\n"
                                  "%f bits average error\n"
                                  "%f bits max error\n"
                                  "Aggregated over %lu instances\n"
                                  "Influenced by erroneous expressions:\n\n",
                                  mark->fnname,
                                  mark->src_filename,
                                  mark->src_line,
                                  mark->addr,
                                  mark->op->evalinfo.total_error /
                                  mark->op->evalinfo.num_calls,
                                  mark->op->evalinfo.max_error,
                                  mark->op->evalinfo.num_calls);
      } else {
        entry_len = VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                                  "(output\n"
                                  "  (function \"%s\")\n"
                                  "  (filename \"%s\")\n"
                                  "  (line-num %u)\n"
                                  "  (instr-addr %lX)\n"
                                  "  (avg-error %f)\n"
                                  "  (max-error %f)\n"
                                  "  (num-calls %lu)\n"
                                  "  (influences\n",
                                  mark->fnname,
                                  mark->src_filename,
                                  mark->src_line,
                                  mark->addr,
                                  mark->op->evalinfo.total_error /
                                  mark->op->evalinfo.num_calls,
                                  mark->op->evalinfo.max_error,
                                  mark->op->evalinfo.num_calls);
      }
      VG_(write)(file_d, buf, entry_len);
    }

    XArray* influences = mark->op->influences;
    if (report_exprs){
      for (int j = VG_(sizeXA)(influences) - 1; j >= 0; --j){
        Op_Info* influence = *(Op_Info**)VG_(indexXA)(influences, j);
        if (influence == NULL) continue;
        recursivelyClearChildren(influence->tea, influences);
      }
    }
    VG_(setCmpFnXA)(influences, (XACmpFn_t)cmp_debuginfo);
    // Sort the entries by maximum error.
    VG_(sortXA)(influences);

    for (int j = 0; j < VG_(sizeXA)(influences); ++j){
      Op_Info* influence = *(Op_Info**)VG_(indexXA)(influences, j);
      if (influence == NULL) continue;

      if (report_exprs){
        char* benchString = teaToBenchString(influence->tea, True);
        if (human_readable){
          entry_len =
            VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                          "    %s\n"
                          "    in %s at %s:%u (address %lX)\n"
                          "    %f bits average error\n"
                          "    %f bits max error\n"
                          "    %f bits max local error\n"
                          "    Aggregated over %lu instances\n\n",
                          benchString,
                          influence->debuginfo.fnname,
                          influence->debuginfo.src_filename,
                          influence->debuginfo.src_line,
                          influence->debuginfo.op_addr,
                          influence->evalinfo.total_error /
                          influence->evalinfo.num_calls,
                          influence->evalinfo.max_error,
                          influence->evalinfo.max_local,
                          influence->evalinfo.num_calls);
        } else {
          entry_len =
            VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                          "    (%s\n"
                          "     (function \"%s\")\n"
                          "     (filename \"%s\")\n"
                          "     (line-num %u)\n"
                          "     (instr-addr %lX)\n"
                          "     (avg-error %f)\n"
                          "     (max-error %f)\n"
                          "     (max-local %f)\n"
                          "     (num-calls %lu))\n",
                          benchString,
                          influence->debuginfo.fnname,
                          influence->debuginfo.src_filename,
                          influence->debuginfo.src_line,
                          influence->debuginfo.op_addr,
                          influence->evalinfo.total_error /
                          influence->evalinfo.num_calls,
                          influence->evalinfo.max_error,
                          influence->evalinfo.max_local,
                          influence->evalinfo.num_calls);
        }
      } else {
        if (human_readable){
          entry_len =
            VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                          "    %s in %s at %s:%u (address %lX)\n"
                          "    %f bits average error\n"
                          "    %f bits max error\n"
                          "    Aggregated over %lu instances\n\n",
                          influence->debuginfo.plain_opname,
                          influence->debuginfo.fnname,
                          influence->debuginfo.src_filename,
                          influence->debuginfo.src_line,
                          influence->debuginfo.op_addr,
                          influence->evalinfo.total_error /
                          influence->evalinfo.num_calls,
                          influence->evalinfo.max_error,
                          influence->evalinfo.num_calls);
        } else {
          entry_len =
            VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                          "    ((plain-name \"%s\")\n"
                          "     (function \"%s\")\n"
                          "     (filename \"%s\")\n"
                          "     (line-num %u)\n"
                          "     (instr-addr %lX)\n"
                          "     (avg-error %f)\n"
                          "     (max-error %f)\n"
                          "     (num-calls %lu))\n",
                          influence->debuginfo.plain_opname,
                          influence->debuginfo.fnname,
                          influence->debuginfo.src_filename,
                          influence->debuginfo.src_line,
                          influence->debuginfo.op_addr,
                          influence->evalinfo.total_error /
                          influence->evalinfo.num_calls,
                          influence->evalinfo.max_error,
                          influence->evalinfo.num_calls);
        }
      }
      VG_(write)(file_d, buf, entry_len);
    }
    if (!human_readable){
      VG_(write)(file_d, "  ))\n", 5);
    }
  }
  VG_(close)(file_d);
  VG_(printf)("Wrote report out to %s\n", filename);
}

void dedupAdd(XArray* array, void* item){
  Bool already_have = False;
  for (SizeT k = 0; k < VG_(sizeXA)(array); ++k){
    if (*(void**)VG_(indexXA)(array, k) == item){
      already_have = True;
      break;
    }
  }
  if (!already_have){
    VG_(addToXA)(array, &item);
  }
}
