/*--------------------------------------------------------------------*/
/*--- Herbgrind: a valgrind tool for Herbie               output.c ---*/
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

#include "output.h"
#include "pub_tool_vki.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_clientstate.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_hashtable.h"
#include "pub_tool_mallocfree.h"
#include "shadowop-info.h"
#include "../../options.h"

#include "../shadowop/symbolic-op.h"
#include "../../helper/runtime-util.h"

#define ENTRY_BUFFER_SIZE 2048000

void writeOutput(void){
  SysRes fileResult =
    VG_(open)(getOutputFilename(),
              VKI_O_CREAT | VKI_O_TRUNC | VKI_O_WRONLY,
              VKI_S_IRUSR | VKI_S_IWUSR);

  if (sr_isError(fileResult)){
    VG_(printf)("Couldn't open output file!\n");
    return;
  }
  Int fileD = sr_Res(fileResult);

  if (VG_(HT_count_nodes)(markMap) == 0 &&
      !haveErroneousIntMarks()){
    if (!output_sexp){
      char output[] = "No marks found!\n";
      VG_(write)(fileD, output, sizeof(output));
    }
    VG_(printf)("Didn't find any marks!\n");
    return;
  }
  VG_(HT_ResetIter)(markMap);
  for(MarkInfoArray* markInfoArray = VG_(HT_Next)(markMap);
      markInfoArray != NULL; markInfoArray = VG_(HT_Next)(markMap)){
    const char* src_filename;
    const char* fnname;
    unsigned int src_line;
    for(int argIdx = 0; argIdx < markInfoArray->nmarks; ++argIdx){
      MarkInfo* markInfo = &(markInfoArray->marks[argIdx]);
      if (markInfo->eagg.num_evals == 0){
        continue;
      }

      if (!VG_(get_filename_linenum)(VG_(current_DiEpoch)(), markInfo->addr, &src_filename,
                                     NULL, &src_line)){
        src_line = -1;
        src_filename = "Unknown";
      }
      fnname = getFnName(markInfo->addr);

      char _buf[ENTRY_BUFFER_SIZE];
      BBuf* buf = mkBBuf(ENTRY_BUFFER_SIZE, _buf);

      if (output_sexp){
        printBBuf(buf, "(output\n");
        printBBuf(buf,
                  "  (argIdx %d)\n"
                  "  (function \"%s\")\n"
                  "  (filename \"%s\")\n"
                  "  (line-num %u)\n"
                  "  (instr-addr %lX)\n",
                  argIdx,
                  fnname, src_filename, src_line,
                  markInfo->addr);
        if (print_object_files){
          const char* objfile_name;
          if (!VG_(get_objname)(VG_(current_DiEpoch)(), markInfo->addr, &objfile_name)){
            objfile_name = "Unknown Object";
          }
          printBBuf(buf,
                    "  (objectfile \"%s\")\n",
                    objfile_name);
        }
        if (output_mark_exprs && !no_exprs){
          printBBuf(buf, "  (full-expr \n");
          int numVars;
          char* exprString = symbExprToString(markInfo->expr, &numVars);
          char* varString = symbExprVarString(numVars);
          printBBuf(buf,
                    "    (FPCore %s\n"
                    "     %s))\n",
                    varString, exprString);
        }
        printBBuf(buf,
                  "  (avg-error %f)\n"
                  "  (max-error %f)\n"
                  "  (num-calls %lld)\n"
                  "  (influences\n",
                  markInfo->eagg.total_error /
                  markInfo->eagg.num_evals,
                  markInfo->eagg.max_error,
                  markInfo->eagg.num_evals);
      } else {
        if (markInfoArray->nmarks > 1){
          printBBuf(buf, "Output, float arg #%d\n", argIdx + 1);
        } else {
          printBBuf(buf, "Output");
        }
        char* addrString = getAddrString(markInfo->addr);
        printBBuf(buf, " @ %s\n", addrString);
        VG_(free)(addrString);
        if (output_mark_exprs && !no_exprs){
          printBBuf(buf, "  Full expr:\n");
          int numVars;
          char* exprString = symbExprToString(markInfo->expr, &numVars);
          char* varString = symbExprVarString(numVars);
          printBBuf(buf,
                    "    (FPCore %s\n"
                    "     %s))\n",
                    varString, exprString);
        }

        printBBuf(buf,
                  "%f bits average error\n"
                  "%f bits max error\n"
                  "Aggregated over %lld instances\n"
                  "Influenced by erroneous expression:\n",
                  markInfo->eagg.total_error /
                  markInfo->eagg.num_evals,
                  markInfo->eagg.max_error,
                  markInfo->eagg.num_evals);
      }
      unsigned int entryLen = ENTRY_BUFFER_SIZE - buf->bound;
      VG_(write)(fileD, _buf, entryLen);

      InfluenceList filteredInfluences = filterInfluenceSubexprs(markInfo->influences);
      if (only_improvable){
        filteredInfluences = filterUnimprovableInfluences(filteredInfluences);
      }
      writeInfluences(fileD, filteredInfluences);
      if (output_sexp){
        char endparens[] = "  )\n)";
        VG_(write)(fileD, endparens, sizeof(endparens) - 1);
      }
      char newline[] = "\n";
      VG_(write)(fileD, newline, 1);
    }
  }
  VG_(HT_ResetIter)(intMarkMap);
  for(IntMarkInfo* intMarkInfo = VG_(HT_Next)(intMarkMap);
      intMarkInfo != NULL; intMarkInfo = VG_(HT_Next)(intMarkMap)){
    if (intMarkInfo->num_mismatches == 0) continue;
    const char* src_filename;
    const char* fnname = getFnName(intMarkInfo->addr);
    const char* objname;
    unsigned int src_line;

    if (!VG_(get_filename_linenum)(VG_(current_DiEpoch)(), intMarkInfo->addr, &src_filename,
                                   NULL, &src_line)){
      src_line = -1;
      src_filename = "Unknown";
    }
    if (print_object_files &&
        !VG_(get_objname)(VG_(current_DiEpoch)(), intMarkInfo->addr, &objname)){
      objname = "Unknown object";
    }

    char _buf[ENTRY_BUFFER_SIZE];
    BBuf* buf = mkBBuf(ENTRY_BUFFER_SIZE, _buf);

    if (output_sexp){
      printBBuf(buf, "(%s\n", intMarkInfo->markType);
      printBBuf(buf,
                "  (function \"%s\")\n"
                "  (filename \"%s\")\n"
                "  (line-num %u)\n"
                "  (instr-addr %lX)\n",
                fnname, src_filename, src_line,
                intMarkInfo->addr);
      if (print_object_files){
        printBBuf(buf,
                  "  (objectfile \"%s\")\n",
                  objname);
      }
      if (output_mark_exprs && !no_exprs){
        printBBuf(buf, "  (full-exprs \n");
        for(int i = 0; i < intMarkInfo->nargs; ++i){
          int numVars;
          char* exprString = symbExprToString(intMarkInfo->exprs[i], &numVars);
          char* varString = symbExprVarString(numVars);
          printBBuf(buf,
                    "    (FPCore %s\n"
                    "     %s)\n",
                    varString, exprString);
        }
        printBBuf(buf, "    )\n");
      }
      printBBuf(buf,
                "  (percent-wrong %d)\n"
                "  (num-wrong %d)\n"
                "  (num-calls %d)\n"
                "  (influences\n",
                (intMarkInfo->num_mismatches * 100)
                / intMarkInfo->num_hits,
                intMarkInfo->num_mismatches,
                intMarkInfo->num_hits);
    } else {
      printBBuf(buf, "%s", intMarkInfo->markType);
      char* addrString = getAddrString(intMarkInfo->addr);
      printBBuf(buf, " @ %s\n", addrString);
      VG_(free)(addrString);
      if (output_mark_exprs && !no_exprs){
        printBBuf(buf, "Full exprs:\n");
        for(int i = 0; i < intMarkInfo->nargs; ++i){
          int numVars;
          char* exprString = symbExprToString(intMarkInfo->exprs[i],
                                              &numVars);
          char* varString = symbExprVarString(numVars);
          printBBuf(buf,
                    "    (FPCore %s\n"
                    "     %s)\n",
                    varString, exprString);
        }
      }

      printBBuf(buf,
                "%d%% incorrect\n"
                "%d incorrect values\n"
                "%d total instances\n"
                "Influenced by erroneous expressions:\n",
                (intMarkInfo->num_mismatches * 100)
                / intMarkInfo->num_hits,
                intMarkInfo->num_mismatches,
                intMarkInfo->num_hits);
    }
    unsigned int entryLen = ENTRY_BUFFER_SIZE - buf->bound;
    VG_(write)(fileD, _buf, entryLen);

    InfluenceList filteredInfluences = filterInfluenceSubexprs(intMarkInfo->influences);
    if (only_improvable){
      filteredInfluences = filterUnimprovableInfluences(filteredInfluences);
    }
    writeInfluences(fileD, filteredInfluences);
    if (output_sexp){
      char endparens[] = "  )\n"
        ")\n\n";
      VG_(write)(fileD, endparens, sizeof(endparens) - 1);
    }
  }
  VG_(close)(fileD);
}

