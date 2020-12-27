//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-14-2001                                                       //
// Header:  ia32/cpu/ivm.h                                                  //
// Purpose: This module of the kernel handles initializing and controlling  //
//          all of the interrupt handling tables and vectors.               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_IVM_H_
#define _IA32_IVM_H_

#include <base/configure.h>
#include <base/cpu/ivm.h>
#include <ia32/cpu/desctables.h>

#define STANDARD_ISR (D_INT + D_PRESENT + D_DPL3)

#define ISR_ENTRYPOINT(x) enter_ ## x
#define ISR_DECLARATION(x)  void x(void);  \
                            void enter_ ## x(void)

#define ISR_DEFINITION(f) \
  asm(                    \
   ".globl enter_" #f " \n" \
   "enter_" #f ":       \n" \
   "   pusha          \n" \
   "   pushw %ds      \n" \
   "   pushw %es      \n" \
   "   pushw %fs      \n" \
   "   pushw %gs      \n" \
   "   pushw %ss      \n" \
   "   popw %ds       \n" \
   "   call " #f "    \n" \
   "   popw %gs       \n" \
   "   popw %fs       \n" \
   "   popw %es       \n" \
   "   popw %ds       \n" \
   "   popa           \n" \
   "   iret           \n" \
   );                     \
   void f(void)

void isr_ignore(void);
void isr_panic(void);
word ivm_getirqbase(void);

#endif
