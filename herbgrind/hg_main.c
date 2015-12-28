
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie              hg_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for Herbie, which
   is mostly for experimenting with the valgrind interface with
   respect to measuring the accuracy of binary floating point
   programs.

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

#include "hg_include.h"

static mpfr_t one, a_billion, result;

// This is where the magic happens. This function gets called to
// instrument every superblock.
static
IRSB* hg_instrument ( VgCallbackClosure* closure,
                      IRSB* bb,
                      const VexGuestLayout* layout, 
                      const VexGuestExtents* vge,
                      const VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy )
{
  // For right now, just print out the VEX representation as we
  // process it.
  if (running == 0) return bb;
  VG_(printf)("Instrumenting block:\n");
  for(int i = 0; i < bb->stmts_used; i++){
    IRStmt* st = bb->stmts[i];
    ppIRStmt(st);
    VG_(printf)("\n");
  }
  VG_(printf)("\n");

  // Do a quick mpfr calculation!
  
  mpfr_init(one);
  mpfr_set_d(one, 1.0, MPFR_RNDN);
  
  mpfr_init2(a_billion, 120);
  mpfr_set_d(a_billion, 1000000000.0, MPFR_RNDN);
  
  mpfr_init2(result, 120);

  mpfr_add(result, one, a_billion, MPFR_RNDN);
  
  return bb;
}

// This handles client requests, the macros that client programs stick
// in to send messages to the tool.
static Bool hg_handle_client_request(ThreadId tid, UWord* arg, UWord* ret) {
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

// This is called after the program exits, for cleanup and such.
static void hg_fini(Int exitcode){}
// This does any initialization that needs to be done after command
// line processing.
static void hg_post_clo_init(void){}

// This disables the instrumentation of this tool.
static void stopHerbGrind(void){ running = 0; }

// This enables the instrumentation of this tool.
static void startHerbGrind(void){ running = 1; }

// This is where we initialize everything
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

   // Tell the gmp stuff to use valgrind c library instead of the
   // standard one for memory allocation and the like.
   mp_set_memory_functions(gmp_alloc, gmp_realloc, gmp_free);
   mpfr_set_strlen_function(VG_(strlen));
   mpfr_set_strcpy_function(VG_(strcpy));
   mpfr_set_memmove_function(VG_(memmove));
   mpfr_set_memcmp_function(VG_(memcmp));
   mpfr_set_memset_function(VG_(memset));
}

VG_DETERMINE_INTERFACE_VERSION(hg_pre_clo_init)

// Some memory allocation functions for gmp support
static void* gmp_alloc(size_t t){ return VG_(malloc)("hg.gmp_alloc.1", t); }
static void* gmp_realloc(void* p, size_t t1, size_t t2){ return VG_(realloc)("hg.gmp_realloc.1", p, t1); }
static void gmp_free(void* p, size_t t){ return VG_(free)(p); }

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
