//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-8-2000                                                        //
// Module:  ia32/cpu/traps.c                                                //
// Purpose: This module of the kernel handle initializing the exception     //
//          handling stubs of the operating system.                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/system.h>
#include <base/configure.h>
#include <base/cpu/traps.h>
#include <base/devices/kterm.h>
#include <base/devices/ktimer.h>
#include <base/cpu/ivm.h>
#include <ia32/cpu/traps.h>

TRAP_DECLARATION(trap_divide_error);
TRAP_DECLARATION(trap_debug);
TRAP_DECLARATION(trap_unknown_error);
TRAP_DECLARATION(trap_breakpoint);
TRAP_DECLARATION(trap_overflow);
TRAP_DECLARATION(trap_bounds_check);
TRAP_DECLARATION(trap_invalid_opcode);
TRAP_DECLARATION(trap_coprocessor);
TRAP_DECLARATION(trap_double_fault);
TRAP_DECLARATION(trap_coprocessor_ovr);
TRAP_DECLARATION(trap_invalid_tss);
TRAP_DECLARATION(trap_segment_missing);
TRAP_DECLARATION(trap_stack_exception);
TRAP_DECLARATION(trap_gpf);
TRAP_DECLARATION(trap_coprocessor_err);

void default_trap(cpustate *, cstr e);

//////////////////////////////////////////////////////////////////////////////
//  initialize default handlers for processor exceptions                    //
//////////////////////////////////////////////////////////////////////////////
void traps_init(void)
{
  ivm_setvector(TRAP_DIVIDE_ERROR,    TRAP_ENTRYPOINT(trap_divide_error),    STANDARD_TRAP);
  ivm_setvector(TRAP_DEBUG_EXCEPTION, TRAP_ENTRYPOINT(trap_debug),           STANDARD_TRAP);
  ivm_setvector(TRAP_UNKNOWN_ERROR,   TRAP_ENTRYPOINT(trap_unknown_error),   STANDARD_TRAP);
  ivm_setvector(TRAP_BREAKPOINT,      TRAP_ENTRYPOINT(trap_breakpoint),      STANDARD_TRAP);
  ivm_setvector(TRAP_OVERFLOW,        TRAP_ENTRYPOINT(trap_overflow),        STANDARD_TRAP);
  ivm_setvector(TRAP_BOUNDS_CHECK,    TRAP_ENTRYPOINT(trap_bounds_check),    STANDARD_TRAP);
  ivm_setvector(TRAP_INVALID_OPCODE,  TRAP_ENTRYPOINT(trap_invalid_opcode),  STANDARD_TRAP);
  ivm_setvector(TRAP_COPROCESSOR,     TRAP_ENTRYPOINT(trap_coprocessor),     STANDARD_TRAP);
  ivm_setvector(TRAP_DOUBLE_FAULT,    TRAP_ENTRYPOINT(trap_double_fault),    STANDARD_TRAP);
  ivm_setvector(TRAP_COPROCESSOR_OVR, TRAP_ENTRYPOINT(trap_coprocessor_ovr), STANDARD_TRAP);
  ivm_setvector(TRAP_INVALID_TSS,     TRAP_ENTRYPOINT(trap_invalid_tss),     STANDARD_TRAP);
  ivm_setvector(TRAP_SEGMENT_MISSING, TRAP_ENTRYPOINT(trap_segment_missing), STANDARD_TRAP);
  ivm_setvector(TRAP_STACK_EXCEPTION, TRAP_ENTRYPOINT(trap_stack_exception), STANDARD_TRAP);
  ivm_setvector(TRAP_GPF,             TRAP_ENTRYPOINT(trap_gpf),             STANDARD_TRAP);
  //  ivm_setvector(TRAP_PAGE_FAULT,      TRAP_ENTRYPOINT(trap_page_fault),      STANDARD_TRAP);
  ivm_setvector(TRAP_COPROCESSOR_ERR, TRAP_ENTRYPOINT(trap_coprocessor_err), STANDARD_TRAP);
}

