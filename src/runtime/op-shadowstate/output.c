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
#include "../../helper/bbuf.h"

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
    char output[] = "No marks found!\n";
    VG_(write)(fileD, output, sizeof(output));
    return;
  }
  VG_(HT_ResetIter)(markMap);
  for(MarkInfo* markInfo = VG_(HT_Next)(markMap);
      markInfo != NULL; markInfo = VG_(HT_Next)(markMap)){
    const char* src_filename;
    const char* fnname;
    unsigned int src_line;

    if (!VG_(get_filename_linenum)(markInfo->addr, &src_filename,
                                   NULL, &src_line)){
      src_line = -1;
      src_filename = "Unknown";
      fnname = "Unknown";
    } else {
      VG_(get_fnname)(markInfo->addr, &fnname);
    }

    char _buf[ENTRY_BUFFER_SIZE];
    BBuf* buf = mkBBuf(ENTRY_BUFFER_SIZE, _buf);

    if (output_sexp){
      printBBuf(buf, "(output\n");
      printBBuf(buf,
                "  (function \"%s\")\n"
                "  (filename \"%s\")\n"
                "  (line-num %u)\n"
                "  (instr-addr %lX)\n",
                fnname, src_filename, src_line,
                markInfo->addr);
      if (print_object_files){
        const char* objfile_name;
        if (!VG_(get_objname)(markInfo->addr, &objfile_name)){
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
      printBBuf(buf, "Result");
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

    writeInfluences(fileD, filterInfluenceSubexprs(markInfo->influences));
    if (output_sexp){
      char endparens[] = "  )\n";
      VG_(write)(fileD, endparens, sizeof(endparens) - 1);
    }
    char newline[] = "\n";
    VG_(write)(fileD, newline, 1);
  }
  VG_(HT_ResetIter)(intMarkMap);
  for(IntMarkInfo* intMarkInfo = VG_(HT_Next)(intMarkMap);
      intMarkInfo != NULL; intMarkInfo = VG_(HT_Next)(intMarkMap)){
    if (intMarkInfo->num_mismatches == 0) continue;
    const char* src_filename;
    const char* fnname;
    const char* objname;
    unsigned int src_line;

    if (!VG_(get_filename_linenum)(intMarkInfo->addr, &src_filename,
                                   NULL, &src_line)){
      src_line = -1;
      src_filename = "Unknown";
      fnname = "Unknown";
    } else {
      VG_(get_fnname)(intMarkInfo->addr, &fnname);
    }
    if (print_object_files &&
        !VG_(get_objname)(intMarkInfo->addr, &objname)){
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
          char* exprString = symbExprToString(intMarkInfo->exprs[i],
                                              &numVars);
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

    writeInfluences(fileD, filterInfluenceSubexprs(intMarkInfo->influences));
    if (output_sexp){
      char endparens[] = "  )\n";
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
  const char* fnname;
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
  for(InfluenceList curNode = influences;
      curNode != NULL; curNode = curNode->next){
    ShadowOpInfo* opinfo = curNode->item;

    int numVars;
    char* exprString = symbExprToString(opinfo->expr, &numVars);
    char* varString = symbExprVarString(numVars);

    if (!VG_(get_filename_linenum)(opinfo->op_addr, &src_filename,
                                   NULL, &src_line)){
      src_line = -1;
      src_filename = "Unknown";
      fnname = "Unknown";
    } else {
      VG_(get_fnname)(opinfo->op_addr, &fnname);
    }
    if (!VG_(get_objname)(opinfo->op_addr, &objname)){
      objname = "Unknown object";
    }

    BBuf* buf = mkBBuf(ENTRY_BUFFER_SIZE, _buf);
    if (output_sexp){
      printBBuf(buf,
                "    (");
      printBBuf(buf,
                "\n"
                "     (expr\n"
                "       (FPCore %s\n"
                "         %s))\n",
                varString, exprString);
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
      printBBuf(buf,
                "     (avg-error %f)\n"
                "     (max-error %f)\n"
                "     (avg-local-error %f)\n"
                "     (max-local-error %f)\n"
                "     (num-calls %lld))\n",
                opinfo->eagg.total_error
                / opinfo->eagg.num_evals,
                opinfo->eagg.max_error,
                opinfo->local_eagg.total_error
                / opinfo->eagg.num_evals,
                opinfo->local_eagg.max_error,
                opinfo->eagg.num_evals);
      printBBuf(buf, "    )\n");
    } else {
      printBBuf(buf,
                "\n"
                "    (FPCore %s\n"
                "     %s)\n",
                varString, exprString);
      char* addrString = getAddrString(opinfo->op_addr);
      printBBuf(buf,
                "   %s",
                addrString);
      VG_(free)(addrString);
      printBBuf(buf, "\n");
      printBBuf(buf,
                "   %f bits average error\n"
                "   %f bits max error\n"
                "   %f bits average local error\n"
                "   %f bits max local error\n"
                "   Aggregated over %lld instances\n",
                opinfo->eagg.total_error
                / opinfo->eagg.num_evals,
                opinfo->eagg.max_error,
                opinfo->local_eagg.total_error
                / opinfo->eagg.num_evals,
                opinfo->local_eagg.max_error,
                opinfo->eagg.num_evals);
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
