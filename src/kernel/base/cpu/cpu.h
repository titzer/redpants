//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-06-2001                                                       //
// Header:  base/cpu/cpu.h                                                  //
// Purpose: This header makes declarations of some of the important         //
//          functions of the cpu code.                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_CPU_H_
#define _BASE_CPU_H_

#include <base/util/types.h>

//////////////////////////////////////////////////////////////////////////////
// the structure that describes everything about a particular processor.    //
//////////////////////////////////////////////////////////////////////////////
typedef struct _cpu 
{
  word id;         // ID number of the processor
  cstr vendor;     // Vendor (manufacturer) of the cpu
  cstr model;      // Model name of the cpu

  word curtask;    // The current task on the cpu 

  // include architecture dependent members
#include <target/cpu/cpu_def.h>

} cpu;

//////////////////////////////////////////////////////////////////////////////
// the structure that describes the execution state of a processor.         //
//////////////////////////////////////////////////////////////////////////////
typedef struct _cpustate {

  // include architecture dependent members
#include <target/cpu/cpustate_def.h>

} cpustate;

void cpu_init(void);
void cpu_detect(void);
word cpu_measure_khz(void);
word cpu_estimate_mhz(word khz);
u64i cpu_cycles(void);
word cpu_which(void);
word cpu_disable_interrupts(void);
void cpu_restore_interrupts(word);
void cpu_ctxsw(cpustate *state);

#endif
