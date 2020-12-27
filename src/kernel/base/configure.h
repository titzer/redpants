//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    4-25-2000                                                       //
// Header:  base/configure.h                                                //
// Purpose: This module contains the configuration options that need to be  //
//          passed to the kernel in order to turn on/off specification      //
//          features or behavior in the kernel.                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_CONFIGURE_H_
#define _BASE_CONFIGURE_H_

#include <base/util/types.h>

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    This header file is intended to allow customization of the kernel     //
// with minimal source editing by combining the most frequently used        //
// switches into one location, facilitating ease of location and automatic  //
// generation by configuration tools.                                       //
//                                                                          //
//    Most of the source files that depend on these switches are leaves,    //
// meaning they do not call functions outside of their own source file      //
// (for their major functionality). This is by design--both to keep         //
// preprocessing directives from cluttering up the main execution tree,     //
// and to ensure that the main execution tree is as portable as possible.   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define INIT_OK      1
#define INIT_ERROR  -1
#define INIT_FAILED -2

// debugging switches
#define DEBUG_ALL         no
#define DEBUG_NONE        yes
#define DEBUG_SMP_DETECT  yes
#define DEBUG_SMP_COLOR   LIGHTBLUE
#define DEBUG_MEM_INIT    no
#define DEBUG_MEM_COLOR   RED
#define DEBUG_KMM         yes
#define DEBUG_KMM_COLOR   BROWN
#define DEBUG_VMM         yes
#define DEBUG_VMM_COLOR   YELLOW
#define DEBUG_CPU_DETECT  yes
#define DEBUG_CPU_COLOR   MAGENTA
#define DEBUG_TIMER       no
#define DEBUG_TIMER_COLOR GREEN
#define DEBUG_RTC         no
#define DEBUG_RTC_COLOR   CYAN
#define DEBUG_INTERRUPTS  no
#define DEBUG_INT_COLOR   WHITE
#define DEBUG_STD_DELAY   1000

// kernel attributes
#define KERNEL_ARCHITECTURE   ARCH_IA32
#define KERNEL_MEMSIZE        F000h
#define KERNEL_SHELL          yes

// keyboard switches
#define KB_DEBUG_SCANS no 
#define KB_BUFFER_SIZE 256
#define KB_NUMLOCK_ON  yes

// multiprocessing switches
#define SMP_SUPPORT yes
#define SMP_MAX_CPUS 8

// task management information
#define MAX_TASKS 64

// timer data
#define TIMER_RESOLUTION 100

// KMM data
#define KMM_MALLOC_ALIGN       4
#define KMM_HEAP_CLASSES       24
#define KMM_EXTEND_SIZE        8

// status reporter switches
#define STATUS_SILENT       no
#define STATUS_SILENTFAIL   no
#define STATUS_SPILL_OK     yes
#define STATUS_SPILL_PASS   yes
#define STATUS_SPILL_FAIL   yes

// console switches
#define CONSOLE_COLOR      yes
#define CONSOLE_STATUSLINE yes
#define CONSOLE_RPBANNER   yes
#define CONSOLE_HEADERLINE yes
#define HEADERLINE_COLOR WHITE
#define HEADERLINE_BACK  CYAN
#define STATUSLINE_COLOR WHITE
#define STATUSLINE_BACK  BLUE

// standard text colors for kernel
#define TEXT_NORMAL  LIGHTGRAY
#define TEXT_BRIGHT  LIGHTCYAN
#define TEXT_HILIGHT LIGHTGREEN
#define TEXT_ERROR   LIGHTRED

#include <target/configure.h>

#endif