const char* getOutputFilename(void){
  if (output_filename == NULL){
    char* default_filename = VG_(perm_malloc)(sizeof(char) * 100,
                                              vg_alignof(char));
    VG_(snprintf)(default_filename, 100, "%s.gh", VG_(args_the_exename));
    return default_filename;
  } else {
    return output_filename;
  }
}

int haveErroneousIntMarks(void){
  VG_(HT_ResetIter)(intMarkMap);
  for(IntMarkInfo* intMarkInfo = VG_(HT_Next)(intMarkMap);
      intMarkInfo != NULL; intMarkInfo = VG_(HT_Next)(intMarkMap)){
    if (intMarkInfo->num_mismatches == 0) continue;
    return True;
  }
  return False;
}

void writeInfluences(Int fileD, InfluenceList influences){
  const char* src_filename;
  const char* objname;
  unsigned int src_line;
  char _buf[ENTRY_BUFFER_SIZE];
  if (influences == NULL){
    if (!output_sexp){
      BBuf* buf = mkBBuf(ENTRY_BUFFER_SIZE, _buf);
      printBBuf(buf,
                "\n"
                "No influences found!\n"
                "\n");
      unsigned int entryLen = ENTRY_BUFFER_SIZE - buf->bound;
      VG_(write)(fileD, _buf, entryLen);
    }
  }
  if (output_sexp){
    char startparen[] = "    (\n";
    VG_(write)(fileD, startparen, sizeof(startparen) - 1);
  }
  for(int j = 0; influences != NULL && j < influences->length; ++j){
    ShadowOpInfo* opinfo = influences->data[j];

    int numVars;
    char* exprString = NULL;
    char* varString = NULL;
    RangeRecord* totalRanges = NULL;
    RangeRecord* problematicRanges = NULL;
    double* exampleProblematicArgs = NULL;
    if (!no_exprs){
      if (var_swallow){
        opinfo->expr = varSwallow(opinfo->expr);
      }
      exprString = symbExprToString(opinfo->expr, &numVars);
      getRangesAndExample(&totalRanges, &problematicRanges, &exampleProblematicArgs,
                          opinfo->expr, numVars);
      varString = symbExprVarString(numVars);
    }

    if (!VG_(get_filename_linenum)(VG_(current_DiEpoch)(), opinfo->op_addr, &src_filename,
                                   NULL, &src_line)){
      src_line = -1;
      src_filename = "Unknown";
    }
    const char* fnname = getFnName(opinfo->op_addr);
    if (!VG_(get_objname)(VG_(current_DiEpoch)(), opinfo->op_addr, &objname)){
      objname = "Unknown object";
    }

    BBuf* buf = mkBBuf(ENTRY_BUFFER_SIZE, _buf);
    if (output_sexp){
      printBBuf(buf,
                "    (");
      if (!no_exprs){
        printBBuf(buf,
                  "\n"
                  "     (expr\n"
                  "       (FPCore %s\n",
                  varString);
        if (fpcore_ranges && use_ranges){
          RangeRecord* preconditionRanges = flip_ranges ? problematicRanges : totalRanges;
          int numNonTrivialRanges = 0;
          for(int i = 0; i < numVars; ++i){
            if (nonTrivialRange(&(preconditionRanges[i]))){
              numNonTrivialRanges += 1;
            }
          }
          if (numNonTrivialRanges > 1){
            printBBuf(buf,
                      "      :pre (and");
          } else if (numNonTrivialRanges == 1) {
            printBBuf(buf,
                      "      :pre");
          }
          for(int i = 0; i < numVars; ++i){
            if (nonTrivialRange(&(preconditionRanges[i]))){
              printRangeAsPreconditionToBBuf(getVar(i), &(preconditionRanges[i]), buf);
            }
          }
          if (numNonTrivialRanges > 1){
            printBBuf(buf, ")\n");
          } else if (numNonTrivialRanges == 1) {
            printBBuf(buf, "\n");
          }
        }
        printBBuf(buf,
                  "         %s))\n",
                  exprString);
        if (use_ranges){
          if (!fpcore_ranges || !flip_ranges){
            writeProblematicRanges(buf, numVars, problematicRanges);
          }
          if (!fpcore_ranges || flip_ranges){
            writeRanges(buf, numVars, totalRanges);
          }
        }
        writeExample(buf, numVars, exampleProblematicArgs);
      }
      printBBuf(buf,
                "     (function \"%s\")\n"
                "     (filename \"%s\")\n"
                "     (line-num %u)\n"
                "     (instr-addr %lX)\n",
                fnname, src_filename, src_line,
                opinfo->op_addr);
      if (print_object_files){
        printBBuf(buf,
                  "    (objectfile \"%s\")\n",
                  objname);
      }
      ErrorAggregate local_error = opinfo->agg.local_error;
      ErrorAggregate global_error = opinfo->agg.global_error;
      printBBuf(buf,
                "     (avg-error %f)\n"
                "     (max-error %f)\n"
                "     (avg-local-error %f)\n"
                "     (max-local-error %f)\n"
                "     (num-calls %lld))\n",
                global_error.total_error
                / global_error.num_evals,
                global_error.max_error,
                local_error.total_error
                / global_error.num_evals,
                local_error.max_error,
                global_error.num_evals);
    } else {
      if (!no_exprs){
        printBBuf(buf,
                  "\n"
                  "    (FPCore %s\n",
                  varString);
        if (fpcore_ranges && use_ranges){
          RangeRecord* preconditionRanges = flip_ranges ? problematicRanges : totalRanges;
          int numNonTrivialRanges = 0;
          for(int i = 0; i < numVars; ++i){
            if (nonTrivialRange(&(preconditionRanges[i]))){
              numNonTrivialRanges += 1;
            }
          }
          if (numNonTrivialRanges > 1){
            printBBuf(buf,
                      "      :pre (and");
          } else if (numNonTrivialRanges == 1) {
            printBBuf(buf,
                      "      :pre");
          }
          for(int i = 0; i < numVars; ++i){
            if (nonTrivialRange(&(preconditionRanges[i]))){
              printRangeAsPreconditionToBBuf(getVar(i), &(preconditionRanges[i]), buf);
            }
          }
          if (numNonTrivialRanges > 1){
            printBBuf(buf, ")\n");
          } else if (numNonTrivialRanges == 1) {
            printBBuf(buf, "\n");
          }
        }
        printBBuf(buf,
                  "         %s)\n",
                  exprString);
      }
      char* addrString = getAddrString(opinfo->op_addr);
      printBBuf(buf,
                "   %s",
                addrString);
      VG_(free)(addrString);
      printBBuf(buf, "\n");
      if (numVars > 0 && use_ranges && !no_exprs){
        if (!fpcore_ranges || flip_ranges){
          writeRanges(buf, numVars, totalRanges);
        }
        if (!fpcore_ranges || !flip_ranges){
          writeProblematicRanges(buf, numVars, problematicRanges);
        }
        writeExample(buf, numVars, exampleProblematicArgs);
      }
      ErrorAggregate local_error = opinfo->agg.local_error;
      ErrorAggregate global_error = opinfo->agg.global_error;
      printBBuf(buf,
                "   %f bits average error\n"
                "   %f bits max error\n"
                "   %f bits average local error\n"
                "   %f bits max local error\n"
                "   Aggregated over %lld instances\n",
                global_error.total_error
                / global_error.num_evals,
                global_error.max_error,
                local_error.total_error
                / global_error.num_evals,
                local_error.max_error,
                global_error.num_evals);
    }
    unsigned int entryLen = ENTRY_BUFFER_SIZE - buf->bound;
    VG_(free)(exprString);
    VG_(free)(varString);
    VG_(write)(fileD, _buf, entryLen);
  }
  if (output_sexp){
    char endparen[] = "    )\n";
    VG_(write)(fileD, endparen, sizeof(endparen) - 1);
  }
}

