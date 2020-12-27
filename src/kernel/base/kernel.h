//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-12-2000                                                       //
// Header:  base/kernel.h                                                   //
// Purpose: This is the kernel, the main portion of the operating system,   //
//          that handles all the duties of a modern operating system.       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_KERNEL_H_
#define _BASE_KERNEL_H_

#ifdef _KERNEL_ANCHOR_
 #define global
 #define globalinit(a) = a
#else
 #define global extern
 #define globalinit(a)
#endif

#include <base/configure.h>
#include <base/util/kprint.h>
#include <base/cpu/cpu.h>

#if SMP_SUPPORT
 #define MAX_CPUS SMP_MAX_CPUS
#else
 #define MAX_CPUS 1
#endif

struct sys_info
{
  byte   num_cpus;   // number of installed processors
  cpu    cpus[MAX_CPUS];   // processor data
  word   cpu_features;     // overall cpu features
  word   physical_memory;  // megs of physical memory
  word   memory_pages;     // total memory pages
  word   page_size;        // size of each memory page

} global system_info;

global word buildnum;

global cstr architecture
#if ( KERNEL_ARCHITECTURE == ARCH_IA32 )
  globalinit("IA-32");
#elif ( KERNEL_ARCHITECTURE == ARCH_X86_64 )
  globalinit("x86-64");
#elif ( KERNEL_ARCHITECTURE == ARCH_ALPHA )
  globalinit("Alpha");
#elif ( KERNEL_ARCHITECTURE == ARCH_SPARC )
  globalinit("Sparc");
#elif ( KERNEL_ARCHITECTURE == ARCH_PPC )
  globalinit("PowerPC");
#elif ( KERNEL_ARCHITECTURE == ARCH_IA64 )
  globalinit("IA-64");
#else
  globalinit("Unknown");
#endif

#endif // _KERNEL_H_
