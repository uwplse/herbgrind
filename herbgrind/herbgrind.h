#ifndef __HERBGRIND_H
#define __HERBGRIND_H

#include "../include/valgrind.h"

// Let's define some user requests!
typedef enum {
  // This will start our tools instrumentation
  VG_USERREQ__BEGIN,
  // And this will end it.
  VG_USERREQ__END,

  VG_USERREQ__PRINT,
  VG_USERREQ__PRINTNUM
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

#endif
