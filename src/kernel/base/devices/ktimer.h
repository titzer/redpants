//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-27-2001                                                      //
// Header:  base/devices/ktimer.h                                           //
// Purpose: This is the standard kernel timer that sends interrupts for     //
//          scheduling and accounting purposes.                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_KTIMER_H_
#define _BASE_KTIMER_H_

#include <base/configure.h>

word ktimer_init(void);
word ktimer_resolution(void);
u64i ktimer_ticks(void);
word ktimer_delay(word ms);

#endif
