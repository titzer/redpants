//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-14-2001                                                       //
// Header:  base/cpu/traps.h                                                //
// Purpose: This module of the kernel handles initializing and managing     //
//          all of the trap handlers.                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_TRAPS_H_
#define _BASE_TRAPS_H_

#include <base/cpu/task.h>

void  traps_init(void);
void  traps_setvector(byte num, void *handler);
void *traps_getvector(byte num);

#endif
