//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-27-2001                                                      //
// Module:  ia32/cpu/cpuid.c                                                //
// Purpose: This module implements the code to interface the processor's    //
//          CPUID instruction.                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <ia32/cpu/cpuid.h>
#include <ia32/cpu/traps.h>
#include <base/cpu/ivm.h>

// Hexadecimal value of the CPUID instruction.
#define CPUID_HEX 0xa20f

TRAP_DECLARATION(cpuid_trap);

word cpuid_exception = 0;
word cpuid_detected  = 0;
word cpuid_ok        = 1;

//////////////////////////////////////////////////////////////////////////////
// Determine whether the CPUID instruction is supported.                    //
// Attempts to execute the CPUID instruction and catches a possible invalid //
// opcode exception.                                                        //
//////////////////////////////////////////////////////////////////////////////
word cpuid_supported(void)
{
  u32i dummy;
  void *oldvec;

  if ( cpuid_detected ) return cpuid_ok;

  cpuid_detected = 1;
  oldvec = ivm_getvector(TRAP_INVALID_OPCODE); // get old exception handler
  ivm_setvector(TRAP_INVALID_OPCODE,
		TRAP_ENTRYPOINT(cpuid_trap), STANDARD_TRAP);
  cpuid(0, &dummy, &dummy, &dummy, &dummy); // try to execute the instruction

  if ( cpuid_exception == true ) cpuid_ok = false; // if we caught an exception

  ivm_setvector(TRAP_INVALID_OPCODE, // reset old handler
		oldvec, STANDARD_TRAP);

  cpuid_exception = false;
  return cpuid_ok;
}


//////////////////////////////////////////////////////////////////////////////
// Determine the vendor based on its ID from the CPUID instruction.         //
//////////////////////////////////////////////////////////////////////////////
cstr cpuid_vendor(u32i vendid)
{
  switch ( vendid ) {
  case VENDOR_INTEL:
    return "Intel";
    break;
  case VENDOR_AMD:
    return "AMD";
    break;
  case VENDOR_CYRIX:
    return "Cyrix";
    break;
  }

  return "Unknown";
}

//////////////////////////////////////////////////////////////////////////////
// Determine the cpu model based on its family/model from the instruction.  //
//////////////////////////////////////////////////////////////////////////////
cstr cpuid_modelname(u32i vendid, word family, word model)
{
  switch ( vendid ) {
  case VENDOR_INTEL: // Intel processor
    if ( family == 15 ) // pent4's report family 15
      return intel_models[7][model];
    else if ( family > 3 )
      return intel_models[family-4][model]; // get model from matrix
    break;
  case VENDOR_AMD: // AMD processor
    if ( family > 3 )
      return amd_models[family-4][model]; // get model from matrix
    break;
  case VENDOR_CYRIX: // Cyrix processor
    if ( family > 3 )
      return cyrix_models[family-4][model]; // get model from matrix
    break;
  }

  return "x86 Compatible";
}

//////////////////////////////////////////////////////////////////////////////
//  issue the cpuid instruction with specified level                        //
//  put results of registers into the specified locations                   //
//////////////////////////////////////////////////////////////////////////////
void cpuid(u32i lvl, u32i *eax, u32i *ebx, u32i *ecx, u32i *edx)
{
  // this new code is from the linux source
  // the old method was a little buggy

  asm (
       "cpuid"
       : "=a" (*eax),"=b" (*ebx),"=c" (*ecx),"=d" (*edx)
       : "a" (lvl)
       : "cc"
       );
}

//////////////////////////////////////////////////////////////////////////////
// Exception handler for detecting cpus                                     //
// Checks the originating instruction and advances to next instruction      //
//////////////////////////////////////////////////////////////////////////////
TRAP_DEFINITION(cpuid_trap)
{
  if ( *(u16i *)taskstate->eip == CPUID_HEX )
     cpuid_exception = true;

  taskstate->eip = taskstate->eip + 2; // advance past CPUID instruction
}
