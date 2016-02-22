#include "hg_op_tracker.h"

#include "../include/hg_options.h"
#include "../types/hg_opinfo.h"
#include "../types/hg_ast.h"

#include "pub_tool_vki.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcprint.h"

Op_Info** tracked_ops;
SizeT num_tracked_ops;
SizeT array_size;

// How many characters are going to be allowed in each entry.
#define ENTRY_BUFFER_SIZE 512
#define START_ARRAY_SIZE 10

void startTrackingOp(Op_Info* opinfo){
  // If our array is already full...
  if (num_tracked_ops >= array_size){
    if (array_size == 0){
      tracked_ops = VG_(malloc)("hg.init_tracked_op_array.1", START_ARRAY_SIZE);
      array_size = START_ARRAY_SIZE;
    }
    tracked_ops = VG_(realloc)("hg.expand_tracked_op_array.1", tracked_ops, array_size * 2);
    array_size = array_size * 2;
  }
  // Put the op into the next slot in the array
  tracked_ops[num_tracked_ops] = opinfo;
  // Update the counter for the next available slot.
  num_tracked_ops++;
}

void writeReport(const HChar* filename){
  HChar buf[ENTRY_BUFFER_SIZE];
  // Try to open the filename they gave us.
  SysRes file_result = VG_(open)(filename,
                                 VKI_O_CREAT | VKI_O_TRUNC | VKI_O_WRONLY,
                                 VKI_S_IRUSR | VKI_S_IWUSR);
  if(sr_isError(file_result)){
    VG_(printf)("Couldn't open output file!\n");
    return;
  }
  Int file_d = sr_Res(file_result);

  if (report_exprs)
    // For each expression, counting from the back where the bigger
    // expressions should be, eliminate subexpressions from the list
    // for reporting.
    for(int i = num_tracked_ops - 1; i >= 0; --i){
      Op_Info* opinfo = tracked_ops[i];
      if (opinfo == NULL) continue;
      if (opinfo->ast->tag == Node_Branch){
        for(int j = 0; j < opinfo->ast->nd.Branch.nargs; ++j){
          OpASTNode* subexpr = opinfo->ast->nd.Branch.args[j];
          for (int k = i - 1; k >= 0; --k){
            if (tracked_ops[k] == NULL) continue;
            if (tracked_ops[k]->ast == subexpr)
              tracked_ops[k] = NULL;
          }
        }
      }
    }

  // Write out an entry for each tracked op.
  for(int i = 0; i < num_tracked_ops; ++i){
    Op_Info* opinfo = tracked_ops[i];

    // This will happen if we had an item in the list we decided to
    // eliminate.
    if (opinfo == NULL) continue;
    UInt entry_len;
    char* astString = opASTtoString(opinfo->ast);
    if (human_readable){
      entry_len = VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                                "%s\n"
                                "%s in %s at %s:%u (address %lX)\n"
                                "%f bits average error\n"
                                "%f bits max error\n"
                                "Aggregated over %lu instances\n\n",
                                astString,
                                opinfo->debuginfo.plain_opname,
                                opinfo->debuginfo.fnname,
                                opinfo->debuginfo.src_filename,
                                opinfo->debuginfo.src_line,
                                opinfo->debuginfo.op_addr,
                                opinfo->evalinfo.total_error / opinfo->evalinfo.num_calls,
                                opinfo->evalinfo.max_error,
                                opinfo->evalinfo.num_calls);
    } else {
      entry_len = VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                                "((expr %s) "
                                 "(plain-name \"%s\") "
                                 "(function \"%s\") "
                                 "(filename \"%s\") "
                                 "(line-num %u) "
                                 "(instr-addr %lX) "
                                 "(avg-error %f) "
                                 "(max-error %f) "
                                 "(num-calls %lu))\n",
                                astString,
                                opinfo->debuginfo.plain_opname,
                                opinfo->debuginfo.fnname,
                                opinfo->debuginfo.src_filename,
                                opinfo->debuginfo.src_line,
                                opinfo->debuginfo.op_addr,
                                opinfo->evalinfo.total_error / opinfo->evalinfo.num_calls,
                                opinfo->evalinfo.max_error,
                                opinfo->evalinfo.num_calls);
    }
    VG_(free)(astString);
    VG_(write)(file_d, buf, entry_len);
  }

  // Finally, close up the file.
  VG_(close)(file_d);
}
