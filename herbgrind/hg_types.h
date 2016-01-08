#ifndef _HG_TYPES
#define _HG_TYPES

// For mpfr_t
#include "mpfr.h"
#include "pub_tool_tooliface.h"

// The shadow value for each logical floating point value
typedef struct _ShadowValue {
  mpfr_t value;
} ShadowValue;

// The type of a floating point location. Many locations contain just
// a single float, but SIMD locations can contain multiple floats or
// doubles in a single location.
typedef enum {
  Lt_Float,
  Lt_Floatx2,
  Lt_Floatx4,
  Lt_Double,
  Lt_Doublex2
} LocType;

// The value we're tracking for each floating point storage location
// in the program.
typedef struct _ShadowLocation {
  // This member is here to make this structure compatible with the
  // hash table implementation in pub_tool_hashtable. None of our code
  // will actually use it.
  struct _ShadowLocation* next;
  // This part is also here for the hash table structure, but we'll
  // actually be messing with it as we'll set it to the address of any
  // memory location we want to store a shadow value for.
  UWord addr;
  // The actual high precision values shadowing a float. In most cases
  // this should be a pointer to a single value, but in cases where we
  // move, for instance, two 64-bit floats into a 128 bit location, we
  // might need to store multiple shadow values in a single
  // location. The lower indices refer to lower order bits.
  ShadowValue* values;
  // What the type of this potentially SIMD location is. If it is a
  // normal location that stores a single value, it'll be either
  // Lt_Float or Lt_Double.
  LocType type;
} ShadowLocation;

ShadowLocation* mkShadowLocation(LocType type);

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
} LoadG_Info;

typedef struct _BinaryOp_Info {
  UWord op;
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
  // This is the index into where we're putting the result.
  UWord dest_tmp;
  // This is the actual computed value of the result, for checking
  // accuracy.
  UWord* dest_value;
} BinaryOp_Info;

#endif
