#include "hg_types.h"

ShadowLocation* mkShadowLocation(LocType type){
  ShadowLocation* location = VG_(malloc)("hg.shadow_location.1", sizeof(ShadowLocation));
  size_t num_values;
  location->type = type;
  switch(type){
  case Lt_Float:
  case Lt_Double:
  case Lt_DoubleDouble:
  case Lt_DoubleDoubleDouble:
    num_values = 1;
    break;
  case Lt_Floatx2:
  case Lt_Doublex2:
    num_values = 2;
    break;
  case Lt_Floatx4:
  case Lt_Doublex4:
    num_values = 3;
    break;
  case Lt_Floatx8:
    num_values = 4;
    break;
  }
  location->values = VG_(malloc)("hg.shadow_values", sizeof(ShadowValue) * num_values);
  for(int i = 0; i < num_values; ++i){
    mpfr_init(location->values[i].value);
  }
  return location;
}
