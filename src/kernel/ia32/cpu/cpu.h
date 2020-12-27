//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-06-2001                                                       //
// Header:  ia32/cpu/cpu.h                                                  //
// Purpose: This header makes declarations of the members of the cpu        //
//          structure needed by the IA-32 implementation.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

/* Additional machine dependent declarations */
#ifndef _IA32_CPU_H_
#define _IA32_CPU_H_

#include <base/util/types.h>
#include <base/cpu/cpu.h>

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10

#define CPU_FEATURE_CPUID  BIT(0)
#define CPU_FEATURE_RDTSC  BIT(1)
#define CPU_FEATURE_FPU    BIT(2)
#define CPU_FEATURE_APIC   BIT(3)
#define CPU_FEATURE_PAE    BIT(4)
#define CPU_FEATURE_PGE    BIT(5)
#define CPU_FEATURE_PSE    BIT(6)
#define CPU_FEATURE_PSE36  BIT(7)
#define CPU_FEATURE_MMX    BIT(8)
#define CPU_FEATURE_3DNOW  BIT(9)
#define CPU_FEATURE_3DNOW2 BIT(10)
#define CPU_FEATURE_SSE    BIT(11)
#define CPU_FEATURE_SSE2   BIT(12)
#define CPU_FEATURE_X86_64 BIT(13)
#define CPU_FEATURE_IA64   BIT(14)

void cpus_detect(void);

typedef int statusword;

#endif