void writeProblematicRanges(BBuf* buf, int numVars, RangeRecord* problematicRanges){
  if (output_sexp){
    printBBuf(buf, "     (var-problematic-ranges");
    for(int i = 0; i < numVars; ++i){
      if (detailed_ranges){
        printBBuf(buf,
                  "\n       (%s\n",
                  getVar(i));
        printBBuf(buf,
                  "         (neg-range-min ");
        pFloat(buf, problematicRanges[i].neg_range.min);
        printBBuf(buf,")\n");
        printBBuf(buf,
                  "         (neg-range-max ");
        pFloat(buf, problematicRanges[i].neg_range.max);
        printBBuf(buf,")\n");
        printBBuf(buf,
                  "         (pos-range-min ");
        pFloat(buf, problematicRanges[i].pos_range.min);
        printBBuf(buf,")\n");
        printBBuf(buf,
                  "         (pos-range-max ");
        pFloat(buf, problematicRanges[i].pos_range.max);
        printBBuf(buf,"))");
      } else {
        printBBuf(buf,
                  "\n       (%s\n",
                  getVar(i));
        printBBuf(buf,
                  "         (range-min ");
        pFloat(buf, problematicRanges[i].pos_range.min);
        printBBuf(buf,")\n");
        printBBuf(buf,
                  "         (range-max ");
        pFloat(buf, problematicRanges[i].pos_range.max);
        printBBuf(buf,"))");
      }
    }
    printBBuf(buf, ")\n");
  } else {
    if (detailed_ranges){
      printBBuf(buf, "   Positive Problematic Values:\n");
      for (int i = 0; i < numVars; ++i){
        tl_assert(problematicRanges != NULL);
        RangeRecord varRange = problematicRanges[i];
        printBBuf(buf, "        ");
        pFloat(buf, varRange.pos_range.min);
        printBBuf(buf,           " <= %s <= ", getVar(i));
        pFloat(buf, varRange.pos_range.max);
        printBBuf(buf, "\n");
      }
      printBBuf(buf, "   Negative Problematic Values:\n");
      for (int i = 0; i < numVars; ++i){
        RangeRecord varRange = problematicRanges[i];
        printBBuf(buf, "        ");
        pFloat(buf, varRange.neg_range.min);
        printBBuf(buf,           " <= %s <= ", getVar(i));
        pFloat(buf, varRange.neg_range.max);
        printBBuf(buf, "\n");
      }
    } else {
      printBBuf(buf, "   Problematic inputs:");
      for(int i = 0; i < numVars; ++i){
        RangeRecord varRange = problematicRanges[i];
        if (i > 0){
          printBBuf(buf, ",");
        }
        printBBuf(buf, " ");
        pFloat(buf, varRange.pos_range.min);
        printBBuf(buf, " <= %s <= ", getVar(i));
        pFloat(buf, varRange.pos_range.max);
      }
      printBBuf(buf, "\n");
    }
  }
}

