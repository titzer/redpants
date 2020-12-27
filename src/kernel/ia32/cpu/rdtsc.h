//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-27-2001                                                      //
// Header:  ia32/cpu/rdtsc.h                                                //
// Purpose: This header contains the declarations for the facilities to     //
//          access the processor's RDTSC instruction, if it is supported.   //
//          This instruction reads the processor timestamp counter, a       //
//          count of the cycles elapsed since the processor powered on.     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_RDTSC_H_
#define _IA32_RDTSC_H_

#include <base/configure.h>

word rdtsc_supported(void);
void rdtsc(u64i *count);

#endif
