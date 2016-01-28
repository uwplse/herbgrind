#ifndef _HG_STORAGE_RUNTIME
#define _HG_STORAGE_RUNTIME

#include "../hg_types.h"
// The maximum number of pre-instrumentation temporary values per
// block we're willing to deal with.
#define	MAX_TEMPS 1000
// The maximum number of per-thread registers we'll account for.
#define	MAX_REGISTERS 1000
#define MAX_THREADS 16

// The functions that we'll insert into the program to move around
// shadow values at run time.
VG_REGPARM(2) void copyShadowTmptoTmp(UWord src_tmp, UWord dest_tmp);
VG_REGPARM(2) void copyShadowTmptoTS(UWord src_tmp, UWord dest_reg);
VG_REGPARM(2) void copyShadowTStoTmp(UWord src_reg, IRType type, UWord dest_tmp);
VG_REGPARM(3) void copyShadowMemtoTmp(Addr src_mem, IRType type, UWord dest_tmp);
VG_REGPARM(1) void copyShadowMemtoTmpIf(LoadG_Info* info);
VG_REGPARM(2) void copyShadowTmptoMem(UWord src_tmp, Addr dest_mem);
VG_REGPARM(3) void copyShadowTmptoMemG(UWord cond, UWord src_tmp, Addr dest_mem);
// Disown all shadow values held in temporaries and set the
// temporaries array to nulls.
VG_REGPARM(0) void initBlock(void);
VG_REGPARM(0) void cleanupBlock(void);
void initStorage(void);
void cleanupStorage(void);

// Getters and setters for temporaries, memory, and thread
// state. Handles reference counting properly.
void setTemp(Addr index, ShadowLocation* newLocation);
ShadowLocation* getTemp(Addr index);

void setMem(Addr index, ShadowLocation* newLocation);
ShadowLocation* getMem(Addr index);

void setTS(Addr index, ShadowLocation* newLocation);
ShadowLocation* getTS(Addr index);

// A helper function that does most of the work for copyShadowMemtoTmp
// and copyShadowTStoTmp.
void copyShadow___toTmp(UWord src_idx, IRType dest_type, UWord dest_tmp, ShadowLocation* (*get)(Addr idx));

#endif