void writeExample(BBuf* buf, int numVars, double* exampleProblematicInput){
  if (output_sexp){
    printBBuf(buf,
              "     (example problematic input (");
    for(int i = 0; i < numVars; ++i){
      double inputVal = exampleProblematicInput[i];
      pFloat(buf, inputVal);
      if (i != numVars - 1){
        printBBuf(buf, ", ");
      }
    }
    printBBuf(buf, "))\n");
  } else {
    printBBuf(buf, "   Example problematic input: (");
    for(int i = 0; i < numVars; ++i){
      double inputVal = exampleProblematicInput[i];
      pFloat(buf, inputVal);
      if (i != numVars - 1){
        printBBuf(buf, ", ");
      }
    }
    printBBuf(buf, ")\n");
  }
}

void writeRanges(BBuf* buf, int numVars, RangeRecord* ranges){
  if (output_sexp){
    printBBuf(buf,
              "     (var-ranges");
    for(int i = 0; i < numVars; ++i){
      if (detailed_ranges){
        printBBuf(buf,
                  "\n       (%s\n",
                  getVar(i));
        printBBuf(buf,
                  "         (neg-range-min ");
        pFloat(buf, ranges->neg_range.min);
        printBBuf(buf,")\n");
        printBBuf(buf,
                  "         (neg-range-max ");
        pFloat(buf, ranges->neg_range.max);
        printBBuf(buf,")\n");
        printBBuf(buf,
                  "         (pos-range-min ");
        pFloat(buf, ranges->pos_range.min);
        printBBuf(buf,")\n");
        printBBuf(buf,
                  "         (pos-range-max ");
        pFloat(buf, ranges->pos_range.max);
        printBBuf(buf,"))");
      } else {
        printBBuf(buf,
                  "\n       (%s\n",
                  getVar(i));
        printBBuf(buf,
                  "         (range-min ");
        pFloat(buf, ranges->pos_range.min);
        printBBuf(buf,")\n");
        printBBuf(buf,
                  "         (range-max ");
        pFloat(buf, ranges->pos_range.max);
        printBBuf(buf,"))");
      }
    }
    printBBuf(buf, ")\n");
  } else {
    if (detailed_ranges){
      printBBuf(buf, "\n        Postive Values:\n");
      for (int i = 0; i < numVars; ++i){
        RangeRecord varRange = ranges[i];
        printBBuf(buf, "        ");
        pFloat(buf, varRange.pos_range.min);
        printBBuf(buf,           " <= %s <= ", getVar(i));
        pFloat(buf, varRange.pos_range.max);
        printBBuf(buf, "\n");
      }
      printBBuf(buf, "\n        Negative Values:\n");
      for (int i = 0; i < numVars; ++i){
        RangeRecord varRange = ranges[i];
        printBBuf(buf, "        ");
        pFloat(buf, varRange.neg_range.min);
        printBBuf(buf,           " <= %s <= ", getVar(i));
        pFloat(buf, varRange.neg_range.max);
        printBBuf(buf, "\n");
      }
    } else {
      printBBuf(buf,   "      With");
      for(int i = 0; i < numVars; ++i){
        RangeRecord varRange = ranges[i];
        if (i > 0){
          printBBuf(buf, ",");
        }
        printBBuf(buf, " ");
        pFloat(buf, varRange.pos_range.min);
        printBBuf(buf, " <= %s <= ", getVar(i));
        pFloat(buf, varRange.pos_range.max);
      }
      printBBuf(buf, "\n");
    }
  }
}
