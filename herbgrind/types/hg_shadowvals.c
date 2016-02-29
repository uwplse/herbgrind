#include "hg_shadowvals.h"
#include "hg_ast.h"
#include "../include/hg_macros.h"
#include "../include/hg_options.h"

// Some basic valgrind stuff
#include "pub_tool_tooliface.h"
#include "pub_tool_mallocfree.h"

// Pull in this header file so that we can call the valgrind version
// of printf and dmsg.
#include "pub_tool_libcprint.h"

ShadowLocation* mkShadowLocation(LocType type){
  ShadowLocation* location;
  ALLOC(location, "hg.shadow_location.1", 1, sizeof(ShadowLocation));
  size_t num_values = capacity(type);
  location->type = type;
  ALLOC(location->values, "hg.shadow_values", num_values, sizeof(ShadowValue));
  for(int i = 0; i < num_values; ++i){
    mpfr_init2(location->values[i].value, precision);
    ALLOC(location->values[i].ast, "hg.shadow_ast", 1, sizeof(ValueASTNode));
  }
  location->ref_count = 1;
  return location;
}

size_t capacity(LocType bytestype){
  switch(bytestype){
  case Lt_Float:
  case Lt_Double:
  case Lt_DoubleDouble:
  case Lt_DoubleDoubleDouble:
    return 1;
  case Lt_Floatx2:
  case Lt_Doublex2:
    return 2;
  case Lt_Floatx4:
  case Lt_Doublex4:
    return 4;
  case Lt_Floatx8:
    return 8;
  }
  return 0;
}

void disownSL(ShadowLocation* sl){
  (sl->ref_count) --;
  if (sl->ref_count < 1){
    for (int i = 0; i < capacity(sl->type); ++i)
      cleanupSV(&(sl->values[i]));
    VG_(free)(sl->values);
    VG_(free)(sl);
  }
}

void copySL(ShadowLocation* src, ShadowLocation** dest){
  if (src != NULL){
    (src->ref_count) ++;
  }
  else if ((*dest) != NULL){
    disownSL(*dest);
  }
  (*dest) = src;
}

ShadowValue* copySV_ptr(ShadowValue* src){
  ShadowValue* result;
  ALLOC(result, "hg.shadow_value.1", 1, sizeof(ShadowValue));
  mpfr_init2(result->value, precision);
  mpfr_set(result->value, src->value, MPFR_RNDN);
  copyValueAST(src, result);
  return result;
}

void copySV(ShadowValue* src, ShadowValue* dest){
  mpfr_init2(dest->value, precision);
  mpfr_set(dest->value, src->value, MPFR_RNDN);
  copyValueAST(src, dest);
}

void cleanupSV(ShadowValue* sv){
  mpfr_clear(sv->value);
  cleanupValueAST(sv);
}

