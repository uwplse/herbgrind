#ifndef _HG_SHADOWVALS
#define _HG_SHADOWVALS

#include "hg_shadowvals.hh"
#include "hg_opinfo.hh"

// For mpfr_t
#include "mpfr.h"

// Some basic valgrind tool stuff
#include "pub_tool_basics.h"

// The shadow value for each logical floating point value
struct _ShadowValue {
  mpfr_t value;
};

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
struct _ShadowLocation {
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
};

struct _ShadowLocation_ptr {
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
};

// Create a new initialized shadow location of the given type.
ShadowLocation* mkShadowLocation(LocType type);
// Copy a shadow location from one area to another
void copySL(ShadowLocation* src, ShadowLocation** dest);
// Release your reference to a shadow location.
void disownSL(ShadowLocation* sl);

#endif
