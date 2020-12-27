//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    5-9-2000                                                        //
// Module:  ia32/devices/pit.c                                              //
// Purpose: This module contains the definitions of the functions that are  //
//          available to the kernel for handling timing, including all      //
//          necessary initialization functions.                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#define DEBUG_SWITCH DEBUG_TIMER
#define DEBUG_MODULE "TIMER"
#define DEBUG_COLOR  DEBUG_TIMER_COLOR

#include <base/kernel.h>

#include <ia32/devices/io.h>
#include <ia32/devices/pit.h>
#include <base/devices/kterm.h>
#include <base/devices/ktimer.h>
#include <ia32/devices/pic.h>
#include <base/util/debug.h>
#include <ia32/cpu/ivm.h>


#define WAIT_START(x) x = ktimer_ticks(); \
     while ( x == ktimer_ticks() ) ; x = ktimer_ticks()

ISR_DECLARATION(pit_isr);

void calibrate_delay(void);
void delay_stub(word count);

u64i volatile uptime_ticks = 0;
word timer_resolution;
word delay_calibration = 0;

//////////////////////////////////////////////////////////////////////////////
//  initialize timer                                                        //
//////////////////////////////////////////////////////////////////////////////
void pit_init(void)
{
  word interval, channel = SYS_CHANNEL;
  timer_resolution = TIMER_RESOLUTION;

  DEBUG_PRINT("Setting timer IRQ...");

  interval = 1193180/timer_resolution; // calculate interval

  DEBUG_PRINT("PIT Interval value: ");
  DEBUG_INT(interval);

  outportb(TMR_CTRL, (channel*0x40) + TMR_BOTH + TMR_MD3);
  outportb((0x40+channel), (unsigned char) interval);
  outportb((0x40+channel), (unsigned char) (interval>>8));

  ivm_setirq(TIMER_IRQ, ISR_ENTRYPOINT(pit_isr), STANDARD_ISR);
  ivm_unmaskirq(TIMER_IRQ); // enable the timer interrupt

  DEBUG_PRINT("Calibrating delay loop...");

#if ( !NO_DELAY_CALIB )
  calibrate_delay();
#endif

  DEBUG_PRINT("Delay calibration complete. ");
  DEBUG_DEL(DEBUG_STD_DELAY);

}

//////////////////////////////////////////////////////////////////////////////
//  get number of ticks passed since kernel start                           //
//////////////////////////////////////////////////////////////////////////////
u64i pit_getticks(void)
{
  return uptime_ticks;
}

//////////////////////////////////////////////////////////////////////////////
//  get the resolution (rate) of the timer in hz                            //
//////////////////////////////////////////////////////////////////////////////
word pit_resolution(void)
{
  return timer_resolution;
}

//////////////////////////////////////////////////////////////////////////////
//  delay specified number of milliseconds                                  //
//////////////////////////////////////////////////////////////////////////////
void pit_delay(u16i ms)
{
  if ( delay_calibration > 0 )
     delay_stub(ms*delay_calibration);
}

//////////////////////////////////////////////////////////////////////////////
// calibration of delay loop, similar to Linux's method                     //
//////////////////////////////////////////////////////////////////////////////
void calibrate_delay(void)
{
  word top, est;
  word ticks;

  top = 8;

  while ( 1 )
    {
    DEBUG_PRINT("Ceiling value: ");
    DEBUG_INT(top);

    top = top << 1;      // next power of two
    WAIT_START(ticks);   // wait until start of clock tick

    delay_stub(top);     // delay
    if ( ticks != uptime_ticks ) break; // a clock period passed
    }

  est = top >> 1; // start our estimate at half of top
  top = top >> 2; // position to flip next bit

  while ( top > 16 ) // refine estimate bit by bit
    {
    top = top >> 1;      // we are using top now as the current bit
    est |= top;          // turn on next bit
    WAIT_START(ticks);   // wait until start of clock tick

    delay_stub(est); // delay with our estimate
    if ( ticks != uptime_ticks ) est ^= top; // too long, turn off bit
                       // otherwise, leave bit on (not long enough)
    }

  delay_calibration = (est * timer_resolution)/1000;

  DEBUG_PRINT("Final calibration value: ");
  DEBUG_INT(delay_calibration);
}

//////////////////////////////////////////////////////////////////////////////
//  stub called to calibrate delay and by delay itself                      //
//////////////////////////////////////////////////////////////////////////////
void delay_stub(word count)
{
  asm(
     "   mov %0, %%ecx      \n"
     "delay_loop:         \n"
     "   xor %%eax, %%eax   \n"
     "   loop delay_loop    \n"
     "                    \n"
     :
     : "g" (count)
     : "%eax", "%ecx");
}

//////////////////////////////////////////////////////////////////////////////
// timer interrupt handler, macro generates assembly wrapper                //
//////////////////////////////////////////////////////////////////////////////
ISR_DEFINITION(pit_isr)
{
  uptime_ticks++;            // increment

  ivm_sendeoi(TIMER_IRQ);
}
