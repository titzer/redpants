//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-8-2000                                                        //
// Header:  ia32/devices/pic.h                                              //
// Purpose: This module of the kernel handle initializing and controlling   //
//          the the programmable interrupt controller, and contains helper  //
//          functions that allow the rest of the kernel to work with        //
//          interrupts quickly and easily.                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//  Notes:                                                                  //
//                                                                          //
//     I distinguish between three different types of interrupts. IRQs      //
// are things that come in via the PIC and have a number between 0-15. The  //
// PIC maps these to interrupts in the interrupt vector table starting at   //
// MASTER_VEC up to MASTER_VEC+15. Another type of interrupt is the trap    //
// which is a processor error or page-fault or similar event that           //
// originates within the processor or on a coprocessor. The rest of         //
// interrupts that aren't either IRQs or traps I just call interrupts or    //
// ISRs. It makes sense if one thinks of the IRQ range and trap range       //
// being mapped "onto the ISR space" of 0-255.                              //
//     Other interrupts from other sources (eg APIC inter-processor         //
// interrupts are mapped into the ISR space so they don't conflict with     //
// the IRQ range or the trap range.                                         //
//     The consequence of this is that the PIC is the only part of the      //
// kernel that deals with IRQs.                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_PIC_H_
#define _IA32_PIC_H_

#include <base/util/types.h>
#include <base/configure.h>
#include <base/kernel.h>

#define MASTER_PIC 0x20
#define MASTER_IMR 0x21
#define SLAVE_PIC  0xA0
#define SLAVE_IMR  0xA1
#define EOI        0x20  /* EOI command */

#define ICW1   0x11      /* Cascade, Edge triggered         */
                         /* ICW2 is vector                  */
                         /* ICW3 is slave bitmap or number  */
#define ICW4   0x01      /* 8088 mode                       */
#define MASTER_VEC  0x20 /* Vector for master               */
#define SLAVE_VEC  0x28  /* Vector for slave                */
#define OCW3_IRR  0x0A   /* Read IRR                        */
#define OCW3_ISR  0x0B   /* Read ISR                        */

void pic_init(void);
void pic_setcontrol(byte irqnum, u16i control);
void pic_unmaskirq(byte irqnum);
void pic_maskirq(byte irqnum);
word pic_getisrnum(byte irqnum);
void pic_sendeoi(byte irqnum);

#endif
