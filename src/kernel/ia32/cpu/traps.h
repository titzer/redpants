//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-14-2001                                                       //
// Header:  ia32/cpu/traps.h                                                //
// Purpose: This module of the kernel handles initializing and managing     //
//          all of the trap handlers.                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_TRAPS_H_
#define _IA32_TRAPS_H_

#include <base/cpu/task.h>
#include <ia32/cpu/ivm.h>

// trap vector numbers
#define TRAP_DIVIDE_ERROR    0x00
#define TRAP_DEBUG_EXCEPTION 0x01
#define TRAP_UNKNOWN_ERROR   0x02
#define TRAP_BREAKPOINT      0x03
#define TRAP_OVERFLOW        0x04
#define TRAP_BOUNDS_CHECK    0x05
#define TRAP_INVALID_OPCODE  0x06
#define TRAP_COPROCESSOR     0x07
#define TRAP_DOUBLE_FAULT    0x08
#define TRAP_COPROCESSOR_OVR 0x09
#define TRAP_INVALID_TSS     0x0A
#define TRAP_SEGMENT_MISSING 0x0B
#define TRAP_STACK_EXCEPTION 0x0C
#define TRAP_GPF             0x0D
#define TRAP_PAGE_FAULT      0x0E
#define TRAP_COPROCESSOR_ERR 0x10

#define TRAP_DEFINITION(f) TRAP_STUB_NO_ERROR(f)

#define TRAP_HANG_STUB(f, s) TRAP_STUB_NO_ERROR(f)\
 { default_trap(taskstate, s); system_hang(); }

#define TRAP_HANG_STUB_E(f, s) TRAP_STUB_ERROR(f)\
 { default_trap(taskstate, s); system_hang(); }

#define STANDARD_TRAP (D_INT + D_PRESENT + D_DPL3)

#define TRAP_ENTRYPOINT(x) enter_ ## x
#define TRAP_DECLARATION(x)    \
  void x(cpustate *taskstate); \
  void enter_ ## x(void)

// trap stub assembly without errorcode
#define TRAP_STUB_NO_ERROR(f)  \
  asm (                             \
   ".globl enter_" #f "      \n"    \
   "enter_" #f ":            \n"    \
       " pushl $0     \n" \
       " pusha        \n" \
       " pushw %ds    \n" \
       " pushw %es    \n" \
       " pushw %ss    \n" \
       " popw %ds     \n" \
       " pushl %esp   \n" \
       " call " #f "  \n" \
       " add $4, %esp \n" \
       " popw %es     \n" \
       " popw %ds     \n" \
       " popa         \n" \
       " add $4, %esp \n" \
       " iret         \n" \
   );                     \
   void f(cpustate *taskstate)

// trap stub assembly with errorcode
#define TRAP_STUB_ERROR(f)   \
  asm (".globl enter_" #f "\n"    \
       "enter_" #f ":\n"          \
       " pusha        \n" \
       " pushw %ds    \n" \
       " pushw %es    \n" \
       " pushw %ss    \n" \
       " popw %ds     \n" \
       " pushl %esp   \n" \
       " call " #f "  \n" \
       " add $4, %esp \n" \
       " popw %es     \n" \
       " popw %ds     \n" \
       " popa         \n" \
       " add $4, %esp \n" \
       " iret         \n" \
   );                      \
   void f(cpustate *taskstate)

void traps_init(void);

#endif
