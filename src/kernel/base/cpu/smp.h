//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-18-2001                                                      //
// Header:  base/cpu/smp.h                                                  //
// Purpose: This header contains declarations for the machine independent   //
//          support of SMP in the kernel.                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_SMP_H_
#define _BASE_SMP_H_

#include <base/cpu/cpu.h>

word smp_init(void);
word smp_sendipi(word proc, word num, word data);

#endif