//////////////////////////////////////////////////////////////////////////////
//  the default handler: reboot in 60 seconds                               //
//////////////////////////////////////////////////////////////////////////////
void default_trap(cpustate *taskstate, cstr excpt)
{
  word secs = 60;

  kterm_print("\nException encountered: ", TEXT_ERROR);
  kterm_print(excpt, TEXT_HILIGHT);
  kterm_print("\nError code: ", TEXT_ERROR);
  kterm_print("%x",taskstate->errcode, TEXT_HILIGHT);
  kterm_print("\nOriginating process: ", TEXT_NORMAL);
  kterm_print("(unknown)", TEXT_HILIGHT);

  kterm_print("\nEIP: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->eip, TEXT_HILIGHT);
  kterm_print(" EFLAGS: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->eflags, TEXT_HILIGHT);
  kterm_print(" CS: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->cs, TEXT_HILIGHT);
  kterm_print(" DS: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->ds, TEXT_HILIGHT);
  //  kterm_print(" ES: 0x", TEXT_NORMAL);
  //  kterm_print("%x",taskstate->es, TEXT_HILIGHT);

  kterm_print("\nEAX: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->eax, TEXT_HILIGHT);
  kterm_print(" EBX: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->ebx, TEXT_HILIGHT);
  kterm_print(" ECX: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->ecx, TEXT_HILIGHT);
  kterm_print(" EDX: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->edx, TEXT_HILIGHT);

  kterm_print("\nEDI: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->edi, TEXT_HILIGHT);
  kterm_print(" ESI: 0x", TEXT_NORMAL);
  kterm_print("%x",taskstate->esi, TEXT_HILIGHT);

  kterm_print("\n\nThis is a fatal exception.", TEXT_ERROR);

  while ( secs > 0 )
     {
     kterm_print("\rThe system will reboot in ", TEXT_NORMAL);
     kterm_print("%d",secs, TEXT_HILIGHT);
     kterm_print(" seconds... \b", TEXT_NORMAL);
     ktimer_delay(1000);
     secs--;
     }

  system_reset();
}


//////////////////////////////////////////////////////////////////////////////
//  macros expand into the C function definitions for exception handlers    //
//////////////////////////////////////////////////////////////////////////////
TRAP_HANG_STUB(  trap_divide_error,    "00 (Divide Error)")
TRAP_HANG_STUB(  trap_debug_exception, "01 (Debug Exception)")
TRAP_HANG_STUB(  trap_unknown_error,   "02 (Unknown NMI Error)")
TRAP_HANG_STUB(  trap_breakpoint,      "03 (Breakpoint)")
TRAP_HANG_STUB(  trap_overflow,        "04 (Overflow)")
TRAP_HANG_STUB(  trap_bounds_check,    "05 (Bounds Check)")
TRAP_HANG_STUB(  trap_invalid_opcode,  "06 (Invalid Opcode)")
TRAP_HANG_STUB(  trap_coprocessor,     "07 (Coprocessor)")
TRAP_HANG_STUB_E(trap_double_fault,    "08 (Double Fault)")
TRAP_HANG_STUB(  trap_coprocessor_ovr, "09 (Coprocessor Overflow)")
TRAP_HANG_STUB_E(trap_invalid_tss,     "0A (Invalid TSS)")
TRAP_HANG_STUB_E(trap_segment_missing, "0B (Segment Missing)")
TRAP_HANG_STUB_E(trap_stack_exception, "0C (Stack Exception)")
TRAP_HANG_STUB_E(trap_gpf,             "0D (General Protection)")
TRAP_HANG_STUB(  trap_coprocessor_err, "0F (Coprocessor Error)")

//////////////////////////////////////////////////////////////////////////////
//  the exception handlers epilog code. all exception handler's assembly    //
//  wrappers generate code to save the scratch registers, call the C code   //
//  exception handler, and return to this portion of code. this exception   //
//  code then checks to see if a task switch should take place, and if so,  //
//  performs a task switch.                                                 //
//////////////////////////////////////////////////////////////////////////////
/*asm (
  ".globl enter_epilog         \n"
  "enter_epilog:               \n"
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n" // check to see if cur == next
  "                            \n" // if so return to user task
  "                            \n" // otherwise, check nested interrupt level
  "                            \n" // if 0 (no more nested interrupts
  "                            \n" // jump to task switch
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  " no_switch:                 \n"
  "  add $10, %esp             \n" // skip state pointer, gs, fs, es
  "  popw %ds                  \n" // get user data segment
  "  popa                      \n" // pop general registers
  "  add $4, %esp              \n" // skip error code
  "  iret                      \n" // end interrupt/exception, return to task
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  "                            \n"
  " task_switch:               \n"
  "                            \n"
  "                            \n" // some code goes in here to switch pgdirs
  "                            \n" // get new stack pointer, change LDT, etc
  "                            \n"
  "                            \n"
  "                            \n"
  "  popw %gs                  \n" // get user gs
  "  popw %fs                  \n" // get user fs
  "  popw %es                  \n" // get user es
  "  popw %ds                  \n" // get user data segment
  "  popa                      \n" // pop general registers
  "  add $4, %esp              \n" // skip error code
  "  iret                      \n" // end interrupt/exception, return to task
  );

void stack_print(void)
{
  u8i *esp;
  u32i cntr;

  asm ( " mov %%esp, %%eax " : "=a" (esp) );

  kterm_print("\nStack:", TEXT_BRIGHT);
  kterm_print("%x",(u32i)esp, TEXT_HILIGHT);
  kterm_print(":", TEXT_BRIGHT);

  for ( cntr = 0; cntr < 32; cntr++ )
     printhex8(esp[cntr], TEXT_HILIGHT);
}*/
