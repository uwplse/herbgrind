#include "hg_types.h"

ShadowLocation* mkShadowLocation(LocType type){
  ShadowLocation* location = VG_(calloc)("hg.shadow_location.1", 1, sizeof(ShadowLocation));
  size_t num_values = capacity(type);
  location->type = type;
  location->values = VG_(calloc)("hg.shadow_values", num_values, sizeof(ShadowValue));
  for(int i = 0; i < num_values; ++i){
    mpfr_init(location->values[i].value);
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
    if (sl->ref_count < 0)
      VG_(printf)("double free!\n");
    VG_(printf)("Shadow location destroyed at 0x%x.\n", sl);
    for (int i = 0; i < capacity(sl->type); ++i)
      cleanupSV(&(sl->values[i]));
    VG_(printf)("Cleaned up values.\n");
    VG_(free)(sl->values);
    VG_(free)(sl);
  }
}

void copySL(ShadowLocation* src, ShadowLocation** dest){
  if (src != NULL){
    (src->ref_count) ++;
  }
  if ((*dest) != NULL){
    disownSL(*dest);
  }
  (*dest) = src;
}

ShadowValue* copySV_ptr(ShadowValue* src){
  ShadowValue* result = VG_(calloc)("hg.shadow_value.1", 1, sizeof(ShadowValue));
  mpfr_init_set(result->value, src->value, MPFR_RNDN);
  return result;
}

  VG_(printf)("clearing value at 0x%x\n", sv->value);
void copySV(ShadowValue* src, ShadowValue* dest){
  mpfr_init_set(dest->value, src->value, MPFR_RNDN);
}

void cleanupSV(ShadowValue* sv){
  mpfr_clear(sv->value);
}
