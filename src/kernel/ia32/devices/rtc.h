//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    11-7-2000                                                       //
// Header:  ia32/devices/rtc.h                                              //
// Purpose: This header contains the declarations of the functions that are //
//          available to the kernel for interfacing the RTC (real time      //
//          clock), which is used for time/date and for precise timing      //
//          of the scheduling interrupts.                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#ifndef _IA32_RTC_H_
#define _IA32_RTC_H_

#include <base/devices/kclock.h>
#include <ia32/devices/io.h>

#define RTC_COMMAND_PORT  0x70
#define RTC_DATA_PORT     0x71

#define RTC_RGSTR_SECOND     0x00
#define RTC_RGSTR_MINUTE     0x02
#define RTC_RGSTR_HOUR       0x04
#define RTC_RGSTR_DAYOFWEEK  0x06
#define RTC_RGSTR_DAYOFMONTH 0x07
#define RTC_RGSTR_MONTH      0x08
#define RTC_RGSTR_YEAR       0x09
#define RTC_RGSTR_CENTURY    0x32
#define RTC_RGSTR_STATUSA    0x0A
#define RTC_RGSTR_STATUSB    0x0B
#define RTC_RGSTR_STATUSC    0x0C

#define RTC_INT_VECTOR 0x70
#define RTC_IRQ        0x08

// externally accessible functions for the RTC

void rtc_init(void);

date rtc_getdate(void); // get current date
time rtc_gettime(void); // get current time
byte rtc_setdate(date *); // set current date on RTC
byte rtc_settime(time *); // set current time on RTC

u64i     rtc_getticks(void);  // get uptime ticks
datetime rtc_getuptime(void); // get uptime in years, days, hours, etc

cstr months[12]
#ifdef _KERNEL_ANCHOR_
 = { "January", "February", "March", "April", "May",
     "June", "July", "August", "September", "October",
     "November", "December" }
#endif
 ;

cstr weekdays[7]
#ifdef _KERNEL_ANCHOR_
 = { "Sunday", "Monday", "Tuesday", "Wednesday",
     "Thursday", "Friday", "Saturday" }
#endif
 ;

#endif
