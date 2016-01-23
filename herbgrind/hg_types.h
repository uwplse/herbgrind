#ifndef _HG_TYPES
#define _HG_TYPES

// For mpfr_t
#include "mpfr.h"

// Some basic valgrind tool stuff
#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_mallocfree.h"

// Pull in this header file so that we can call the valgrind version
// of printf and dmsg.
#include "pub_tool_libcprint.h"

// The shadow value for each logical floating point value
typedef struct _ShadowValue {
  mpfr_t value;
} ShadowValue;

// Approximately what you expect. copySV will do a deep copy for you,
// freeSV will free up the structure.
void cleanupSV(ShadowValue* val);
ShadowValue* copySV_ptr(ShadowValue* src);
void copySV(ShadowValue* src, ShadowValue* dest);

// The type of a floating point location. Many locations contain just
// a single float, but SIMD locations can contain multiple floats or
// doubles in a single location.
typedef enum {
  Lt_Float,
  Lt_Floatx2,
  Lt_Floatx4,
  Lt_Floatx8,
  Lt_Double,
  Lt_Doublex2,
  Lt_Doublex4,
  Lt_DoubleDouble,
  Lt_DoubleDoubleDouble,
} LocType;

// How many values the loc holds
size_t capacity(LocType bytestype);

// The value we're tracking for each floating point storage location
// in the program.
typedef struct _ShadowLocation {
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
  // Reference counter. This lets us free up shadow locations that
  // are no longer accessible.
  size_t ref_count;
} ShadowLocation;

typedef struct _ShadowLocation_ptr {
  // This member is here to make this structure compatible with the
  // hash table implementation in pub_tool_hashtable. None of our code
  // will actually use it.
  struct _ShadowLocation_ptr* next;
  // This part is also here for the hash table structure, but we'll
  // actually be messing with it as we'll set it to the address of any
  // memory location we want to store a shadow value for.
  UWord addr;
  // The actual shadow value we're pointing to.
  ShadowLocation* sl;
} ShadowLocation_ptr;

// Create a new initialized shadow location of the given type.
ShadowLocation* mkShadowLocation(LocType type);
// Copy a shadow location from one area to another
void copySL(ShadowLocation* src, ShadowLocation** dest);
// Release your reference to a shadow location.
void disownSL(ShadowLocation* sl);

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

typedef struct _UnaryOp_Info {
  UWord op;
  // This is the index of the temporary that holds these values. We'll
  // use this index to index into the shadow value array too.
  UWord arg_tmp;
  // If we don't have a shadow value yet for these arguments, we're
  // going to use the existing float value to create one. This float
  // value can be as big as 256 bits, since we account for SIMD
  // locations, so we're going to store it as an array that's
  // malloc'd when we know how big it's going to be.
  UWord* arg_value;
  // This is the index into where we're putting the result.
  UWord dest_tmp;
  // This is the actual computed value of the result, for checking
  // accuracy.
  UWord* dest_value;
} UnaryOp_Info;

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

typedef struct _TernaryOp_Info {
  UWord op;
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
  // This is the index into where we're putting the result.
  UWord dest_tmp;
  // This is the actual computed value of the result, for checking
  // accuracy.
  UWord* dest_value;
} TernaryOp_Info;

typedef struct _QuadnaryOp_Info {
  UWord op;
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
  // This is the index into where we're putting the result.
  UWord dest_tmp;
  // This is the actual computed value of the result, for checking
  // accuracy.
  UWord* dest_value;
} QuadnaryOp_Info;

#ifdef VG_LITTLEENDIAN
#define ENDIAN Iend_LE
#elif
#define ENDIAN Iend_BE;
#endif

#endif
