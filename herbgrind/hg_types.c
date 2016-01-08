#include "hg_types.h"

ShadowLocation* mkShadowLocation(LocType type){
  ShadowLocation* location = VG_(malloc)("hg.shadow_location.1", sizeof(ShadowLocation));
  location->type = type;
  switch(type){
  case Lt_Float:
  case Lt_Double:
    location->values = VG_(malloc)("hg.shadow_value.1", sizeof(ShadowValue));
    break;
  case Lt_Floatx2:
  case Lt_Doublex2:
    location->values = VG_(malloc)("hg.shadow_value.2", sizeof(ShadowValue) * 2);
    break;
  case Lt_Floatx4:
    location->values = VG_(malloc)("hg.shadow_value.4", sizeof(ShadowValue) * 4);
    break;
  }
  return location;
}
