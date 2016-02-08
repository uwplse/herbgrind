#include "hg_op_tracker.h"

Op_Info** tracked_ops;
SizeT num_tracked_ops;
SizeT array_size;

void startTrackingOp(Op_Info* opinfo){
  if (num_tracked_ops + 1 >= array_size){
    tracked_ops = VG_(realloc)("hg.expand_tracked_op_array.1", tracked_ops, array_size * 2);
    array_size = array_size * 2;
  }
  tracked_ops[num_tracked_ops] = opinfo;
  num_tracked_ops++;
}
