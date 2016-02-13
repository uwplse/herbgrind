#ifndef _HG_INCLUDE
#define _HG_INCLUDE

// For handling client requests. Those are those macros that you can
// put in your code to be instrumented, which will allow you to talk
// to the tool. For instance, HERBGRIND_BEGIN() and HERBGRIND_END()
// will turn the tool on an off for certain sections.
#include "herbgrind.h"
#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

// This does any initialization that needs to be done after command
// line processing.
static void hg_post_clo_init(void);
// This is where the magic happens. This function gets called to
// instrument every superblock.
static IRSB* hg_instrument ( VgCallbackClosure* closure,
                             IRSB* bb,
                             const VexGuestLayout* layout, 
                             const VexGuestExtents* vge,
                             const VexArchInfo* archinfo_host,
                             IRType gWordTy, IRType hWordTy );
// This is called after the program exits, for cleanup and such.
static void hg_fini(Int exitcode);
// This is called after the program exits, for cleanup and such.
// This handles client requests, the macros that client programs stick
// in to send messages to the tool.
static Bool hg_handle_client_request(ThreadId tid, UWord* arg, UWord* ret);
// This is where we initialize everything
static void hg_pre_clo_init(void);

// Print an IR superblock
static void printSuperBlock(IRSB* superblock);

#endif
