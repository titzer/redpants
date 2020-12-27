//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-8-2000                                                        //
// Module:  ia32/devices/pic.c                                              //
// Purpose: This module of the kernel handle initializing and controlling   //
//          the the programmable interrupt controller.                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#define DEBUG_SWITCH DEBUG_INTERRUPTS
#define DEBUG_MODULE "INTR"
#define DEBUG_COLOR  WHITE

#include <base/util/debug.h>
#include <ia32/devices/io.h>
#include <ia32/devices/pic.h>
#include <ia32/cpu/ivm.h>
#include <base/devices/kterm.h>

ISR_DECLARATION(irq_ignore);

void short_delay(void);

void short_delay() { int cntr; for ( cntr = 0; cntr < 10; cntr++ ) ; }

u32i irq_mask = 0xFFFF; // All IRQs disabled initially

//////////////////////////////////////////////////////////////////////////////
//  initialize the 8259 programmable interrupt controller                   //
//////////////////////////////////////////////////////////////////////////////
void pic_init(void)
{
  word cntr;
  byte irqbase = ivm_getirqbase();

  outportb(MASTER_PIC, ICW1);          // Start 8259 initialization
  short_delay();
  outportb(SLAVE_PIC, ICW1);
  short_delay();

  outportb(MASTER_PIC+1, irqbase);  // Base interrupt vector
  short_delay();
  outportb(SLAVE_PIC+1, irqbase+8);
  short_delay();

  outportb(MASTER_PIC+1, 4);        // Bitmask for cascade on IRQ 2
  short_delay();
  outportb(SLAVE_PIC+1, 2);            // Cascade on IRQ 2
  short_delay();

  outportb(MASTER_PIC+1, ICW4);        // Finish 8259 initialization
  short_delay();
  outportb(SLAVE_PIC+1, ICW4);
  short_delay();

  outportb(MASTER_IMR, 0xff);          // Mask all interrupts
  short_delay();
  outportb(SLAVE_IMR, 0xff);
  short_delay();

  for ( cntr = 0; cntr < 16; cntr++ )
    ivm_setirq(cntr, ISR_ENTRYPOINT(irq_ignore), STANDARD_ISR);
}

//////////////////////////////////////////////////////////////////////////////
//  enable a maskable irq                                                   //
//////////////////////////////////////////////////////////////////////////////
void pic_unmaskirq(u8i num)
{
  irq_mask &= ~(((u16i)1) << ((u16i)num)); // change mask

  if ( num >= 8 )
     irq_mask &= ~(1 << 2);

  DEBUG_PRINT("Enable interrupt ");
  DEBUG_INT(num);
  DEBUG_STR(", new mask: ");
  DEBUG_H16(irq_mask);

  outportb(MASTER_IMR, irq_mask & 0xFF); // update irq mask on PIC
  outportb(SLAVE_IMR, (irq_mask >> 8) & 0xFF);
}

//////////////////////////////////////////////////////////////////////////////
//  disable a maskable irq                                                  //
//////////////////////////////////////////////////////////////////////////////
void pic_maskirq(u8i num)
{
  irq_mask |= (((u16i)1) << ((u16i)num)); // change irq mask

  if ( (irq_mask & 0xFF00) == 0xFF00 )
     irq_mask |= (1 << 2);

  DEBUG_PRINT("Disable interrupt ");
  DEBUG_INT(num);
  DEBUG_STR(", new mask: ");
  DEBUG_H16(irq_mask);

  outportb(MASTER_IMR, irq_mask & 0xFF); // update irq mask on pic
  outportb(SLAVE_IMR, (irq_mask >> 8) & 0xFF);
}

void pic_sendeoi(byte irqnum)
{
  outportb(MASTER_PIC, EOI); // tell pic we're done
  if ( irqnum >= 8 )
    outportb(SLAVE_PIC, EOI); // tell slave pic if needed
}

//////////////////////////////////////////////////////////////////////////////
// The default, ignore handler                                              //
//////////////////////////////////////////////////////////////////////////////
asm ( ".globl irq_ignore   \n"
      "enter_irq_ignore:   \n"
      "irq_ignore:         \n"
      "   pushl %eax       \n"
      "   movl $0x20, %eax \n" // output 0x20 to port 0x20
      "   outb $0x20       \n" // this is the EOI command to the PIC.
      "   popl %eax        \n"
      "   iret             \n" );
