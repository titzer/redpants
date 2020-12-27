//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-30-2000                                                       //
// Header:  base/system.h                                                   //
// Purpose: This header contains the definitions for the system API         //
//          functions that are provided to the shell that are less          //
//          dependent on the underlying mechanics of the kernel.            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_SYSTEM_H_
#define _BASE_SYSTEM_H_

#include <base/configure.h>

// system service routines
void system_service_key(byte keycode);
void system_service_event(byte syscode);

byte system_shutdown(void); // shutdown system and stop
byte system_reset(void);    // unconditional system reset
byte system_restart(void);  // shutdown and restart
void system_hang(void);     // hang in infinite loop

#endif // _SYSTEM_H_
