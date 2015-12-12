
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie              hg_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for Herbie, which
   is mostly for experimenting with the valgrind interface with
   respect to measuring the accuracy of binary floating point
   programs.

   Copyright (C) 2002-2015 Nicholas Nethercote
      njn@valgrind.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

// For handling client requests. Those are those macros that you can
// put in your code to be instrumented, which will allow you to talk
// to the tool. For instance, HERBGRIND_BEGIN() and HERBGRIND_END()
// will turn the tool on an off for certain sections.
#include "herbgrind.h"

static int running = 0;

static void hg_post_clo_init(void)
{
}

static
IRSB* hg_instrument ( VgCallbackClosure* closure,
                      IRSB* bb,
                      const VexGuestLayout* layout, 
                      const VexGuestExtents* vge,
                      const VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy )
{
  // For right now, just print out the first print_blocks block's VEX
  // representation as we process it.
  if (running == 0) return bb;
  VG_(printf)("Instrumenting block:\n");
  for(int i = 0; i < bb->stmts_used; i++){
    IRStmt* st = bb->stmts[i];
    ppIRStmt(st);
    VG_(printf)("\n");
  }
  VG_(printf)("\n");

  print_blocks--;
  
  return bb;
}

static void hg_fini(Int exitcode)
{
}

static void stopHerbGrind(){
  running = 0;
}

static void startHerbGrind(){
  running = 1;
}

static void hg_handle_client_request(ThreadId tid, UWord* arg, UWord* ret) {
  switch(arg[0]) {
  case VG_USERREQ__BEGIN:
    startHerbGrind();
    break;
  case VG_USERREQ__END:
    stopHerbGrind();
    break;
  }
  return False;
}

static void hg_pre_clo_init(void)
{
   VG_(details_name)            ("HerbGrind");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("a valgrind tool for Herbie");
   VG_(details_copyright_author)("");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );

   VG_(basic_tool_funcs)        (hg_post_clo_init,
                                 hg_instrument,
                                 hg_fini);

   VG_(needs_client_requests) (hg_handle_client_request);

   /* No needs, no core events to track */
}

VG_DETERMINE_INTERFACE_VERSION(hg_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
