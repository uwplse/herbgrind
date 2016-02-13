#include "hg_op_tracker.h"

#include "pub_tool_vki.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcprint.h"
#include "../include/hg_options.h"

Op_Info** tracked_ops;
SizeT num_tracked_ops;
SizeT array_size;

// How many characters are going to be allowed in each entry.
#define ENTRY_BUFFER_SIZE 300

void startTrackingOp(Op_Info* opinfo){
  // If our array is already full...
  if (num_tracked_ops >= array_size){
    // Double its size.
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

  // Write out an entry for each tracked op.
  for(int i = 0; i < num_tracked_ops; ++i){
    Op_Info* opinfo = tracked_ops[i];
    UInt entry_len;
    if (human_readable){
      entry_len = VG_(snprintf)(buf, ENTRY_BUFFER_SIZE,
                                "%s in %s at %s:%u (address %lX)\n%f bits average error\n%f bits max error\nAggregated over %lu instances\n\n",
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
                                "((plain-name \"%s\") (function \"%s\") (filename \"%s\") (line-num %u) (instr-addr %lX) (avg-error %f) (max-error %f) (num-calls %lu))\n",
                                opinfo->debuginfo.plain_opname,
                                opinfo->debuginfo.fnname,
                                opinfo->debuginfo.src_filename,
                                opinfo->debuginfo.src_line,
                                opinfo->debuginfo.op_addr,
                                opinfo->evalinfo.total_error / opinfo->evalinfo.num_calls,
                                opinfo->evalinfo.max_error,
                                opinfo->evalinfo.num_calls);
    }
    VG_(write)(file_d, buf, entry_len);
  }

  // Finally, cose up the file.
  VG_(close)(file_d);
}
