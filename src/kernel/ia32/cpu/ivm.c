//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    7-14-2001                                                       //
// Module:  ivm.c (Interrupt Vector Manager)                                //
// Purpose: This module of the kernel handles initializing and controlling  //
//          all of the interrupt handling tables and vectors.               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/cpu/ivm.h>
#include <base/util/debug.h>
#include <base/devices/kterm.h>
#include <ia32/cpu/cpu.h>
#include <ia32/cpu/traps.h>
#include <ia32/cpu/desctables.h>
#include <ia32/devices/pic.h>

desc_table(idt_table, 256) { }; // the IDT

struct // this structure is required for the format of the lidt instruction
{
  unsigned short limit __attribute__ ((packed));
  union DT_entry *idt  __attribute__ ((packed));
} loadidt = { (256 * sizeof(union DT_entry) - 1), idt_table };

static int irqbase = 0x20;
static int ioapicmode = 0;

//////////////////////////////////////////////////////////////////////////////
// Set up the idt and initialize the PIC in order to begin                  //
// handling interrupts. Called by the main() function during                //
// initialization, but at no other time.                                    //
//////////////////////////////////////////////////////////////////////////////
void ivm_init(void)
{
  u32i cntr;

  for ( cntr = 0; cntr < 256; cntr++ ) // ignore all interrupts for now
    ivm_setvector(cntr, isr_ignore, STANDARD_ISR);

  pic_init();
  traps_init(); // handle processor exceptions

  asm ("lidt (%0)                 \n"   // Load the IDT
       "pushfl                    \n"
       "andl $0xffffbfff,(%%esp)  \n"
       "popfl                     \n"
       "sti                       \n"  // restore interrupts
       :  : "r" ((char *) &loadidt)   );
}

//////////////////////////////////////////////////////////////////////////////
//  get the offset in the vector table where IRQs start                     //
//////////////////////////////////////////////////////////////////////////////
word ivm_getirqbase(void)
{
  return irqbase;
}


//////////////////////////////////////////////////////////////////////////////
//  set the handler for an interrupt vector                                 //
//////////////////////////////////////////////////////////////////////////////
void ivm_setvector(word num, void *handler, word flags)
{
  u16i codeselector = KERNEL_CS;

  idt_table[num].gate.offset_low    = (u16i) (((u32i)handler)&0xffff);
  idt_table[num].gate.selector      = codeselector;
  idt_table[num].gate.access        = flags;
  idt_table[num].gate.offset_high   = (u16i) (((u32i)handler) >> 16);

  DEBUG_PRINT("Install handler #");
  DEBUG_INT(num);
  DEBUG_STR(", 0x");
  DEBUG_H32((u32i)handler);
}

//////////////////////////////////////////////////////////////////////////////
//  get the handler for an interrupt vector                                 //
//////////////////////////////////////////////////////////////////////////////
void *ivm_getvector(word num)
{
  u32i result;

  result = (u32i)idt_table[num].gate.offset_high;
  result = (result<<16)|(idt_table[num].gate.offset_low);

  DEBUG_PRINT("Request handler #");
  DEBUG_INT(num);
  DEBUG_STR(", 0x");
  DEBUG_H32(result);

  return (void *)result;
}

//////////////////////////////////////////////////////////////////////////////
//  set the handler for an IRQ vector                                       //
//////////////////////////////////////////////////////////////////////////////
void ivm_setirq(byte num, void *handler, word flags)
{
  ivm_setvector(num+irqbase, handler, flags);
}

//////////////////////////////////////////////////////////////////////////////
//  get the handler for an IRQ vector                                       //
//////////////////////////////////////////////////////////////////////////////
void *ivm_getirq(byte num)
{
  return ivm_getvector(num+irqbase);
}

//////////////////////////////////////////////////////////////////////////////
//  mask an irq by turning it off at the appropriate device                 //
//////////////////////////////////////////////////////////////////////////////
void ivm_maskirq(byte num)
{
  if ( ioapicmode != 0 )
    ;
  else pic_maskirq(num);
}

//////////////////////////////////////////////////////////////////////////////
//  unmask an irq by turning it on at the appropriate device                //
//////////////////////////////////////////////////////////////////////////////
void ivm_unmaskirq(byte num)
{
  if ( ioapicmode != 0 )
    ;
  else pic_unmaskirq(num);
}

//////////////////////////////////////////////////////////////////////////////
//  signal that an interrupt is finished to the appropriate device          //
//////////////////////////////////////////////////////////////////////////////
void ivm_signaleoi(byte num)
{
  if ( ioapicmode != 0 )
    ;
  else pic_sendeoi(num);
}

//////////////////////////////////////////////////////////////////////////////
//  kernel panic handler, currently unused                                  //
//////////////////////////////////////////////////////////////////////////////
ISR_DEFINITION(isr_panic)
{
  kterm_print("Redpants Kernel ", TEXT_NORMAL);
  kterm_print("PANIC! ", TEXT_ERROR);
  kterm_print("You must ", TEXT_NORMAL);
  kterm_print("REBOOT!", TEXT_HILIGHT);
  while ( 1 ) ;
}

//////////////////////////////////////////////////////////////////////////////
// The default, ignore handler                                              //
//////////////////////////////////////////////////////////////////////////////
asm ( ".globl isr_ignore   \n"
      "enter_isr_ignore:   \n"
      "isr_ignore:         \n"
      "   iret             \n" );
