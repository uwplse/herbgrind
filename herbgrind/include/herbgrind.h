
/*--------------------------------------------------------------------*/
/*--- HerbGrind: a valgrind tool for Herbie            herbgrind.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of HerbGrind, a valgrind tool for diagnosing
   floating point accuracy problems in binary programs and extracting
   problematic expressions.

   Copyright (C) 2016 Alex Sanchez-Stern

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

#ifndef __HERBGRIND_H
#define __HERBGRIND_H

#include "../../include/valgrind.h"

// Let's define some user requests!
typedef enum {
  // This will start our tools instrumentation
  VG_USERREQ__BEGIN = VG_USERREQ_TOOL_BASE('H', 'B'),
  // And this will end it.
  VG_USERREQ__END,

  VG_USERREQ__PRINT,
  VG_USERREQ__PRINTNUM,

  VG_USERREQ__PERFORM_OP
} Vg_HerbGrindClientRequests;

#define HERBGRIND_BEGIN() \
  (__extension__({unsigned long _qzz_res; \
      VALGRIND_DO_CLIENT_REQUEST(_qzz_res, 0 /* default return (What does that mean?) */, \
                                 VG_USERREQ__BEGIN, \
                                 0, 0, 0, 0, 0); \
      _qzz_res; \
    }))

#define HERBGRIND_END() \
  (__extension__({unsigned long _qzz_res; \
      VALGRIND_DO_CLIENT_REQUEST(_qzz_res, 0, \
                                 VG_USERREQ__END, \
                                 0, 0, 0, 0, 0); \
      _qzz_res; \
    }))

#define HERBGRIND_PRINT(_qzz_mesg) \
  (__extension__({unsigned long _qzz_res; \
      VALGRIND_DO_CLIENT_REQUEST(_qzz_res, 0, \
                                 VG_USERREQ__PRINT, \
                                 _qzz_mesg, 0, 0, 0, 0); \
      _qzz_res; \
    }))

#define HERBGRIND_PRINTNUM(_qzz_num) \
  (__extension__({unsigned long _qzz_res; \
      VALGRIND_DO_CLIENT_REQUEST(_qzz_res, 0, \
                                 VG_USERREQ__PRINTNUM, \
                                 _qzz_num, 0, 0, 0, 0); \
      _qzz_res; \
    }))

#define HERBGRIND_PERFORM_OP(_qzz_op, _qzz_result_addr, _qzz_args)      \
  (__extension__({unsigned long _qzz_res; \
      VALGRIND_DO_CLIENT_REQUEST(_qzz_res, 0, \
                                 VG_USERREQ__PERFORM_OP, \
                                 _qzz_op, _qzz_result_addr, _qzz_args, 0, 0); \
      _qzz_res; \
    }))

#endif
