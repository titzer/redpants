//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    11-25-2001                                                      //
// Header:  ia32/configure.h                                                //
// Purpose: This header makes declarations of some of the configuration     //
//          options for the IA-32 portion of the Redpants kernel.           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_CONFIGURE_H_
#define _IA32_CONFIGURE_H_


// memory configuration
#define VMM_64BIT_POINTERS     no
#define VMM_4MB_PAGES          yes
#define VMM_ADDRESS_EXTENSIONS no

// CPU defaults
#define CPU_DEFAULT_MODEL    "x86 compatible"
#define CPU_DEFAULT_VENDOR   "Unknown"
#define CPU_DEFAULT_SPEED    0
#define CPU_DEFAULT_L1CACHE  0
#define CPU_DEFAULT_L2CACHE  0
#define CPU_DETECT_SPEED     yes
#define CPU_DETECT_TICKS     2

#endif
