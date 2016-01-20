#include "hg_types.h"

ShadowLocation* mkShadowLocation(LocType type){
  ShadowLocation* location = VG_(malloc)("hg.shadow_location.1", sizeof(ShadowLocation));
  VG_(printf)("Shadow location created at 0x%x.\n", location);
  size_t num_values = capacity(type);
  location->type = type;
  location->values = VG_(malloc)("hg.shadow_values", sizeof(ShadowValue) * num_values);
  for(int i = 0; i < num_values; ++i){
    VG_(printf)("initializing value at 0x%x\n", &location->values[i].value);
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
  sl->ref_count --;
  if (sl->ref_count < 1){
    if (sl->ref_count < 0)
      VG_(printf)("double free!\n");
    for (int i = 0; i < capacity(sl->type); ++i)
      freeSV(&sl->values[i]);
    VG_(printf)("Shadow location destroyed at 0x%x.\n", sl);
    VG_(free)(sl);
  }
}

void copySL(ShadowLocation* src, ShadowLocation** dest){
  if (src != NULL)
    src->ref_count ++;
  if ((*dest) != NULL){
    disownSL(*dest);
  }
  (*dest) = src;
}

ShadowValue* copySV(ShadowValue* src){
  ShadowValue* result = VG_(malloc)("hg.shadow_value.1", sizeof(ShadowValue));
  VG_(printf)("initializing value at 0x%x\n", result->value);
  mpfr_init_set(result->value, src->value, MPFR_RNDN);
  return result;
}

void freeSV(ShadowValue* sv){
  VG_(printf)("clearing value at 0x%x\n", sv->value);
  mpfr_clear(sv->value);
  VG_(free)(sv);
}
