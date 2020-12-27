//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    11-16-2000                                                      //
// Module:  ia32/devices/rtc.c                                              //
// Purpose: This module contains code that initializes and interfaces       //
//          the standard real time clock chip in the PC AT specification.   //
//          It is used for precise time tracking as well as date and time   //
//          support in the kernel.                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/configure.h>

#define DEBUG_SWITCH DEBUG_RTC
#define DEBUG_MODULE "RTC"
#define DEBUG_COLOR  DEBUG_RTC_COLOR

#include <base/kernel.h>

#include <ia32/devices/rtc.h>
#include <ia32/devices/io.h>
#include <ia32/cpu/ivm.h>
#include <base/util/debug.h>
#include <base/devices/kclock.h>

#define BCD8_TO_DECIMAL(a) ((((a&0xf0)>>4)*10)+(a&0xf))

date rtc_readdate(void); // read date directly from RTC
time rtc_readtime(void); // read time directly from RTC
byte rtc_readregister(byte num);
void rtc_writeregister(byte num, byte val);
byte bcd2byte(byte val);

ISR_DECLARATION(rtc_isr); // interrupt request handler

date currentdate; // maintain current date
time currenttime; // maintain current time
u64i rtc_ticks = 0; // current tick count

//////////////////////////////////////////////////////////////////////////////
//  initialize function which sets up RTC and interrupt handler             //
//////////////////////////////////////////////////////////////////////////////
void rtc_init(void)
{
  byte statusa, statusb, statusc;

  DEBUG_PRINT("Reading current date...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  currentdate = rtc_readdate();

  DEBUG_PRINT("Reading current time...");
  DEBUG_DEL(DEBUG_STD_DELAY);
  currenttime = rtc_readtime();

  DEBUG_PRINT("Boot time: ");
  DEBUG_STR(weekdays[currentdate.dayweek]);
  DEBUG_STR(", ");
  DEBUG_STR(months[currentdate.month-1]);
  DEBUG_STR(" ");
  DEBUG_INT(currentdate.daymonth);
  DEBUG_STR(", ");
  DEBUG_INT(currentdate.year);
  DEBUG_STR(" ");
  DEBUG_INT(currenttime.hour);
  DEBUG_STR(":");
  DEBUG_INT(currenttime.minute);
  DEBUG_STR(":");
  DEBUG_INT(currenttime.second);
  DEBUG_DEL(DEBUG_STD_DELAY);

  ivm_setirq(RTC_INT_VECTOR, ISR_ENTRYPOINT(rtc_isr), STANDARD_ISR);

  ivm_unmaskirq(RTC_IRQ);

  statusa = rtc_readregister(RTC_RGSTR_STATUSA);
  statusb = rtc_readregister(RTC_RGSTR_STATUSB);
  statusc = rtc_readregister(RTC_RGSTR_STATUSC);

  DEBUG_PRINT("Status A: 0x");
  DEBUG_H8(statusa);
  DEBUG_PRINT("Status B: 0x");
  DEBUG_H8(statusb);
  DEBUG_PRINT("Status C: 0x");
  DEBUG_H8(statusc);

  statusb |= BIT(6); // enable periodic interrupt

  rtc_writeregister(RTC_RGSTR_STATUSB, statusb);
  statusc = rtc_readregister(RTC_RGSTR_STATUSC);

  DEBUG_DEL(DEBUG_STD_DELAY);
  DEBUG_STR(" ");

}

//////////////////////////////////////////////////////////////////////////////
//  function to read date directly from RTC                                 //
//////////////////////////////////////////////////////////////////////////////
date rtc_readdate(void)
{
  date res;
  byte century;

  res.dayweek = bcd2byte(rtc_readregister(RTC_RGSTR_DAYOFWEEK));
  res.daymonth = bcd2byte(rtc_readregister(RTC_RGSTR_DAYOFMONTH));
  res.month = bcd2byte(rtc_readregister(RTC_RGSTR_MONTH));
  res.year = bcd2byte(rtc_readregister(RTC_RGSTR_YEAR));
  century = bcd2byte(rtc_readregister(RTC_RGSTR_CENTURY));
  res.year += bcd2byte(century)*100;

  return res;
}

byte bcd2byte(byte bcdval)
{
  return BCD8_TO_DECIMAL(bcdval);
}

//////////////////////////////////////////////////////////////////////////////
//  function to read time directly from RTC                                 //
//////////////////////////////////////////////////////////////////////////////
time rtc_readtime(void)
{
  time res;

  res.second = bcd2byte(rtc_readregister(RTC_RGSTR_SECOND));
  res.minute = bcd2byte(rtc_readregister(RTC_RGSTR_MINUTE));
  res.hour = bcd2byte(rtc_readregister(RTC_RGSTR_HOUR));

  return res;
}

//////////////////////////////////////////////////////////////////////////////
//  function to get current system date                                     //
//////////////////////////////////////////////////////////////////////////////
date rtc_getdate(void)
{
  return rtc_readdate();
}

//////////////////////////////////////////////////////////////////////////////
//  function to get current system time                                     //
//////////////////////////////////////////////////////////////////////////////
time rtc_gettime(void)
{
  return rtc_readtime();
}

//////////////////////////////////////////////////////////////////////////////
//  Read a register from the RTC                                            //
//////////////////////////////////////////////////////////////////////////////
byte rtc_readregister(byte num)
{
  outportb(RTC_COMMAND_PORT, num); // read specified register
  return inportb(RTC_DATA_PORT);
}

//////////////////////////////////////////////////////////////////////////////
//  Write to a register in the RTC                                          //
//////////////////////////////////////////////////////////////////////////////
void rtc_writeregister(byte num, byte val)
{
  outportb(RTC_COMMAND_PORT, num); // write specified register
  outportb(RTC_DATA_PORT, val);
}

//////////////////////////////////////////////////////////////////////////////
//  RTC interrupt stub                                                      //
//////////////////////////////////////////////////////////////////////////////
ISR_DEFINITION(rtc_isr)
{
  byte unused;

  rtc_ticks++;               // increment

//  if ( (rtc_ticks % 2048) == 0 )
  DEBUG_PRINT("RTC says hi.");

  outportb(RTC_COMMAND_PORT, RTC_RGSTR_STATUSC); // read status C
  unused = inportb(RTC_DATA_PORT);

  ivm_sendeoi(RTC_INT_VECTOR);
}

