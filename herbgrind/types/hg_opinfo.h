#ifndef _HG_OPINFO
#define _HG_OPINFO

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

// When I was looking through the FpDebug source as inspiration for
// this project, I kept seeing these structures all over the place
// that looked like they only were really meant to be used and passed
// around in one place, and I kept wondering why you would use them
// when there are easier ways to pass things around, like usually as
// seperate arguments. Now that I've messed around with valgrind
// enough, I finally understand that these weird structures are used
// to get around the three argument restriction that valgrind puts on
// inserting client calls. It's possible that by the time I'm done
// here, I'll have recreated everything I didn't like about the
// FpDebug code base.
typedef struct _LoadG_Info {
  UWord cond;
  Addr src_mem;
  UWord alt_tmp;
  UWord dest_tmp;
  IRType dest_type;
} LoadG_Info;

typedef struct _OpDebug_Info {
  // The address of the operation in the binary
  Addr op_addr;
  // The source line of the operation
  UInt src_line;
  // The source file of the operation
  const HChar* src_filename;
  // The function the operation resides in.
  const HChar* fnname;
  // The plaintext name of the operation, like "subtraction"
  const HChar* plain_opname;
} OpDebug_Info;

void getOpDebug_Info(Addr op_addr, const HChar* plain_opname, OpDebug_Info* result);

typedef struct _Eval_Info {
  double max_error;
  double total_error;
} Eval_Info;

typedef enum {
  Unary,
  Binary,
  Ternary,
  Quadnary
} Arity;

typedef struct _Unary_Args {
  // This is the index of the temporary that holds these values. We'll
  // use this index to index into the shadow value array too.
  UWord arg_tmp;
  // If we don't have a shadow value yet for these arguments, we're
  // going to use the existing float value to create one. This float
  // value can be as big as 256 bits, since we account for SIMD
  // locations, so we're going to store it as an array that's
  // malloc'd when we know how big it's going to be.
  UWord* arg_value;
} Unary_Args;

typedef struct _Binary_Args {
  // These are the indicies of the temporaries that hold these
  // values. We'll use these indices to index into the shadow value
  // array too.
  UWord arg1_tmp;
  UWord arg2_tmp;
  // If we don't have a shadow value yet for these arguments, we're
  // going to use the existing float values to create one. These float
  // values can be as big as 256 bits, since we account for SIMD
  // locations, so we're going to store them as an array that's
  // malloc'd when we know how big they're going to be.
  UWord* arg1_value;
  UWord* arg2_value;
} Binary_Args;

typedef struct _Ternary_Args {
  // These are the indicies of the temporaries that hold these
  // values. We'll use these indices to index into the shadow value
  // array too.
  UWord arg1_tmp;
  UWord arg2_tmp;
  UWord arg3_tmp;
  // If we don't have a shadow value yet for these arguments, we're
  // going to use the existing float values to create one. These float
  // values can be as big as 256 bits, since we account for SIMD
  // locations, so we're going to store them as an array that's
  // malloc'd when we know how big they're going to be.
  UWord* arg1_value;
  UWord* arg2_value;
  UWord* arg3_value;
} Ternary_Args;

typedef struct _Quadnary_Args {
  // These are the indicies of the temporaries that hold these
  // values. We'll use these indices to index into the shadow value
  // array too.
  UWord arg1_tmp;
  UWord arg2_tmp;
  UWord arg3_tmp;
  UWord arg4_tmp;
  // If we don't have a shadow value yet for these arguments, we're
  // going to use the existing float values to create one. These float
  // values can be as big as 256 bits, since we account for SIMD
  // locations, so we're going to store them as an array that's
  // malloc'd when we know how big they're going to be.
  UWord* arg1_value;
  UWord* arg2_value;
  UWord* arg3_value;
  UWord* arg4_value;
} Quadnary_Args;

typedef struct _Op_Info {
  // The arity of the operation. This determines
  Arity tag;
  // The VEX op code of the operation.
  IROp op;
  // Information about where the operation resides, for reporting
  // error to the user.
  OpDebug_Info debuginfo;
  // Information about the evaluated behaviour of the operation
  Eval_Info evalinfo;
  // This is the index into where we're putting the result.
  UWord dest_tmp;
  // This is the actual computed value of the result, for checking
  // accuracy.
  UWord* dest_value;
  union {
    Unary_Args uargs;
    Binary_Args bargs;
    Ternary_Args targs;
    Quadnary_Args qargs;
  } args;
} Op_Info;

Op_Info* mkOp_Info(Arity arity, IROp op, Addr opAddr, const HChar* name);

#ifdef VG_LITTLEENDIAN
#define ENDIAN Iend_LE
#elif
#define ENDIAN Iend_BE;
#endif

#endif
