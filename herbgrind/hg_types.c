#include "hg_types.h"
#include "hg_macros.h"
#include "pub_tool_debuginfo.h"

ShadowLocation* mkShadowLocation(LocType type){
  ShadowLocation* location;
  ALLOC(location, "hg.shadow_location.1", 1, sizeof(ShadowLocation));
  size_t num_values = capacity(type);
  location->type = type;
  ALLOC(location->values, "hg.shadow_values", num_values, sizeof(ShadowValue));
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
  if ((*dest) != NULL){
    disownSL(*dest);
  }
  (*dest) = src;
}

ShadowValue* copySV_ptr(ShadowValue* src){
  ShadowValue* result;
  ALLOC(result, "hg.shadow_value.1", 1, sizeof(ShadowValue));
  mpfr_init_set(result->value, src->value, MPFR_RNDN);
  return result;
}

void copySV(ShadowValue* src, ShadowValue* dest){
  mpfr_init_set(dest->value, src->value, MPFR_RNDN);
}

void cleanupSV(ShadowValue* sv){
  mpfr_clear(sv->value);
}

void getOpDebug_Info(Addr op_addr, const HChar* plain_opname, OpDebug_Info* result){
  result->op_addr = op_addr;
  result->plain_opname = plain_opname;
  VG_(get_filename_linenum)(op_addr,
                            &(result->src_filename),
                            NULL,
                            &(result->src_line));
  VG_(get_fnname)(op_addr, &(result->fnname));
}
Op_Info* mkOp_Info(Arity arity, IROp op, Addr opAddr, const HChar* name){
  Op_Info* result;
  ALLOC(result, "hg.op_info.1", 1, sizeof(Op_Info));
  result->tag = arity;
  result->op = op;
  getOpDebug_Info(opAddr, name, &(result->debuginfo));
  return result;
}
