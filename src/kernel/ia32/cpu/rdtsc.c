//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-27-2001                                                      //
// Module:  ia32/cpu/rdtsc.h                                                //
// Purpose: This header contains the declarations for the facilities to     //
//          access the processor's RDTSC instruction, if it is supported.   //
//          This instruction reads the processor timestamp counter, a       //
//          count of the cycles elapsed since the processor powered on.     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <ia32/cpu/rdtsc.h>
#include <ia32/cpu/traps.h>
#include <base/cpu/ivm.h>

// Hexadecimal value of the RDTSC instruction.
#define RDTSC_HEX 0x310f

TRAP_DECLARATION(rdtsc_trap);

word rdtsc_exception = 0;
word rdtsc_detected  = 0;
word rdtsc_ok        = 1;

//////////////////////////////////////////////////////////////////////////////
// Determine whether the RDTSC instruction is supported.                    //
// Attempts to execute the RDTSC instruction and catches a possible invalid //
// opcode exception.                                                        //
//////////////////////////////////////////////////////////////////////////////
word rdtsc_supported(void)
{
  u64i dummy;
  void *oldvec;

  if ( rdtsc_detected ) return rdtsc_ok;

  rdtsc_detected = 1;
  oldvec = ivm_getvector(TRAP_INVALID_OPCODE); // get old exception handler
  ivm_setvector(TRAP_INVALID_OPCODE,
		TRAP_ENTRYPOINT(rdtsc_trap), STANDARD_TRAP);
  rdtsc(&dummy); // try to execute the instruction

  if ( rdtsc_exception == true ) rdtsc_ok = false; // if we caught an exception

  ivm_setvector(TRAP_INVALID_OPCODE, // reset old handler
		oldvec, STANDARD_TRAP);

  rdtsc_exception = false;
  return rdtsc_ok;
}

//////////////////////////////////////////////////////////////////////////////
// Exception handler for detecting cpus                                     //
// Checks the originating instruction and advances to next instruction      //
//////////////////////////////////////////////////////////////////////////////
TRAP_DEFINITION(cpuid_trap)
{
  if ( *(u16i *)taskstate->eip == RDTSC_HEX )
     rdtsc_exception = true;

  taskstate->eip = taskstate->eip + 2; // advance past CPUID instruction
}

//////////////////////////////////////////////////////////////////////////////
// Perform the RDTSC instruction.                                           //
// Returns the result in the appropriate pointer.                           //
//////////////////////////////////////////////////////////////////////////////
void rdtsc(u64i *count)
{
  u32i *ptr = (u32i *)count;

  asm (
       " rdtsc "
       : "=a" (ptr[0]), "=b" (ptr[1])
       );
}
