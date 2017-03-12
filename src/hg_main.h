
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie           hg_include.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016-2017 Alex Sanchez-Stern

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of the
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

#ifndef _HG_MAIN_H
#define _HG_MAIN_H

// For handling client requests. Those are those macros that you can
// put in your code to be instrumented, which will allow you to talk
// to the tool. For instance, HERBGRIND_BEGIN() and HERBGRIND_END()
// will turn the tool on an off for certain sections.
#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

// This does any initialization that needs to be done after command
// line processing.
static void hg_post_clo_init(void);
// This is called after the program exits, for cleanup and such.
static void hg_fini(Int exitcode);
// This is called after the program exits, for cleanup and such.
// This handles client requests, the macros that client programs stick
// in to send messages to the tool.
static Bool hg_handle_client_request(ThreadId tid, UWord* arg, UWord* ret);
// This is where we initialize everything
static void hg_pre_clo_init(void);

#endif
